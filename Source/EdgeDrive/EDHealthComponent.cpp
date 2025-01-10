
// EDHealthComponent.cpp
#include "EDHealthComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UEDHealthComponent::UEDHealthComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = true;  // Tick 활성화
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    MaxShield = 50.0f;
    ShieldRecoveryAmount = 10.0f;
    CurrentShield = MaxShield;
    ShieldReChargeTime = .0f;  // 3초 후 쉴드 재충전
    CurrentNoHitTime = 0.0f;
    DeathState = EEDDeathState::NotDead;
    bInvincible = false;
    InvincibilityDuration = 0.5f;
}

void UEDHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    OnShieldChanged.Clear();
    OnMaxShieldChanged.Clear();
    OnHealthChanged.Clear();
    OnMaxHealthChanged.Clear();
    OnDeathStarted.Clear();
    OnDeathFinished.Clear();
    SetComponentTickEnabled(true);
    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->OnTakeAnyDamage.RemoveDynamic(this, &UEDHealthComponent::HandleTakeAnyDamage);
        Owner->OnTakeAnyDamage.AddDynamic(this, &UEDHealthComponent::HandleTakeAnyDamage);
        CurrentHealth = MaxHealth;
    }


    
}
void UEDHealthComponent::BeginDestroy()
{
    if (AActor* Owner = GetOwner())
    {
        Owner->OnTakeAnyDamage.RemoveDynamic(this, &UEDHealthComponent::HandleTakeAnyDamage);
    }

    Super::BeginDestroy();
}
void UEDHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green,
        FString::Printf(TEXT("Shield: %.1f, NoHitTime: %.1f"), CurrentShield, CurrentNoHitTime));
    // 쉴드 재충전 로직
    if (CurrentShield < MaxShield)
    {
        CurrentNoHitTime += DeltaTime;
        if (CurrentNoHitTime >= ShieldReChargeTime)
        {
            const float OldShield = CurrentShield;
            CurrentShield = FMath::Min(CurrentShield + (DeltaTime * ShieldRecoveryAmount), MaxShield);

            // 쉴드 값이 변경되었을 때만 이벤트 발생
            if (!FMath::IsNearlyEqual(OldShield, CurrentShield))
            {
                OnShieldChanged.Broadcast(this, OldShield, CurrentShield, nullptr);
            }
        }
    }
}
void UEDHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage,
    const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    TakeDamage(Damage, DamageCauser);
}

float UEDHealthComponent::GetHealthNormalized() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void UEDHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser)
{
    if (DamageAmount <= 0.0f || DeathState != EEDDeathState::NotDead || bInvincible)
    {
        return;
    }

    // 쉴드가 있는 경우
    if (CurrentShield > 0.0f)
    {
        const float OldShield = CurrentShield;
        CurrentShield = FMath::Clamp(CurrentShield - DamageAmount, 0.0f, MaxShield);
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue,
            FString::Printf(TEXT("Shield: %.1f"), CurrentShield));
        CurrentNoHitTime = 0.0f;
        // 쉴드 변경 이벤트 발생
        OnShieldChanged.Broadcast(this, OldShield, CurrentShield, DamageCauser);

        // 쉴드 타이머 리셋
 
    }
    // 쉴드가 없는 경우에만 체력 감소
    else
    {
        const float OldHealth = CurrentHealth;
        CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red,
            FString::Printf(TEXT("Health: %.1f"), CurrentHealth));
        OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, DamageCauser);

        if (CurrentHealth <= 0.0f)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Death");
            StartDeath();
            GetOwner()->Destroy();
        }
    }

    if (InvincibilityDuration > 0.0f)
    {
        bInvincible = true;
        GetWorld()->GetTimerManager().SetTimer(
            InvincibilityTimerHandle,
            [this]() { bInvincible = false; },
            InvincibilityDuration,
            false
        );
    }
}


void UEDHealthComponent::Heal(float HealAmount, AActor* HealCauser)
{
    if (HealAmount <= 0.0f || DeathState != EEDDeathState::NotDead)
    {
        return;
    }

    const float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);

    OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, HealCauser);
}

void UEDHealthComponent::SetMaxHealth(float NewMaxHealth, AActor* Instigator)
{
    if (NewMaxHealth <= 0.0f)
    {
        return;
    }

    const float OldMaxHealth = MaxHealth;
    MaxHealth = NewMaxHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);

    OnMaxHealthChanged.Broadcast(this, OldMaxHealth, MaxHealth, Instigator);
}
UFUNCTION(BlueprintCallable, Category = "ED|Shield")
void UEDHealthComponent::SetMaxShield(float NewMaxShield, AActor* Instigator)
{
    if (NewMaxShield <= 0.0f)
    {
        return;
    }

    const float OldMaxShield = MaxShield;
    MaxShield = NewMaxShield;
    CurrentShield = FMath::Clamp(CurrentShield, 0.0f, MaxShield);

    OnMaxShieldChanged.Broadcast(this, OldMaxShield, MaxShield, Instigator);
}
void UEDHealthComponent::StartDeath()
{

    if (DeathState != EEDDeathState::NotDead)
    {
        return;
    }

    DeathState = EEDDeathState::DeathStarted;

    AActor* Owner = GetOwner();
    check(Owner);

    OnDeathStarted.Broadcast(Owner);
}

void UEDHealthComponent::FinishDeath()
{
    if (DeathState != EEDDeathState::DeathStarted)
    {
        return;
    }

    DeathState = EEDDeathState::DeathFinished;

    AActor* Owner = GetOwner();
    check(Owner);

    OnDeathFinished.Broadcast(Owner);
}