// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectUMItemGenerator.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UProjectUMItemGenerator : public UInterface
{
	GENERATED_BODY()
};

class PROJECTUM_API IProjectUMItemGenerator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Generate(AProjectUMCharacter* CharacterToOutput);
};
