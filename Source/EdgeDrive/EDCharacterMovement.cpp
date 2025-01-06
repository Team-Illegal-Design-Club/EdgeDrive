// Fill out your copyright notice in the Description page of Project Settings.
#include "EDCharacterMovement.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

/*
* $TODO
UE_INLINE_GENERATED_CPP_BY_NAME�� �𸮾� ���� 5.1���� ���Ե� ��ũ�η�, ������ �ð��� �����Ű�� ���� ���˴ϴ�
�� ��ũ�δ� .gen.cpp ������ �ش� Ŭ������ �ζ������� ���Խ�ŵ�ϴ�6
�����Ϸ��� cpp ���ϵ��� �м��� �� �ð��� ��������ݴϴ�6
�ݵ�� cpp ������ ������ include�� ����ؾ� �մϴ�9
���ǻ���
��ũ�� ���� �Ķ���ʹ� �ݵ�� �ش� Ŭ������ �̸��� ��ġ�ؾ� �մϴ�
������ Ŭ������ ���� �� �� ����ϸ� LNK2005 ��ũ ������ �߻��� �� �ֽ��ϴ�*/
#include UE_INLINE_GENERATED_CPP_BY_NAME(EDCharacterMovement)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_MovementStopped, "Gameplay.MovementStopped");

namespace EDCharacter
{
	static float GroundTraceDistance = 100000.f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("EDCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};
UEDCharacterMovement::UEDCharacterMovement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEDCharacterMovement::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}
bool UEDCharacterMovement::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.

}
void UEDCharacterMovement::InitializeComponent()
{
	Super::InitializeComponent();
}
const FEDCharacterGroundInfo& UEDCharacterMovement::GetGroundInfo()
{

	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}
	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - EDCharacter::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UEDCharacterMovement_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);
		
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);


		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = EDCharacter::GroundTraceDistance;
		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight),0.f);
		}
	
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;
	return CachedGroundInfo;
}


void UEDCharacterMovement::SetReplicatedAcceleration(const FVector& InAccelration) 
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAccelration; 
}


FRotator UEDCharacterMovement::GetDeltaRotation(float DeltaTime) const
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Gameplay_MovementStopped))
		{
			return FRotator(0, 0, 0);
		}
	}
	return Super::GetDeltaRotation(DeltaTime);
}

float UEDCharacterMovement::GetMaxSpeed() const
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Gameplay_MovementStopped))
		{
			return 0;
		}

	}

	return Super::GetMaxSpeed();
}