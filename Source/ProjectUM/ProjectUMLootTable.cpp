// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectUMLootTable.h"
#include "ProjectUMAssetCache.h"
#include "ProjectUMItem.h"
#include "ProjectUMGameState.h"

// Sets default values for this component's properties
UProjectUMLootTable::UProjectUMLootTable()
{
	MinLoot = 1;
	MaxLoot = 4;
}


// Called when the game starts
void UProjectUMLootTable::BeginPlay()
{
	Super::BeginPlay();

}

void UProjectUMLootTable::GenerateLoot()
{
	TArray<UProjectUMItem*> ItemPool = GetWorld()->GetGameState<AProjectUMGameState>()->AssetCache->ItemCache.Array();
	int LootAmount = FMath::RandRange(MinLoot, MaxLoot);
	int ItemPoolLastIndex = ItemPool.Num()-1;

	for (int i = 0; i < LootAmount; i++) {
		Loot.Add(DuplicateObject<UProjectUMItem>(ItemPool[FMath::RandRange(0, ItemPoolLastIndex)], nullptr));
	}
}

