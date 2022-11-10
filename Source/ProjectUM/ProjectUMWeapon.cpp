// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMWeapon.h"
#include "Components/SphereComponent.h"

// Sets default values
AProjectUMWeapon::AProjectUMWeapon()
{
	HitboxComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollider"));
	HitboxComponent->SetupAttachment(RootComponent);
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