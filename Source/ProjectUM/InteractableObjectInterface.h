// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InteractableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTUM_API IInteractableObjectInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(AProjectUMCharacter* Interactor);
};
