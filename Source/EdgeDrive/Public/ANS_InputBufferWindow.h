// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_InputBufferWindow.generated.h"

/**
 * 
 */
UCLASS(editinlinenew, Blueprintable, meta=(DisplayName = "Input Buffer Window"))
class EDGEDRIVE_API UANS_InputBufferWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_InputBufferWindow();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input Buffer")
	FName BufferWindowTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Buffer")
	TArray<FName> AllowedInputs;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
