// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMWeapon.h"
#include "Components/SphereComponent.h"

// Sets default values
AProjectUMWeapon::AProjectUMWeapon()
{
	WeaponComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollider"));
	WeaponComponent->SetupAttachment(RootComponent);
	WeaponComponent->SetCollisionProfileName("NoCollision");
	WeaponComponent->SetHiddenInGame(false);
	WeaponComponent->SetNotifyRigidBodyCollision(false);
	bReplicates = true;
}

// Called when the game starts or when spawned
void AProjectUMWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void AProjectUMWeapon::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

