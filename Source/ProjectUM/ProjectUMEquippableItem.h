// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItem.h"
#include "EquippableSlotsEnum.h"
#include "ToolTypeEnum.h"
#include "ProjectUMEquipment.h"
#include "ProjectUMWeaponType.h"
#include "ProjectUMEquippableItem.generated.h"

UCLASS()
class PROJECTUM_API UProjectUMEquippableItem : public UProjectUMItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EEquippableSlotsEnum EquipSlot = EEquippableSlotsEnum::HEAD;

	UPROPERTY(EditDefaultsOnly, Category = "Parameters")
		TSubclassOf<AProjectUMEquipment> EquipmentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EToolTypeEnum ToolType = EToolTypeEnum::NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EProjectUMWeaponType WeaponType = EProjectUMWeaponType::NONE;

	virtual void Use(class AProjectUMCharacter* CharacterUser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
		FQuat BaseRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
		FVector BaseLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
		FVector BaseScale = FVector::One();
};
