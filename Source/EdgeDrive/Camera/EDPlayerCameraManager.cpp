// Fill out your copyright notice in the Description page of Project Settings.


#include "EDPlayerCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EDCameraComponent.h"
#include "EDUICameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EDPlayerCameraManager)
class FDebugDisplayInfo;

static FName UICameraComponentName(TEXT("UICamera"));
AEDPlayerCameraManager::AEDPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DefaultFOV = ED_CAMERA_DEFAULT_FOV;
	ViewPitchMin = ED_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = ED_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<UEDUICameraManager>(UICameraComponentName);
}

UEDUICameraManager* AEDPlayerCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

void AEDPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI Camera is looking at something, let it have priority.
	if (UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void AEDPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("LyraPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UEDCameraComponent* CameraComponent = UEDCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}
