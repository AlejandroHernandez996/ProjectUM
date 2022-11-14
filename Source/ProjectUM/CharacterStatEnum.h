// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStatEnum.generated.h"

UENUM(BlueprintType)
enum class ECharacterStatEnum : uint8
{
	HEALTH,
	MANA,
	AGILITY,
	STRENGTH,
	INTELLECT,
	WISDOM
};