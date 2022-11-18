// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChannableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UChannableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTUM_API IChannableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Channel();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Release();
};
