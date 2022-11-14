// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterStatEnum.h"
#include "ProjectUMStatsData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API UProjectUMStatsData : public UDataAsset
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
		TMap<ECharacterStatEnum, float> Stats;
};
