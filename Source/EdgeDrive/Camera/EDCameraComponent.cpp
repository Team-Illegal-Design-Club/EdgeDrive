// Fill out your copyright notice in the Description page of Project Settings.


#include "EDCameraComponent.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EDCameraMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EDCameraComponent)

UEDCameraComponent::UEDCameraComponent(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	FieldOfViewOffset = 0.0f;
}

void UEDCameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("EDCameraComponent: %s"), *GetNameSafe(GetTargetActor())));


	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));


	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}

void UEDCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}

void UEDCameraComponent::OnRegister()
{
	Super::OnRegister();
	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UEDCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UEDCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);
	UpdateCameraModes();

	FEDCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	// Keep player controller in sync with the latest view.
	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}
	//Apply Any offset that was added to the field of view.
	CameraModeView.FieldOfView += FieldOfViewOffset;
	FieldOfView = 0.0f;
	//Keep Camera componet in Sync with the Lastest View
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;

	// Fill in desired view.
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}


	if (IsXRHeadTrackedCamera())
	{
		// In XR much of the camera behavior above is irrellevant, but the post process settings are not.
		Super::GetCameraView(DeltaTime, DesiredView);
	}

}

void UEDCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);
	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UEDCameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}
