#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
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
protected:
    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkSpeed = 500.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SprintSpeed = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float DodgeDistance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float DodgeCooldown = 0.5f;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* DodgeAction;

    UTimelineComponent* DodgeTimeline;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    UCurveFloat* DodgeCurve;
    UFUNCTION()
    void OnDodgeEnd();
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

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    UAnimMontage* DodgeMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    FMotionMatchingParams MotionMatchingParams;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    float BlendInTime = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Motion Matching")
    float BlendOutTime = 0.15f;

private:

    //void UpdateMotionMatching();

    UAnimMontage* SelectBestDodgeAnimation();
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
};