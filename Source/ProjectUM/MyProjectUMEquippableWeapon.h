// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMEquippableItem.h"
#include "ProjectUMWeapon.h"
#include "MyProjectUMEquippableWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API UMyProjectUMEquippableWeapon : public UProjectUMEquippableItem
{
	GENERATED_BODY()
	
public:
	virtual void Use(class AProjectUMCharacter* CharacterUser) override;

	UPROPERTY(EditDefaultsOnly, Category = "Parameters")
		TSubclassOf<AProjectUMWeapon> WeaponClass;
};
