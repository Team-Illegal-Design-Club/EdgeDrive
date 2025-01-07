// Fill out your copyright notice in the Description page of Project Settings.
#include "EDCharacterMovement.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

/*
* $TODO
UE_INLINE_GENERATED_CPP_BY_NAME는 언리얼 엔진 5.1부터 도입된 매크로로, 컴파일 시간을 단축시키기 위해 사용됩니다
이 매크로는 .gen.cpp 파일을 해당 클래스에 인라인으로 포함시킵니다6
컴파일러가 cpp 파일들을 분석할 때 시간을 단축시켜줍니다6
반드시 cpp 파일의 마지막 include로 사용해야 합니다9
주의사항
매크로 안의 파라미터는 반드시 해당 클래스의 이름과 일치해야 합니다
동일한 클래스에 대해 두 번 사용하면 LNK2005 링크 에러가 발생할 수 있습니다*/
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