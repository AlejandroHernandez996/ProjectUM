// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectUMReturnOrbItem.h"
#include "ProjectUMCharacter.h"

void UProjectUMReturnOrbItem::Use(AProjectUMCharacter* CharacterUser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "ORB");
	CharacterUser->Disconnect();
}
