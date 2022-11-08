// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItem.h"
#include "MyProjectUMFoodItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API UMyProjectUMFoodItem : public UProjectUMItem
{
	GENERATED_BODY()
public:
	virtual void Use(class AProjectUMCharacter* CharacterUser) override;
};
