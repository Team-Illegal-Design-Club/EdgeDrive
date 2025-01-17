#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "EDMovementComponent.generated.h"

UCLASS()
class EDGEDRIVE_API UEDMovementComponent : public UActorComponent
{
    GENERATED_BODY()

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

    void SetDodgeTimelineSpeed(float Speed);
    
    FOnTimelineFloat DodgeTimelineProgress;

    bool bIsSprint = false;
    bool bCanDodge = true;
    FTimerHandle DodgeCooldownTimer;
    FVector2D CurrentMovementInput;

public:
    UEDMovementComponent();
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsDodge() const { return bIsDodge; }
    UPROPERTY(BlueprintReadWrite)
    bool b2DModeEnabled = false;
    UPROPERTY()
    bool bIsDodge = false;
    void InitializeMovementComponent();
    UFUNCTION()
    void OnDodgeTimelineProgress(float Value);

    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);
    void MoveInput(const FInputActionValue& Value);
    void StartSprint();
    void EndSprint();
    void Dodge(const FInputActionValue& Value);
 

    UFUNCTION(BlueprintCallable)
    bool IsSprint() const;

    UFUNCTION(BlueprintCallable)
    bool IsFalling() const;
};
