// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableSlotsEnum.generated.h"

UENUM(BlueprintType)
enum class EEquippableSlotsEnum : uint8
{
	HEAD,
	CHEST,
	LEGS,
	HAND,
	SHIELD
};