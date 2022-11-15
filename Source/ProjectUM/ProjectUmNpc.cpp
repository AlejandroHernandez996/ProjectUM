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
#include "Perception/PawnSensingComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"

// Sets default values
AProjectUmNpc::AProjectUmNpc()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initialize the player's Health
	MaxHealth = 100.0f;
	LeashRange = 2000.0f;
	CurrentHealth = MaxHealth;

	// Create an inventory
	Inventory = CreateDefaultSubobject<UProjectUMInventoryComponent>("Inventory");
	LootTable = CreateDefaultSubobject<UProjectUMLootTable>("LootTable");
	PawnSensor = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensor");
	Inventory->Capacity = 20;
	LootTable->OwningNpc = this;

	State = ENpcState::IDLE;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}

	AttackHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("AttackCollider"));
	AttackHitbox->SetupAttachment(RootComponent);
	AttackHitbox->SetCollisionProfileName("NoCollision");
	AttackHitbox->SetNotifyRigidBodyCollision(false);

	if (GetLocalRole() == ROLE_Authority)
	{
		PawnSensor->OnSeePawn.AddDynamic(this, &AProjectUmNpc::OnSeePawn);

		AttackHitbox->OnComponentHit.AddDynamic(this, &AProjectUmNpc::OnAttackHit);
		AttackHitbox->OnComponentEndOverlap.AddDynamic(this, &AProjectUmNpc::OnAttackOverlapEnd);
		AttackHitbox->OnComponentBeginOverlap.AddDynamic(this, &AProjectUmNpc::OnAttackOverlapBegin);
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeAttackMontageObject(TEXT("AnimMontage'/Game/Characters/Mannequins/Animations/Manny/MeleeAttackMontage.MeleeAttackMontage'"));
	if (MeleeAttackMontageObject.Succeeded()) {

		MeleeAttackMontage = MeleeAttackMontageObject.Object;
	}

	AttackRate = 1.0f;
	bIsAttacking = false;
	AttackDamage = 10.0f;
}

// Called when the game starts or when spawned
void AProjectUmNpc::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	AttackHitbox->AttachToComponent(GetMesh(), AttachmentRules, "hand_l");
	AttackHitbox->SetHiddenInGame(false);
}

// Called every frame
void AProjectUmNpc::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() == ROLE_Authority) {

		if (State == ENpcState::DEAD) return;

		if (GetActorLocation().Dist(SpawnLocation, GetActorLocation()) > LeashRange) {
			State = ENpcState::LEASHING;
			Cast<AAIController>(GetController())->MoveToLocation(SpawnLocation);
			FocusPawn = nullptr;
		}

		if (State == ENpcState::AGGRESIVE) {
			Cast<AAIController>(GetController())->MoveToActor(FocusPawn);
			if (GetActorLocation().Dist(FocusPawn->GetActorLocation(), GetActorLocation()) < 100.0f) {
				StartAttack();
			}

		}

		if (State == ENpcState::LEASHING) {
			if (GetActorLocation().Dist(SpawnLocation, GetActorLocation()) < 100.0f) {
				State = ENpcState::IDLE;
			}
			else {
				return;
			}
		}
		
	}

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
			State = ENpcState::DEAD;
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
	if (State == ENpcState::DEAD) {
		return 0.0f;
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		if (EventInstigator->GetPawn()->IsA(AProjectUMCharacter::StaticClass())) {
			FocusPawn = EventInstigator->GetPawn();
			State = ENpcState::AGGRESIVE;
		}
	}
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
	Interactor->BroadcastNpcLoot(Inventory->GetAllInventoryItems());

}

void AProjectUmNpc::OnSeePawn(APawn* OtherPawn)
{
	if (State == ENpcState::DEAD) return;
	if (!FocusPawn && State == ENpcState::IDLE) {
		FocusPawn = OtherPawn;
		State = ENpcState::AGGRESIVE;
	}
}

void AProjectUmNpc::StartAttack()
{
	if (!bIsAttacking)
	{
		bIsAttacking = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(AttackingTimer, this, &AProjectUmNpc::StopAttack, AttackRate, false);
		HandleAttack();
	}
}

void AProjectUmNpc::StopAttack()
{
	bIsAttacking = false;
	AttackHitbox->SetCollisionProfileName("NoCollision");
}

void AProjectUmNpc::HandleAttack()
{
	AttackHitbox->SetCollisionProfileName("Weapon");
	PlayProjectUMCharacterAnimMontage(MeleeAttackMontage);
}

void AProjectUmNpc::PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage) {
	PlayAnimMontage(AnimMontage, 1.0f, FName("start_1"));
}

void AProjectUmNpc::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

void AProjectUmNpc::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetName() != this->GetName() && !AttackedCharactersSet.Contains(OtherActor->GetName()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, OtherActor->GetName());
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetInstigator()->Controller, this, UDamageType::StaticClass());
		AttackedCharactersSet.Add(OtherActor->GetName());
	}
}

void AProjectUmNpc::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AttackHitbox->SetCollisionProfileName("NoCollision");
	AttackedCharactersSet.Remove(OtherActor->GetName());
}