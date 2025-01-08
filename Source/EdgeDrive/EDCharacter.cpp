// Fill out your copyright notice in the Description page of Project Settings.


#include "EDCharacter.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


AEDCharacter::AEDCharacter()
{
	// Enable tick by default for smooth movement
	PrimaryActorTick.bCanEverTick = true;

	// Set default movement values
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	GetCharacterMovement()->bOrientRotationToMovement = false;

	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
		MovementComponent->JumpZVelocity = 500.f;
		MovementComponent->AirControl = 0.35f;
		MovementComponent->MaxWalkSpeed = 500.f;
		MovementComponent->MinAnalogWalkSpeed = 20.f;
		MovementComponent->BrakingDecelerationWalking = 2000.f;
	}

	GloveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword Mesh"));
	GloveMesh->SetupAttachment(GetMesh(), FName("GloveSocket"));
}
 
void AEDCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Verify input mapping and actions are set
	if (!InputMapping || !MoveAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("Input mapping or actions not set in EDCharacter"));
	}
}

void AEDCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

	// Update character state
	if (GetCharacterMovement()->IsFalling())
	{
		CurrentState = ECharacterState::Falling;
	}
	else if (GetCharacterMovement()->Velocity.SizeSquared() > 0)
	{
		CurrentState = ECharacterState::Walking;
	}
	else
	{
		CurrentState = ECharacterState::Idle;
	}
}

void AEDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);

		}
	}

	if (UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEDCharacter::MoveInput);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEDCharacter::LookInput);
		Input->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AEDCharacter::StartSprint);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AEDCharacter::EndSprint);
		Input->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AEDCharacter::StartAttack);
	//	Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AEDCharacter::Jump);

	}
}
void AEDCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}
void AEDCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AEDCharacter::EndSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

}
void AEDCharacter::StartAttack()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Pressed input action ");
	// Call Attack animation
	if (AttackAnim&& !bIsAttacking)
	{
		GetMesh()->PlayAnimation(AttackAnim, false);
		bIsAttacking = true;
	}
}
void AEDCharacter::LineTrace()
{
	// Deal damage to enemys in range
	FVector StartLocation = GloveMesh->GetSocketLocation(FName("Start"));
	FVector EndLocation = GloveMesh->GetSocketLocation(FName("End"));

	//Set Up Linetrace
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	//Debug Lines
	 DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false , 1, 0,1);
	//Linetrace
	GetWorld()->LineTraceSingleByChannel(HitResult,StartLocation, EndLocation, ECC_Visibility, TraceParams);
	if (HitResult.bBlockingHit)
	{
		AActor* ActorHit = HitResult.GetActor();
		ActorHit->Destroy();
	}
}
void AEDCharacter::MoveInput(const FInputActionValue& Value)
{
	/*GEngine->AddOnScreenDebugMessage(-1,1.f, FColor::Red, "Pressed input action ");*/
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

