// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMEquipment.h"
#include "ProjectUMWeapon.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMWeapon : public AProjectUMEquipment
{
	GENERATED_BODY()

	// Sets default values for this character's properties
public:
	AProjectUMWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
