// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMItem.h"
#include "CharacterStatEnum.h"

UProjectUMItem::UProjectUMItem()
{
	ItemInventoryWeight = 1.0f;
	ItemName = FText::FromString("PLACEHOLDER_NAME");
	ItemDescription = FText::FromString("PLACEHOLDER_DESCRIPTION");
}

void UProjectUMItem::Use(AProjectUMCharacter* CharacterUser)
{
}