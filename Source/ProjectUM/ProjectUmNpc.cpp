// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUmNpc.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMInventoryComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectUMProjectile.h"
#include "ProjectUMLootTable.h"

// Sets default values
AProjectUmNpc::AProjectUmNpc()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// Create an inventory
	Inventory = CreateDefaultSubobject<UProjectUMInventoryComponent>("Inventory");
	LootTable = CreateDefaultSubobject<UProjectUMLootTable>("LootTable");
	Inventory->Capacity = 20;
	LootTable->OwningNpc = this;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}
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
	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, this->GetActorLocation(), FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
	if (GetLocalRole() == ROLE_Authority && CurrentHealth == 0.0f && Inventory->Items.IsEmpty()) {
			LootTable->GenerateLoot();
			for (auto& Item : LootTable->GetLoot()) {
				Inventory->AddItem(Item);
			}
	}

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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("npc tooK damage"));
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

void AProjectUmNpc::Interact_Implementation(AProjectUMCharacter* Interactor) {

	FString msg = "NPC  " + this->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	Inventory->LootingCharacters.Add(Interactor);
	Interactor->SetLootingInventory(Inventory);
	Interactor->OpenLoot();
	Interactor->BroadcastNpcLoot(Inventory->GetAllInventoryItemIds());

}