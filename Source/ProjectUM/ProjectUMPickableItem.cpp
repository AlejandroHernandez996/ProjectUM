// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMPickableItem.h"
#include "ProjectUMItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProjectUMCharacter.h"

// Sets default values
AProjectUMPickableItem::AProjectUMPickableItem()
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
}

void AProjectUMPickableItem::Interact_Implementation(AProjectUMCharacter* Interactor)
{
	FString msg = "PICKING UP " + this->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	Interactor->AddItemToInventory(Item);
	Destroy();
}

// Called when the game starts or when spawned
void AProjectUMPickableItem::BeginPlay()
{
	Super::BeginPlay();
	if (Item) {
		StaticMesh->SetStaticMesh(Item->PickupMesh);
	}
}