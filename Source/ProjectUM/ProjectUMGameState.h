// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjectUMGameState.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced)
class PROJECTUM_API AProjectUMGameState : public AGameStateBase
{
	GENERATED_BODY()

		virtual void BeginPlay();

public:
	AProjectUMGameState();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, Category = "Cache")
		class UProjectUMAssetCache* AssetCache;

	UFUNCTION(BlueprintCallable, Category = "Game State")
		class UProjectUMItem* GetItemById(int32 ItemId);
};
