#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDCombatComponent.generated.h"

UCLASS()
class EDGEDRIVE_API UEDCombatComponent : public UActorComponent
{
    GENERATED_BODY()

protected:

    UPROPERTY(EditAnywhere, Category = "Combat")
    TArray<UAnimMontage*> ComboAttackMontages;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TArray<float> ComboDamages;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TArray<float> ComboRadiuses;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ComboTimeWindow = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ComboResetTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* ParryAction;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 CurrentComboIndex = 0;

    UPROPERTY()
    FTimerHandle ComboResetTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bCanCombo = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAttacking;

    UPROPERTY(EditAnywhere)
    float Damage;

    UPROPERTY(EditAnywhere, Category = "Effects")
    class UNiagaraSystem* AfterImageTemplate;
public:
    UPROPERTY()
    class UStaticMeshComponent* GloveMesh;

public:
    UEDCombatComponent();

    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);
    void StartAttack();
    UFUNCTION(BlueprintCallable)
    void LineTrace();
    void PlayHitEffect(const FVector& HitLocation);

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(BlueprintCallable)
    void EnableComboWindow();

    UFUNCTION(BlueprintCallable)
    void DisableComboWindow();

    void ResetCombo();

    UFUNCTION(BlueprintCallable)
    float GetCurrentComboDamage() const;

    UFUNCTION(BlueprintCallable)
    float GetCurrentComboRadius() const;
private:
    bool bHasHitThisAttack = false;
};
