// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProjectUMEquippableWeapon.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMInventoryComponent.h"

void UMyProjectUMEquippableWeapon::Use(class AProjectUMCharacter* CharacterUser) {

	if (CharacterUser->GetInventory()->HandSlot == nullptr) {
		CharacterUser->GetInventory()->EquipItem(this, EquipSlot);
		CharacterUser->AttachWeapon(WeaponClass);
	}
	else {
		CharacterUser->GetInventory()->UnEquipItem(this, EquipSlot);
		CharacterUser->DeAttachWeapon();
	}
}