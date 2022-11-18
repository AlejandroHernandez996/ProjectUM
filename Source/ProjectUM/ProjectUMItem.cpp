// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMItem.h"
#include "CharacterStatEnum.h"

UProjectUMItem::UProjectUMItem()
{
	ItemInventoryWeight = 1.0f;
	ItemName = FText::FromString("PLACEHOLDER_NAME");
	ItemDescription = FText::FromString("PLACEHOLDER_DESCRIPTION");

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    char rand_string[10];

    for (int i = 0; i < 10; ++i) {
        rand_string[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    ItemGuid = rand_string;
}

void UProjectUMItem::Use(AProjectUMCharacter* CharacterUser)
{
}