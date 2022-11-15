// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectUMResource.h"
#include "Components/BoxComponent.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMItem.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AProjectUMResource::AProjectUMResource()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootComponent"));
	BoxComponent->bHiddenInGame = false;
	BoxComponent->SetCollisionProfileName(TEXT("Resource"));
	RootComponent = BoxComponent;

}

// Called when the game starts or when spawned
void AProjectUMResource::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectUMResource::Tick(float DeltaTime)
{
	if (GetLocalRole() == ROLE_Authority) {
		if (Capacity == 0) {
			Destroy();
		}
	}
	Super::Tick(DeltaTime);

}

void AProjectUMResource::Generate_Implementation(AProjectUMCharacter* CharacterToGenerate)
{
	if (Capacity == 0) return;
	CharacterToGenerate->AddItemToInventory(DuplicateObject(OutputItem, nullptr));
	Capacity--;
}

