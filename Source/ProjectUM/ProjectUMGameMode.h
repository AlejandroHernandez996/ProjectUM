// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Http.h"
#include "ProjectUMGameMode.generated.h"

UCLASS(minimalapi)
class AProjectUMGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AProjectUMGameMode();

	virtual void BeginPlay() override;
private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};



