// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "NpcState.generated.h"

UENUM(BlueprintType)
enum class ENpcState : uint8
{
	IDLE,
	AGGRESIVE,
	LEASHING,
	DEAD
};