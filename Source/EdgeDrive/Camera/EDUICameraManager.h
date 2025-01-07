// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDUICameraManager.generated.h"
class AEDPlayerCameraManager;

class AActor;
class AHUD;
class APlayerController;
class FDebugDisplayInfo;
class UCanvas;
class UObject;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EDGEDRIVE_API UEDUICameraManager : public UActorComponent
{
	GENERATED_BODY()
public:
	static UEDUICameraManager* GetComponent(APlayerController* PC);
public:	
	// Sets default values for this component's properties
	UEDUICameraManager();
	virtual void InitializeComponent() override;

	bool InSettingViewTarget() const { return bUpdatingViewTarget; }
	AActor* GetViewTarget() const { return ViewTarget; }
	void SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

	bool NeedsToUpdateViewTarget() const;
	void UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime);

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);


private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> ViewTarget;

	UPROPERTY(Transient)
	bool bUpdatingViewTarget;
		
};
