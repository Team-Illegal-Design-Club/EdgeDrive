#include "EDCharacter.h"
#include "EDHeroComponent.h"
#include "EDMovementComponent.h"
#include "EDCombatComponent.h"
#include "EDAbilityComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

AEDCharacter::AEDCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 컴포넌트 생성
    HeroComponent = CreateDefaultSubobject<UEDHeroComponent>("HeroComponent");
    MovementComponent = CreateDefaultSubobject<UEDMovementComponent>("MovementComponent");
    CombatComponent = CreateDefaultSubobject<UEDCombatComponent>("CombatComponent");
    AbilityComponent = CreateDefaultSubobject<UEDAbilityComponent>("AbilityComponent");

    // 기본 설정
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->bUsePawnControlRotation = true;

    Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
    CombatComponent->LimbMeshes.Empty();
    LeftHandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandMesh"));
    LeftHandMesh->SetupAttachment(GetMesh(), FName("hand_l"));

    RightHandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandMesh"));
    RightHandMesh->SetupAttachment(GetMesh(), FName("hand_r"));

    LeftFootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftFootMesh"));
    LeftFootMesh->SetupAttachment(GetMesh(), FName("calf_l"));

    RightFootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightFootMesh"));
    RightFootMesh->SetupAttachment(GetMesh(), FName("calf_r"));

    // CombatComponent에 메시 컴포넌트 연결


}
UEDMovementComponent* AEDCharacter::GetEDMovementComponent()
{
    return MovementComponent;
}

UEDCombatComponent* AEDCharacter::GetEDCombatComponent()
{
    return CombatComponent;
}

void AEDCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 컴포넌트 초기화
    if (SpringArm )
    {
        if ( Camera )
        {
            HeroComponent->SetupComponent(SpringArm, Camera);
            MovementComponent->InitializeMovementComponent();
        }
    }
    if (CombatComponent)
    {
        CombatComponent->LimbMeshes.Add(EAttackLimb::LeftHand, LeftHandMesh);
        CombatComponent->LimbMeshes.Add(EAttackLimb::RightHand, RightHandMesh);
        CombatComponent->LimbMeshes.Add(EAttackLimb::LeftFoot, LeftFootMesh);
        CombatComponent->LimbMeshes.Add(EAttackLimb::RightFoot, RightFootMesh);
    }



}

void AEDCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AbilityComponent->UpdateLockOnCamera(DeltaTime);
}

void AEDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Enhanced Input 설정
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    // 각 컴포넌트의 입력 설정
    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        HeroComponent->SetupInput(EnhancedInput);
        MovementComponent->SetupInput(EnhancedInput);
        CombatComponent->SetupInput(EnhancedInput);
        AbilityComponent->SetupInput(EnhancedInput);
    }
}
