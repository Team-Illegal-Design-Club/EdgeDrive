#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDAbilityComponent.generated.h"

UINTERFACE(MinimalAPI)
class ULockOnInterface : public UInterface
{
    GENERATED_BODY()
};

class ILockOnInterface
{
    GENERATED_BODY()
public:
};

UCLASS()
class EDGEDRIVE_API UEDAbilityComponent : public UActorComponent
{
    GENERATED_BODY()
private:
    UPROPERTY()
    bool bIsLockingOn;

    UPROPERTY()
    AActor* LockedTarget;
protected:
    UPROPERTY(EditAnywhere, Category = "Lock On")
    float LockOnRadius = 100.f;

    UPROPERTY(EditAnywhere, Category = "Lock On")
    float LockOnRotationSpeed = 10.f;

    UPROPERTY(EditAnywhere, Category = "Lock On")
    class UInputAction* LockOnAction;

    UPROPERTY(EditAnywhere, Category = "EnhancedInput")
    class UInputAction* AbilityAction;



    UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
    float PerfectDodgeTimeWindow = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
    float SlowMotionScale = 0.2f;

    UPROPERTY()
    bool bIsPerfectDodge;

    UPROPERTY()
    class UNiagaraComponent* AfterImageEffect;

    UPROPERTY(EditAnywhere, Category = "Perfect Dodge")
    class UNiagaraSystem* AfterImageTemplate;

    UPROPERTY()
    FTimerHandle PerfectDodgeTimer;

public:
    UEDAbilityComponent();
    // EDAbilityComponent.h 
    UFUNCTION()
    void ToggleLockOn(const FInputActionValue& Value);
    void SetupInput(class UEnhancedInputComponent* PlayerInputComponent);

    void UpdateLockOnCamera(float DeltaTime);
    AActor* FindNearestTarget();
    void CheckPerfectDodge();
    void EnablePerfectDodgeEffects();
    void DisablePerfectDodgeEffects();
};
