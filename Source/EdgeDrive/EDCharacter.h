// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "CoreMinimal.h"
#include "ModularCharacter.h"
#include "EDCharacter.generated.h"

class AActor;
class AController;
class AEDPlayerController;
class AEDPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class UEDAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class EDGEDRIVE_API AEDCharacter : public AModularCharacter
{
	GENERATED_BODY()
	
};
