// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "ProjectUMItem.h"
#include "ProjectUMReturnOrbItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API UProjectUMReturnOrbItem : public UProjectUMItem
{
	GENERATED_BODY()
	
public:
	virtual void Use(class AProjectUMCharacter* CharacterUser) override;

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
