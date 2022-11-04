// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUmNpc.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ProjectUMProjectile.h"

// Sets default values
AProjectUmNpc::AProjectUmNpc()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

}

// Called when the game starts or when spawned
void AProjectUmNpc::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectUmNpc::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectUmNpc::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AProjectUmNpc, CurrentHealth);
}

void AProjectUmNpc::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AProjectUmNpc::OnHealthUpdate()
{
	FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}
void AProjectUmNpc::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AProjectUmNpc::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

