// Fill out your copyright notice in the Description page of Project Settings.


#include "EDUICameraManager.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EDUICameraManager)

class AActor;
class FDebugDisplayInfo;

UEDUICameraManager* UEDUICameraManager::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (AEDPlayerCameraManager* PCCamera = Cast<AEDPlayerCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}

	return nullptr;
}

// Sets default values for this component's properties
UEDUICameraManager::UEDUICameraManager()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register "showdebug" hook.
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

void UEDUICameraManager::InitializeComponent()
{
	Super::InitializeComponent();
}

void UEDUICameraManager::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<AEDPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool UEDUICameraManager::NeedsToUpdateViewTarget() const
{
	return false;
}

void UEDUICameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
}

void UEDUICameraManager::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}
