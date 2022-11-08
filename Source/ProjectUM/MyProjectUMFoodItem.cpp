// Fill out your copyright notice in the Description page of Project Settings.


#include "MyProjectUMFoodItem.h"
#include "ProjectUMCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"
#include "ProjectUMInventoryComponent.h"

void UMyProjectUMFoodItem::Use(class AProjectUMCharacter* CharacterUser) {

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("EATING FOOD"));
	if (CharacterUser->GetLocalRole() == ROLE_Authority) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("AUTH"));
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("NOT AUTH"));
	}
	UGameplayStatics::ApplyDamage(CharacterUser, -10.0f, nullptr, nullptr, UDamageType::StaticClass());

	OwningInventory->RemoveItem(this);
}