// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMItem.h"
#include "EquippableSlotsEnum.h"
#include "ProjectUMEquippableItem.generated.h"

UCLASS()
class PROJECTUM_API UProjectUMEquippableItem : public UProjectUMItem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EEquippableSlotsEnum EquipSlot = EEquippableSlotsEnum::HEAD;

	virtual void Use(class AProjectUMCharacter* CharacterUser) override;
};
