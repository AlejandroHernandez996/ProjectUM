// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMWeapon.h"
#include "ChanneledStateEnum.h"
#include "ChannableInterface.h"
#include "ProjectUMBow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API AProjectUMBow : public AProjectUMWeapon, public IChannableInterface
{
	GENERATED_BODY()
	
public:
	AProjectUMBow();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float FireRate;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float ChannelCurrentTime;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float ChannelStartTime;

	FTimer ChannelTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float MaxChannelTime;

	EChanneledStateEnum ChannelState = EChanneledStateEnum::IDLE;

	bool bIsChanneling;

	FTimerHandle FiringTimer;

	bool bIsFiringWeapon;

	void StartFire();

	void StopFire();

	void HandleFire();
	
	void Channel_Implementation() override;

	void Release_Implementation() override;


protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
