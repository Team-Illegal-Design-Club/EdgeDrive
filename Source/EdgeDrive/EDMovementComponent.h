#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "InputBufferTypes.h"
#include "EDMovementComponent.generated.h"


USTRUCT(BlueprintType)
struct FMotionMatchingParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrajectoryPredictionTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PoseMatchingWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VelocityMatchingWeight = 0.4f;
};
UCLASS()
class EDGEDRIVE_API UEDMovementComponent : public UActorComponent
{
    GENERATED_BODY()

    UPROPERTY()
    class UEDAbilityComponent* AbilityComponent;

    UPROPERTY()
    class UEDCombatComponent* CombatComponent;
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkSpeed = 500.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SprintSpeed = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float DodgeDistance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float DodgeCooldown = 0.5f;

    // Input Buffer System
    UPROPERTY(EditAnywhere, Category = "Movement|Input Buffer")
    FInputBufferSettings MovementBufferSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Movement|Input Buffer")
    TArray<FBaseBufferedInput> MovementInputBuffer;

    // Sekiro Style Penalty System
    UPROPERTY(BlueprintReadOnly, Category = "Movement|Penalty System")
    TMap<FName, int32> InputPenalties;

    UPROPERTY(BlueprintReadOnly, Category = "Movement|Penalty System")
    TMap<FName, float> LastInputTimes;
    // Enhanced Input
    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* DodgeAction;
    // Timeline Components
    UTimelineComponent* DodgeTimeline;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    UCurveFloat* DodgeCurve;

    UFUNCTION()
    void OnDodgeEnd();

    // Motion Matching Animations
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimationAsset* DodgeForwardAnim;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimationAsset* DodgeBackwardAnim;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimationAsset* DodgeLeftAnim;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimationAsset* DodgeRightAnim;
    void SetDodgeTimelineSpeed(float Speed);

    FOnTimelineFloat DodgeTimelineProgress;
    bool bIsWalking = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsSprint = false;

    bool bCanDodge = true;
    FTimerHandle DodgeCooldownTimer;
    
private:
    FTimerHandle SprintEndCooldownTimer;
    bool bCanDodgeAfterSprint = true;
    float SprintEndCooldown = 0.02f;

    // Motion Matching
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimMontage* DodgeMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    FMotionMatchingParams MotionMatchingParams;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    float BlendInTime = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    float BlendOutTime = 0.15f;


    //void UpdateMotionMatching();

    UAnimMontage* SelectBestDodgeAnimation();

    void CleanupMovementBuffer();
    void UpdateInputPenalty(FName InputName);
    int32 GetInputPriority(FName InputName) const;
    void ExecuteMovementInput(const FBaseBufferedInput& Input);
    void ExecuteDodgeLogic();
    void ExecuteSprintLogic();
public:
    UEDMovementComponent();
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsDodge() const { return bIsDodge; }

    UPROPERTY(BlueprintReadWrite)
    bool b2DModeEnabled = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsDodge = false;

    void InitializeMovementComponent();

    UFUNCTION()
    void OnDodgeTimelineProgress(float Value);

    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);
    void MoveInput(const FInputActionValue& Value);
    void StartSprint();

    UFUNCTION(BlueprintCallable)
    void EndSprint();
    FVector CalculateDodgeDirection(ACharacter* Character);
    FVector2D CurrentMovementInput;
    void Dodge(const FInputActionValue& Value);


    UFUNCTION(BlueprintCallable)
    bool IsSprint() const;
    UFUNCTION(BlueprintCallable)
    bool IsWalk() const;
    UFUNCTION(BlueprintCallable)
    bool IsFalling() const;

	// Input Buffer Public Functions
	UFUNCTION(Blueprintcallable, Category = "Movement|Input Buffer")
    void AddMovementInputToBuffer(FName InputName, int32 Priority = 1, FVector2D InputData = FVector2D::ZeroVector);
    UFUNCTION(BlueprintCallable, Category = "Movement|Input Buffer")
    bool ProcessMovementBuffer();
    UFUNCTION(BlueprintCallable, Category = "Movement|Input Buffer")
    void ClearMovementBuffer();

    UFUNCTION(BlueprintCallable, Category = "Movement|Input Buffer")
    bool CanExecuteMovementInput(FName InputName) const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Input Buffer")
    void SetMovementBufferSettings(const FInputBufferSettings& NewSettings);
};