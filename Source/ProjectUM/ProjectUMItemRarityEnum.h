// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItemRarityEnum.generated.h"

UENUM(BlueprintType)
enum class EProjectUMItemRarityEnum : uint8
{
	COMMON UMETA(DisplayName = "COMMON"),
	UNCOMMON = 1  UMETA(DisplayName = "UNCOMMON"),
	RARE = 2 UMETA(DisplayName = "RARE"),
	EPIC = 3 UMETA(DisplayName = "EPIC"),
	LEGENDARY = 4 UMETA(DisplayName = "LEGENDARY")
};
