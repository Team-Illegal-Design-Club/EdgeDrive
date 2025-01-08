// EDHeroComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "InputActionValue.h"
#include "EDHeroComponent.generated.h"

class UInputComponent;
class UEDCameraMode;
class UInputAction;
class UInputMappingContext;

UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class EDGEDRIVE_API UEDHeroComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    UEDHeroComponent(const FObjectInitializer& ObjectInitializer);

    static UEDHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UEDHeroComponent>() : nullptr); }

    void SetAbilityCameraMode(TSubclassOf<UEDCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);
    void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:
    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

    void Input_Move(const FInputActionValue& InputActionValue);
    void Input_LookMouse(const FInputActionValue& InputActionValue);
    void Input_LookStick(const FInputActionValue& InputActionValue);
    void Input_Crouch(const FInputActionValue& InputActionValue);
    void Input_Jump(const FInputActionValue& InputActionValue);
    void Input_Sprint(const FInputActionValue& InputActionValue);

    TSubclassOf<UEDCameraMode> DetermineCameraMode() const;

protected:
    // Input Mapping Context
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // Input Actions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputAction> CrouchAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ED|Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY()
    TSubclassOf<UEDCameraMode> AbilityCameraMode;

    FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

    bool bReadyToBindInputs;
};
