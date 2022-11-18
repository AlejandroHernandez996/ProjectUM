// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChanneledStateEnum.generated.h"

UENUM(BlueprintType)
enum class EChanneledStateEnum : uint8
{
	IDLE,
	CHANNELING,
};