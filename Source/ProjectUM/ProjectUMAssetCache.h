// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectUMAssetCache.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECTUM_API UProjectUMAssetCache : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced, Category = "Cache")
		TSet<class UProjectUMItem*> ItemCache;

		TMap<int32, class UProjectUMItem*> ItemCacheMap;

};
