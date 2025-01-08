#include "EDHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

UEDHealthComponent::UEDHealthComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    DeathState = EEDDeathState::NotDead;

}


void UEDHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Move delegate bindings from constructor to BeginPlay
    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->OnTakeAnyDamage.AddDynamic(this, &UEDHealthComponent::HandleTakeAnyDamage);
    }
}



float UEDHealthComponent::GetHealthNormalized() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}
void UEDHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser)
{
    if (DamageAmount <= 0.0f || DeathState != EEDDeathState::NotDead)
    {
        return;
    }

    const float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    // ü�� ���� �̺�Ʈ ��ε�ĳ��Ʈ
    OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        StartDeath();
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

    // ü�� ȸ�� �̺�Ʈ ��ε�ĳ��Ʈ
    OnHealthChanged.Broadcast(this, OldHealth, CurrentHealth, HealCauser);
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

    // ��� ���� �̺�Ʈ ��ε�ĳ��Ʈ
    OnDeathStarted.Broadcast(Owner);
    Owner->ForceNetUpdate();
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

    // ��� �Ϸ� �̺�Ʈ ��ε�ĳ��Ʈ
    OnDeathFinished.Broadcast(Owner);
    Owner->ForceNetUpdate();
}


