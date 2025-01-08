// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "EDHealthComponent.generated.h"

/**
 * 
 */
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
public:
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
        const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
    {
        TakeDamage(Damage, DamageCauser);
    }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    float GetHealthNormalized() const;

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    EEDDeathState GetDeathState() const { return DeathState; }

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    virtual void TakeDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "ED|Health")
    virtual void Heal(float HealAmount, AActor* HealCauser);

    virtual void StartDeath();
    virtual void FinishDeath();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Health")
    float MaxHealth;

    UPROPERTY()
    float CurrentHealth;

    UPROPERTY()
    EEDDeathState DeathState;
};
