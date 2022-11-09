// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMEquipment.h"

AProjectUMEquipment::AProjectUMEquipment()
{
	bReplicates = true;
}

// Called when the game starts or when spawned
void AProjectUMEquipment::BeginPlay()
{
	Super::BeginPlay();

}

// Called to bind functionality to input
void AProjectUMEquipment::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

