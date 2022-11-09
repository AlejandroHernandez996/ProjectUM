// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMEquippableItem.h"
#include "ProjectUMCharacter.h"
#include "Engine/Engine.h"
#include "ProjectUMInventoryComponent.h"

void UProjectUMEquippableItem::Use(class AProjectUMCharacter* CharacterUser) {
	if (this == CharacterUser->GetInventory()->HeadSlot || this == CharacterUser->GetInventory()->ChestSlot ||this == CharacterUser->GetInventory()->LegsSlot)
	{
		CharacterUser->GetInventory()->UnEquipItem(this, EquipSlot);
		CharacterUser->DeAttachArmor(EquipSlot);
	} else {
		CharacterUser->GetInventory()->EquipItem(this, EquipSlot);
		CharacterUser->AttachArmor(EquipmentClass, EquipSlot);
	}
}
