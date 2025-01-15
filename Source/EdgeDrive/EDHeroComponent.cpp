#include "EDHeroComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

UEDHeroComponent::UEDHeroComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEDHeroComponent::SetupComponent(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
    SpringArm = InSpringArm;
    Camera = InCamera;

    if (SpringArm)
    {
        SpringArm->TargetArmLength = 300.0f;
        SpringArm->bUsePawnControlRotation = true;
    }

    if (Camera)
    {
        Camera->bUsePawnControlRotation = false;
    }
}
void UEDHeroComponent::SetupInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent) return;

    if (LookAction)
    {
        PlayerInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
            this, &UEDHeroComponent::LookInput);
    }
}

void UEDHeroComponent::LookInput(const FInputActionValue& Value)
{
   if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
        if (!_2DModeEnabled)
        {
  
            const FVector2D LookAxisVector = Value.Get<FVector2D>();
            Pawn->AddControllerYawInput(LookAxisVector.X);
            Pawn->AddControllerPitchInput(-LookAxisVector.Y);
        }
     
    }
}
