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

    // ������Ʈ ����
    HeroComponent = CreateDefaultSubobject<UEDHeroComponent>("HeroComponent");
    MovementComponent = CreateDefaultSubobject<UEDMovementComponent>("MovementComponent");
    CombatComponent = CreateDefaultSubobject<UEDCombatComponent>("CombatComponent");
    AbilityComponent = CreateDefaultSubobject<UEDAbilityComponent>("AbilityComponent");

    // �⺻ ����
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

    GloveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword Mesh"));
    GloveMesh->SetupAttachment(GetMesh(), FName("GloveSocket"));
    CombatComponent->GloveMesh = GloveMesh;

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

    // ������Ʈ �ʱ�ȭ
    if (SpringArm )
    {
        if ( Camera )
        {
            HeroComponent->SetupComponent(SpringArm, Camera);
            MovementComponent->InitializeMovementComponent();
        }
    }
    Super::BeginPlay();

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.AddDynamic(CombatComponent, &UEDCombatComponent::OnMontageEnded);
        }
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

    // Enhanced Input ����
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    // �� ������Ʈ�� �Է� ����
    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        HeroComponent->SetupInput(EnhancedInput);
        MovementComponent->SetupInput(EnhancedInput);
        CombatComponent->SetupInput(EnhancedInput);
        AbilityComponent->SetupInput(EnhancedInput);
    }
}
