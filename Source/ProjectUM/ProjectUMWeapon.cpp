// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMWeapon.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AProjectUMWeapon::AProjectUMWeapon()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Weapon");
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName("NoCollision");
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetNotifyRigidBodyCollision(false);

	HitboxComponent = CreateDefaultSubobject<USphereComponent>("Hitbox");
	HitboxComponent->SetupAttachment(MeshComponent);
	HitboxComponent->SetCollisionProfileName("NoCollision");
	HitboxComponent->SetHiddenInGame(false);
	HitboxComponent->SetNotifyRigidBodyCollision(false);
	bReplicates = true;
}

// Called when the game starts or when spawned
void AProjectUMWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}