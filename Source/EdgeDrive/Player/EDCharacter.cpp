#include "EDCharacter.h"
#include "../Camera/EDCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "EDHealthComponent.h"

AEDCharacter::AEDCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // �⺻ ����
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // ĳ���� �����Ʈ ������Ʈ ����
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->bOrientRotationToMovement = false;
        MovementComponent->bUseControllerDesiredRotation = false;
        MovementComponent->bConstrainToPlane = true;
        MovementComponent->bSnapToPlaneAtStart = true;
    }

    // ������Ʈ ����
   // HealthComponent = CreateDefaultSubobject<UEDHealthComponent>(TEXT("HealthComponent"));
    CameraComponent = CreateDefaultSubobject<UEDCameraComponent>(TEXT("CameraComponent"));

    if (CameraComponent)
    {
        CameraComponent->SetupAttachment(GetRootComponent());
    }
}

void AEDCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AEDCharacter::ToggleCrouch()
{
    if (CanCrouch())
    {
        if (bIsCrouched)
        {
            UnCrouch();
        }
        else
        {
            Crouch();
        }
    }
}
