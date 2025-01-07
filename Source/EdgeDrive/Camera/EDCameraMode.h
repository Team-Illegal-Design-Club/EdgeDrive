// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "EDCameraMode.generated.h"

class Actor;
class UCanvas;
class UEDCameraComponent;

/**
 * EEDCameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EEDCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)

};

/**
 * FEDCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FEDCameraModeView
{
public:

	FEDCameraModeView();

	void Blend(const FEDCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};



/**
 * UEDCameraMode
 *
 *	Base class for all camera modes.
 */
UCLASS()
class EDGEDRIVE_API UEDCameraMode : public UObject
{
	GENERATED_BODY()
public:
	UEDCameraMode();
	UEDCameraComponent* GetEDCameraComponent() const;
	virtual UWorld* GetWorld() const override;
	AActor* GetTargetActor() const;
	const FEDCameraModeView& GetCameraModeView() const { return View; }
	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation() {};
	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation() {};

	void UpdateCameraMode(float DeltaTime);
	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const{ return BlendWeight; }
	void SetBlendWeight(float Weight);
	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}
	virtual void DrawDebug(UCanvas* Canvas) const;
protected:

	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);
protected:
	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;
	// View output produced by the camera mode.
	FEDCameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;
	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;
	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;
	// How long it takes to blend in this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;
	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EEDCameraModeBlendFunction BlendFunction;
	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;
protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation : 1;
};
/**
 * ULyraCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class UEDCameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UEDCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UEDCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FEDCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	UEDCameraMode* GetCameraModeInstance(TSubclassOf<UEDCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FEDCameraModeView& OutCameraModeView) const;

protected:

	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UEDCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UEDCameraMode>> CameraModeStack;
};
