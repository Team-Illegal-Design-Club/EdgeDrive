// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EDCharacterMovement.generated.h"

class UObject;
struct FFrame;

//�ٸ� ����̳� �ڵ忡�� �� �±׸� ����� �� �ְ� ���� ������ ������ ���� �±� �ý����� ����(Gameplay.Movement.Stopped ����) ��Ÿ�ӿ� �±� ��Ī�� �˻��� ������ ȿ����
EDGEDRIVE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);
/**
 * FEDCharacterGroundInfo
 * 
 *	Information about the ground under the character.  It only gets updated as needed.
 */

USTRUCT(BlueprintType)
struct FEDCharacterGroundInfo
{
	GENERATED_BODY()
	
	FEDCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{
	}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 * UEDCharacterMovement
 *
 *	The base character movement component class used by this project.
 */
UCLASS(Config = Game)
class EDGEDRIVE_API UEDCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UEDCharacterMovement(const FObjectInitializer& ObjectInitializer);
	virtual void SimulateMovement(float DeltaTime) override;
	virtual bool CanAttemptJump() const override;
	//Return the current ground info. Calling this will update the ground info if it's out of date.
	const FEDCharacterGroundInfo& GetGroundInfo();
	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovemnetComponent interface
protected:
	virtual void InitializeComponent() override;
protected:
	//Cached ground info for the character. Do not access this directly! it's only updated when accessed via GetGroundInfo()
	FEDCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;


};
