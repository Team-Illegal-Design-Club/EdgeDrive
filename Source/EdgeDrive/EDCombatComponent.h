#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "EDCombatComponent.generated.h"

UCLASS()
class EDGEDRIVE_API UEDCombatComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
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

    UTimelineComponent* AttackTimeline;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UCurveFloat* AttackCurve;


    void SetAttackTimelineSpeed(float Speed);

    FOnTimelineFloat AttackTimelineProgress;
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
    UFUNCTION()
    void ResetCombo();

    UFUNCTION(BlueprintCallable)
    float GetCurrentComboDamage() const;

    UFUNCTION(BlueprintCallable)
    float GetCurrentComboRadius() const;
private:
    bool bHasHitThisAttack = false;
};


UENUM(BlueprintType)
enum class EDamageType : uint8
{
    None UMETA(DisplayName = "None"),
    Melee UMETA(DisplayName = "Melee"),
    Projectile UMETA(DisplayName = "Projectile"),
    Explosion UMETA(DisplayName = "Explosion"),
    Environment UMETA(DisplayName = "Environment")
};

UENUM(BlueprintType)
enum class EDamageResponse : uint8
{
    None UMETA(DisplayName = "None"),
    HitReact UMETA(DisplayName = "HitReact"),
    Stagger UMETA(DisplayName = "Stagger"),
    Stun UMETA(DisplayName = "Stun"),
    KnockBack UMETA(DisplayName = "KnockBack")
};

UCLASS(BlueprintType)
class UEDDamageType : public UDamageType
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDamageType DamageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDamageResponse DamageResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldDamageInvincible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeParried;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldForceInterrupt;

    UEDDamageType()
    {
        Amount = 0.0f;
        DamageType = EDamageType::None;
        DamageResponse = EDamageResponse::None;
        bShouldDamageInvincible = false;
        bCanBeBlocked = false;
        bCanBeParried = false;
        bShouldForceInterrupt = true;
    }
};