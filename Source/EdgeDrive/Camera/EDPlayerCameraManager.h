// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "EDPlayerCameraManager.generated.h"


class FDebugDisplayInfo;
class UCanvas;
class UObject;

#define ED_CAMERA_DEFAULT_FOV		(80.0f)
#define ED_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define ED_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

class UEDUICameraManager;
/**
 * AEDPlayerCameraManager
 *
 *	The base player camera manager class used by this project.
 */

UCLASS(notplaceable)
class EDGEDRIVE_API AEDPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AEDPlayerCameraManager(const FObjectInitializer& ObjectInitialzer);

	UEDUICameraManager* GetUICameraComponent() const;
protected:
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

private:
	/** The UI Camera Component, controls the camera when UI is doing something important that gameplay doesn't get priority over. */
	UPROPERTY(Transient)
	TObjectPtr<UEDUICameraManager> UICamera;
};