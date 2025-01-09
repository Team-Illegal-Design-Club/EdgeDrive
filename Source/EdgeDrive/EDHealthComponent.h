// EDHealthComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Components/GameFrameworkComponent.h"
#include "EDHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEDShield_AttributeChanged, class UEDHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FEDHealth_AttributeChanged, class UEDHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEDHealth_DeathEvent, AActor*, OwningActor);

UENUM(BlueprintType)
enum class EEDDeathState : uint8
{
    NotDead = 0,
    DeathStarted,
    DeathFinished
};

UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class EDGEDRIVE_API UEDHealthComponent : public UGameFrameworkComponent
{
    GENERATED_BODY()
public:
    UEDHealthComponent(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintAssignable)
    FEDHealth_AttributeChanged OnShieldChanged;

    UPROPERTY(BlueprintAssignable)
    FEDHealth_AttributeChanged OnMaxShieldChanged;

    UPROPERTY(BlueprintAssignable)
    FEDHealth_AttributeChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FEDHealth_AttributeChanged OnMaxHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FEDHealth_DeathEvent OnDeathStarted;

    UPROPERTY(BlueprintAssignable)
    FEDHealth_DeathEvent OnDeathFinished;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage,
        const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "ED|Shield")
    float GetShield() const { return CurrentShield; }

    UFUNCTION(BlueprintCallable, Category = "ED|Shield")
    float GetMaxShield() const { return MaxShield; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetHealthNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    EEDDeathState GetDeathState() const { return DeathState; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    virtual void TakeDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    virtual void Heal(float HealAmount, AActor* HealCauser);

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    virtual void SetMaxHealth(float NewMaxHealth, AActor* Instigator);
    UFUNCTION(BlueprintCallable, Category = "ED|Shield")
    virtual void SetMaxShield(float NewMaxShield, AActor* Instigator);

    virtual void StartDeath();
    virtual void FinishDeath();

protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Health")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Health")
    float CurrentHealth;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Shield")
    float MaxShield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Shield")
    float CurrentShield;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Shield")
    float ShieldReChargeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Shield")
    float CurrentNoHitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Shield")
    float ShieldRecoveryAmount;


    UPROPERTY()
    EEDDeathState DeathState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Health")
    bool bInvincible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Health")
    float InvincibilityDuration;

    FTimerHandle InvincibilityTimerHandle;
};