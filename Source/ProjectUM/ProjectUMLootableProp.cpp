// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectUMLootableProp.h"
#include "ProjectUMItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProjectUMInventoryComponent.h"
#include "ProjectUMCharacter.h"

// Sets default values
AProjectUMLootableProp::AProjectUMLootableProp()
{
	bReplicates = true;
	//Definition for the SphereComponent that will serve as the Root component for the projectile and its collision.
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->InitSphereRadius(100.0f);
	SphereComponent->bHiddenInGame = false;
	SphereComponent->SetCollisionProfileName(TEXT("Item"));
	RootComponent = SphereComponent;

	//Definition for the Mesh that will serve as your visual representation.
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetIsReplicated(true);
	StaticMesh->SetCollisionProfileName("NoCollision");
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetRelativeScale3D(FVector(.1f, .1f, .1f));

	Inventory = CreateDefaultSubobject<UProjectUMInventoryComponent>("Inventory");
	Inventory->Capacity = 20;
}

void AProjectUMLootableProp::Interact_Implementation(AProjectUMCharacter* Interactor)
{
	FString msg = "LOOTBOX  " + this->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, msg);
	Inventory->LootingCharacters.Add(Interactor);
	Interactor->SetLootingInventory(Inventory);
	Interactor->OpenLoot();
	Interactor->BroadcastNpcLoot(Inventory->GetAllInventoryItemIds());
}

// Called when the game starts or when spawned
void AProjectUMLootableProp::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority) {
		for (auto& Item : Inventory->DefaultItems) {
			Inventory->AddItem(Item);
		}
	}
}