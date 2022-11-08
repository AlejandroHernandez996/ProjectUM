// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMWeapon.h"
#include "Components/SphereComponent.h"

// Sets default values
AProjectUMWeapon::AProjectUMWeapon()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollider"));
	WeaponComponent->SetupAttachment(RootComponent);
	WeaponComponent->SetCollisionProfileName("NoCollision");
	WeaponComponent->SetHiddenInGame(false);
	WeaponComponent->SetNotifyRigidBodyCollision(false);
}

// Called when the game starts or when spawned
void AProjectUMWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectUMWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AProjectUMWeapon::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

