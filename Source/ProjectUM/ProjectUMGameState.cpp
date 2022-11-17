// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMGameState.h"
#include "ProjectUMAssetCache.h"
#include "Engine/ObjectLibrary.h"
#include "ProjectUMItem.h"

AProjectUMGameState::AProjectUMGameState()
{
	
}

void AProjectUMGameState::BeginPlay() {

	Super::BeginPlay();
}

UProjectUMItem* AProjectUMGameState::GetItemById(int32  ItemId){

	for (auto& Item : AssetCache->ItemCache) {
		if (Item && Item->ItemId == ItemId) {
			return Item;
		}
	}
	return nullptr;
}