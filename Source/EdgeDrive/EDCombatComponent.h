#pragma once
#include "CoreMinimal.h"
#include "EDMovementComponent.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "EDCombatComponent.generated.h"

USTRUCT(BlueprintType)
struct FCommitState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCommitted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CommittedAction = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CommitEndTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted = false;
};

UENUM(BlueprintType)
enum class EAttackLimb : uint8
{
    LeftHand UMETA(DisplayName = "Left Hand"),
    RightHand UMETA(DisplayName = "Right Hand"),
    LeftFoot UMETA(DisplayName = "Left Foot"),
    RightFoot UMETA(DisplayName = "Right Foot")
};
USTRUCT(BlueprintType)
struct FAdvancedInputSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BaseBufferDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MinBufferDuration = 0.067f; // 4 frames at 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    float PenaltyDecayTime = 0.5f; // 30 frames at 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    int32 MaxPenaltyStacks = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penalty")
    float PenaltyMultiplier = 0.5f;
};
USTRUCT(BlueprintType)
struct FInputWithTiming
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanOverrideCommit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsumed = false;
};


USTRUCT(BlueprintType)
struct FBufferedInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName InputName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConsumed = false;

    FBufferedInput()
        : InputName(NAME_None), TimeStamp(0.0f), bIsConsumed(false) {
    }

    FBufferedInput(FName InInputName, float InTimeStamp)
        : InputName(InInputName), TimeStamp(InTimeStamp), bIsConsumed(false) {
    }
};
USTRUCT(BlueprintType)
struct FInputBufferWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BufferTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AllowedInputs;

    FInputBufferWindow()
        : bIsOpen(false), BufferTag(NAME_None) {
    }
};
USTRUCT(BlueprintType)
struct FComboAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAttackLimb AttackLimb;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;
};
UCLASS()
class EDGEDRIVE_API UEDCombatComponent : public UActorComponent
{
    GENERATED_BODY()


protected:
    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere, Category = "Combat|Hitstop")
    float HitStopDuration = 0.1f;  // 히트스톱 지속 시간

    UPROPERTY(EditAnywhere, Category = "Combat|Hitstop")
    float HitStopTimeDilation = 0.1f;  // 히트스톱 시 시간 배율
    UPROPERTY(EditAnywhere, Category = "Combat")
    float ComboTimeWindow = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ComboResetTime = 2.0f;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
    TArray<FComboAttackData> ComboAttacks;


    UPROPERTY(EditAnywhere, Category = "Combat|Sockets")
    TMap<EAttackLimb, FName> StartSocketNames;

    UPROPERTY(EditAnywhere, Category = "Combat|Sockets")
    TMap<EAttackLimb, FName> EndSocketNames;
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
    bool bIsAttacking= false;

    UPROPERTY(EditAnywhere)
    float Damage;

    UPROPERTY(EditAnywhere, Category = "Effects")
    class UNiagaraSystem* AfterImageTemplate;

    UTimelineComponent* AttackTimeline;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UCurveFloat* AttackCurve;
    UPROPERTY(EditAnywhere, Category = "Combat|Input Buffer")
    float InputBufferDuration = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Combat|Input Buffer")
    int32 MaxBufferedInputs = 3;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Input Buffer")
    TArray<FBufferedInput> InputBuffer;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Input Buffer")
    FInputBufferWindow CurrentBufferWindow;

    // Input Buffer Functions
    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    void AddInputToBuffer(FName InputName);

    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    bool HasBufferedInput(FName InputName);

    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    FBufferedInput ConsumeBufferedInput(FName InputName);
public:
    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    void OpenInputBufferWindow(FName BufferTag, const TArray<FName>& AllowedInputs);
    
    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    void CloseInputBufferWindow();
protected:
    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    void ProcessBufferedInputs();

    UFUNCTION(BlueprintCallable, Category = "Combat|Input Buffer")
    void ClearInputBuffer();
    // Advanced Input Buffer System
    UPROPERTY(EditAnywhere, Category = "Combat|Advanced Input Buffer")
    FAdvancedInputSettings InputSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Advanced Input Buffer")
    TArray<FInputWithTiming> AdvancedInputBuffer;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Advanced Input Buffer")
    FCommitState CurrentCommitState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Advanced Input Buffer")
    int32 InputSpamPenalty = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Advanced Input Buffer")
    float LastInputTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Advanced Input Buffer")
    bool bUseSekiroPenaltySystem = true;

    UPROPERTY(EditAnywhere, Category = "Combat|Advanced Input Buffer")
    bool bUseEldenRingCommitSystem = true;

    // Deflect/Parry System (Sekiro Style)
    UPROPERTY(EditAnywhere, Category = "Combat|Deflect")
    float DeflectWindow = 0.2f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Deflect")
    bool bCanDeflect = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Deflect")
    float CurrentDeflectWindow = 0.2f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Combat|Advanced Input Buffer")
    void AddAdvancedInputToBuffer(FName InputName, float Priority = 1.0f, bool bCanOverrideCommit = false);

    UFUNCTION(BlueprintCallable, Category = "Combat|Advanced Input Buffer")
    bool ProcessAdvancedBuffer();

    UFUNCTION(BlueprintCallable, Category = "Combat|Advanced Input Buffer")
    void CommitToAction(FName ActionName, float Duration, bool bCanBeInterrupted = false);

    UFUNCTION(BlueprintCallable, Category = "Combat|Deflect")
    void StartDeflectWindow();

    UFUNCTION(BlueprintCallable, Category = "Combat|Deflect")
    void EndDeflectWindow();

    UFUNCTION(BlueprintCallable, Category = "Combat|Deflect")
    bool AttemptDeflect();

private:
    void UpdateInputPenalty();
    float CalculateCurrentBufferDuration() const;
    void CleanupAdvancedBuffer();
    bool IsActionCommitted() const;
private:
    void CleanupExpiredInputs();
    bool IsInputAllowedInCurrentWindow(FName InputName) const;

    void SetAttackTimelineSpeed(float Speed);

    FOnTimelineFloat AttackTimelineProgress;


public:
    UEDCombatComponent();

    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);
    void StartAttack();

    UFUNCTION(BlueprintCallable)
    void LineTrace();
    void PlayHitEffect(const FVector& HitLocation);

 

    UFUNCTION(BlueprintCallable)
    void EnableComboWindow();

    UFUNCTION(BlueprintCallable)
    void DisableComboWindow();
    UFUNCTION()
    void ResetCombo();
    UPROPERTY()
    TMap<EAttackLimb, UStaticMeshComponent*> LimbMeshes;


    UFUNCTION(BlueprintCallable)
    float GetCurrentComboDamage() const;

    UFUNCTION(BlueprintCallable)
    float GetCurrentComboRadius() const;
  


private:
    UPROPERTY()
    UEDMovementComponent* MovementComponent;
    bool bHasHitThisAttack = false;
    FVector CalculateAttackDirection(ACharacter* Character);
    FTimerHandle HitStopTimerHandle;
    FTimerHandle DeflectTimerHandle;
    float OriginalTimeDilation=1;

    // 히트스톱 관련 함수들
    UFUNCTION()
    void ApplyHitStop();

    UFUNCTION()
    void ResetHitStop();
    UFUNCTION()
    void ExecuteBufferedAction(FName ActionName);
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