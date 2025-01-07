#include "EDCharacter.h"
#include "../Camera/EDCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "EDHealthComponent.h"

AEDCharacter::AEDCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 기본 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // 캐릭터 무브먼트 컴포넌트 설정
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->bOrientRotationToMovement = false;
        MovementComponent->bUseControllerDesiredRotation = false;
        MovementComponent->bConstrainToPlane = true;
        MovementComponent->bSnapToPlaneAtStart = true;
    }

    // 컴포넌트 생성
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
