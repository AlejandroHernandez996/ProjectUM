// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectUMCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "ProjectUMProjectile.h"
#include "ProjectUMGameMode.h"
#include "Components/SphereComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectUMWeapon.h"
#include "ProjectUMEquipment.h"
#include "MyProjectUMFoodItem.h"
#include "ProjectUMItem.h"
#include "ProjectUMPickableItem.h"
#include "ProjectUmNpc.h"
#include "ProjectUMInventoryComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProjectUMCharacter

AProjectUMCharacter::AProjectUMCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	PrimaryActorTick.bCanEverTick = true;

	//Initialize the player's Health
	BaseMaxHealth = 100.0f;
	MaxHealth = BaseMaxHealth;
	CurrentHealth = MaxHealth;

	//Initialize projectile class
	ProjectileClass = AProjectUmProjectile::StaticClass();
	//Initialize fire rate
	FireRate = 0.25f;
	bIsFiringWeapon = false;

	//Initialize attack rate
	AttackRate = 1.0f;
	bIsAttacking = false;

	EquipRate = .25f;
	bIsEquipping = false;

	UseItemRate = .5f;
	bIsUsingItem = false;

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create an inventory
	Inventory = CreateDefaultSubobject<UProjectUMInventoryComponent>("Inventory");
	Inventory->Capacity = 20;
	Inventory->OwningCharacter = this;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeleeAttackMontageObject(TEXT("AnimMontage'/Game/Characters/Mannequins/Animations/Manny/MeleeAttackMontage.MeleeAttackMontage'"));
	if (MeleeAttackMontageObject.Succeeded()) {

		MeleeAttackMontage = MeleeAttackMontageObject.Object;
	}

	FistComponent = CreateDefaultSubobject<USphereComponent>(TEXT("HandCollider"));
	FistComponent->SetupAttachment(RootComponent);
	FistComponent->SetCollisionProfileName("NoCollision");
	FistComponent->SetNotifyRigidBodyCollision(false);

	InteractComponent = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollider"));
	InteractComponent->SetupAttachment(RootComponent);
	InteractComponent->SetCollisionProfileName("Interaction");
	InteractComponent->SetNotifyRigidBodyCollision(false);

	if (GetLocalRole() == ROLE_Authority)
	{
		FistComponent->OnComponentHit.AddDynamic(this, &AProjectUMCharacter::OnAttackHit);
		FistComponent->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapEnd);
		FistComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapBegin);

		InteractComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnInteractOverlapBegin);
		InteractComponent->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnInteractOverlapEnd);
	}
	
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::HAND, "hand_l");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::HEAD, "head");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::CHEST, "spine_05");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::LEGS, "pelvis");

}

// Called every frame
void AProjectUMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// Called after constructor
void AProjectUMCharacter::BeginPlay() {
	Super::BeginPlay();

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	FistComponent->AttachToComponent(GetMesh(), AttachmentRules, "hand_l");
	FistComponent->SetHiddenInGame(false);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectUMCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AProjectUMCharacter::CrouchStarted);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AProjectUMCharacter::CrouchStopped);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AProjectUMCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AProjectUMCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AProjectUMCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AProjectUMCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Skill 1", IE_Pressed, this, &AProjectUMCharacter::StartFire);
	PlayerInputComponent->BindAction("Primary Attack", IE_Pressed, this, &AProjectUMCharacter::StartAttack);
	PlayerInputComponent->BindAction("Spawn Items", IE_Pressed, this, &AProjectUMCharacter::SpawnItems);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AProjectUMCharacter::StartInteracting);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AProjectUMCharacter::OpenInventory);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AProjectUMCharacter::BroadcastInventory);

}

void AProjectUMCharacter::CrouchStarted()
{
	if (CanJump()) {
		Crouch();
	}
}

void AProjectUMCharacter::CrouchStopped()
{
	if (!CanJump()) {
		UnCrouch();
	}

}


void AProjectUMCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AProjectUMCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AProjectUMCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProjectUMCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Replicated Properties

void AProjectUMCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AProjectUMCharacter, CurrentHealth);
	DOREPLIFETIME(AProjectUMCharacter, MaxHealth);
}

void AProjectUMCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AProjectUMCharacter::OnRep_MaxHealth()
{
	
}

void AProjectUMCharacter::OnHealthUpdate()
{
	
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
		if (CurrentHealth == 0) {
			Destroy();
		}
		
}

void AProjectUMCharacter::SetCurrentMaxHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		MaxHealth = FMath::Clamp(healthValue, 0.f, 1000.f);
	}
	FString healthMessage = FString::Printf(TEXT("%s now has %f max health remaining."), *GetFName().ToString(), MaxHealth);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, healthMessage);
	if (CurrentHealth > MaxHealth) {
		FString fd = FString::Printf(TEXT("%s now has %f health remaining due to being higher than max health."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, fd);
		SetCurrentHealth(MaxHealth);
	}
}

void AProjectUMCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AProjectUMCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

void AProjectUMCharacter::StartFire()
{
	UE_LOG(LogTemp, Warning, TEXT("SHOOT"));
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AProjectUMCharacter::StopFire, FireRate, false);
		HandleFire();
	}
}

void AProjectUMCharacter::StopFire()
{
	bIsFiringWeapon = false;
}

void AProjectUMCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetActorRotation().Vector() * 100.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetActorRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	AProjectUmProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectUmProjectile>(spawnLocation, spawnRotation, spawnParameters);
}

void AProjectUMCharacter::StartAttack()
{
	if (!bIsAttacking)
	{
		bIsAttacking = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(AttackingTimer, this, &AProjectUMCharacter::StopAttack, AttackRate, false);
		HandleAttack();
	}
}

void AProjectUMCharacter::StopAttack()
{
	bIsAttacking = false;
	if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)) {
		FistComponent->SetCollisionProfileName("NoCollision");
	}
	else {
		EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)->GetHitboxComponent()->SetCollisionProfileName("NoCollision");
	}
}

void AProjectUMCharacter::HandleAttack_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ATTACK"));
	if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)){
		FistComponent->SetCollisionProfileName("Weapon");
	}
	else {
		EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)->GetHitboxComponent()->SetCollisionProfileName("Weapon");
	}
	PlayProjectUMCharacterAnimMontage(MeleeAttackMontage);
}

void AProjectUMCharacter::PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage) {
	PlayAnimMontage(AnimMontage, 1.0f, FName("start_1"));
}

void AProjectUMCharacter::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
}

void AProjectUMCharacter::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetName() != this->GetName() && !AttackedCharactersSet.Contains(OtherActor->GetName()))
	{
		if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)) {
			FString msg = "HAND ATTACK";
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
			UGameplayStatics::ApplyDamage(OtherActor, 5.0f, GetInstigator()->Controller, this, UDamageType::StaticClass());
		}
		else {
			FString msg = "SWORD ATTACK";
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
			UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetInstigator()->Controller, this, UDamageType::StaticClass());
		}
		AttackedCharactersSet.Add(OtherActor->GetName());
	}
}

void AProjectUMCharacter::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)) {
		FistComponent->SetCollisionProfileName("NoCollision");
	}
	else {
		EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)->GetHitboxComponent()->SetCollisionProfileName("NoCollision");
	}
	AttackedCharactersSet.Remove(OtherActor->GetName());
}

void AProjectUMCharacter::StartEquipping(EEquippableSlotsEnum EquipSlot)
{
	if (!bIsEquipping)
	{
		bIsEquipping = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(EquippingTimer, this, &AProjectUMCharacter::StopEquipping, EquipRate, false);
		HandleEquip(EquipSlot);
	}
}

void AProjectUMCharacter::StopEquipping()
{
	bIsEquipping = false;
}

void AProjectUMCharacter::HandleEquip(EEquippableSlotsEnum EquipSlot) {
		FString msg = "HANDLING EQUIP ARMOR";
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);

		const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
		EquippedItemMap.Add(EquipSlot, GetWorld()->SpawnActor<AProjectUMEquipment>(EquipmentClassMap.FindRef(EquipSlot)));
		EquippedItemMap.FindRef(EquipSlot)->GetRootComponent()->SetupAttachment(RootComponent);
		EquippedItemMap.FindRef(EquipSlot)->GetRootComponent()->AttachToComponent(GetMesh(), AttachmentRules, EquipSlotSkeletonMapping.FindRef(EquipSlot));
		if (EquipSlot == EEquippableSlotsEnum::HAND) {
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->SetCollisionProfileName("NoCollision");
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->SetNotifyRigidBodyCollision(false);
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->SetHiddenInGame(false);
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->OnComponentHit.AddDynamic(this, &AProjectUMCharacter::OnAttackHit);
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapEnd);
			EquippedItemMap.FindRef(EquipSlot)->GetHitboxComponent()->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapBegin);
		}
}


void AProjectUMCharacter::StartUsingItem(int32 ItemId)
{
	
	if (!bIsUsingItem)
	{
		bIsUsingItem = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(UseItemTimer, this, &AProjectUMCharacter::StopUsingItem, UseItemRate, false);
		HandleUseItemServer(ItemId);
	}
}

void AProjectUMCharacter::StopUsingItem()
{
		bIsUsingItem = false;
}

void AProjectUMCharacter::HandleUseItemServer_Implementation(int32 ItemId)
{
	UProjectUMItem* Item = Inventory->GetItemById(ItemId);
	if (!Item) {
		return;
	}
	FString msg = "USING ITEM";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, msg);
	
	Item->Use(this);
	BroadcastInventory();
}

void AProjectUMCharacter::StartDroppingItem(int32 ItemId)
{
	if (!bIsUsingItem)
	{
		bIsUsingItem = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(UseItemTimer, this, &AProjectUMCharacter::StopUsingItem, UseItemRate, false);
		HandleDropItemServer(ItemId);
	}
}

void AProjectUMCharacter::StopDroppingItem()
{
	bIsUsingItem = false;
}

void AProjectUMCharacter::HandleDropItemServer_Implementation(int32 ItemId)
{
	UProjectUMItem* Item = Inventory->GetItemById(ItemId);
	if (!Item) {
		return;
	}
	FString msg = "DROPPING ITEM";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, msg);
	AProjectUMPickableItem* PickableItem = (AProjectUMPickableItem*)GetWorld()->SpawnActor(AProjectUMPickableItem::StaticClass());
	PickableItem->SetActorLocation(GetActorLocation());
	PickableItem->SetStaticMesh(Item->PickupMesh);
	PickableItem->SetItem(Item);
	PickableItem->StaticMesh->SetCollisionProfileName("NoCollision");
	Inventory->RemoveItem(Item);
	BroadcastInventory();
}

void AProjectUMCharacter::SpawnItems_Implementation() {
	for (auto& Item : Inventory->DefaultItems) {
		Inventory->AddItem(Item);
	}
	BroadcastInventory();
}

void AProjectUMCharacter::AttachEquipment(TSubclassOf<AProjectUMEquipment> Equipment, EEquippableSlotsEnum EquipSlot) {
	if (GetLocalRole() == ROLE_Authority) {
		EquipmentClassMap.Add(EquipSlot, Equipment);
		SetCurrentMaxHealth(MaxHealth + Inventory->EquipmentMap.FindRef(EquipSlot)->HealthAmount);
		StartEquipping(EquipSlot);
	}
}

void AProjectUMCharacter::DeAttachEquipment(EEquippableSlotsEnum EquipSlot) {
	if (GetLocalRole() == ROLE_Authority) {
		EquipmentClassMap.FindRef(EquipSlot) = nullptr;
		EquippedItemMap.FindRef(EquipSlot)->Destroy();
		SetCurrentMaxHealth(MaxHealth - Inventory->EquipmentMap.FindRef(EquipSlot)->HealthAmount);
	}
}

void AProjectUMCharacter::StartInteracting() {
	if (bIsInteracting) {
		return;
	}
	bIsInteracting = true;
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(InteractTimer, this, &AProjectUMCharacter::StopInteracting, InteractRate, false);
	HandleInteraction();
}

void AProjectUMCharacter::StopInteracting() {
	bIsInteracting = false;

}

void AProjectUMCharacter::HandleInteraction_Implementation() {
	if (!InteractableObjects.IsEmpty()) {
		for (auto& Element : InteractableObjects) {
			FString msg2 = "HANDLE INTERACTING ON " + Element->_getUObject()->GetName();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, msg2);
			 IInteractableObjectInterface::Execute_Interact(Element->_getUObject(), this);
			return;
		}
	}
}

void AProjectUMCharacter::Interact_Implementation(AProjectUMCharacter* Interactor)
{
	FString msg = "Character  " + this->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	Inventory->LootingCharacters.Add(Interactor);
	Interactor->SetLootingInventory(Inventory);
	Interactor->OpenLoot();
	Interactor->BroadcastNpcLoot(Inventory->GetAllInventoryItemIds());
}

void AProjectUMCharacter::OnInteractOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractableObjectInterface* InteractableObject = Cast<IInteractableObjectInterface>(OtherActor);
	if (OtherActor != this && InteractableObject) {
		FString msg = "DETECTED OBJECT " + OtherActor->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
		InteractableObjects.Add(InteractableObject);
	}
}

void AProjectUMCharacter::OnInteractOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IInteractableObjectInterface* InteractableObject = Cast<IInteractableObjectInterface>(OtherActor);
	if (InteractableObjects.Contains(InteractableObject)) {
		FString msg = "OBJECT OUT OF RANGE " + OtherActor->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
		InteractableObjects.Remove(InteractableObject);
	}
}

void AProjectUMCharacter::AddItemToInventory_Implementation(UProjectUMItem* Item) {
	Inventory->AddItem(Item);
	BroadcastInventory();
}

void AProjectUMCharacter::BroadcastNpcLoot_Implementation(const TArray<int32>& ItemIds) {
	FString msg = "BROADCASTING NPC LOOT :)";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);

	OnNpcCorpseInteracted.Broadcast(ItemIds);
}

void AProjectUMCharacter::OpenLoot_Implementation() {
	OnNpcCorpseInteractedOpenLootWidget.Broadcast();
}

void AProjectUMCharacter::BroadcastInventory_Implementation() {

	TArray<UProjectUMEquippableItem*> EquippedItems;
	Inventory->EquipmentMap.GenerateValueArray(EquippedItems);
	TArray<int32> EquippedItemIds = TArray<int32>();
	for (auto& Item : EquippedItems){
		if (Item) {
			EquippedItemIds.Add(Item->ItemId);
		}
	}
	BroadcastInventoryToClient(Inventory->GetAllInventoryItemIds(), EquippedItemIds);
}

void AProjectUMCharacter::OpenInventory_Implementation() {
	OnInventoryOpen.Broadcast();
}

void AProjectUMCharacter::BroadcastInventoryToClient_Implementation(const TArray<int32>& InventoryItemIds, const TArray<int32>& EquippedItemIds) {
	OnInventoryOpenDisplayItems.Broadcast(InventoryItemIds, EquippedItemIds);
}

void AProjectUMCharacter::StartLootingItem(int32 ItemId)
{

	if (!bIsUsingItem)
	{
		bIsUsingItem = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(UseItemTimer, this, &AProjectUMCharacter::StopUsingItem, UseItemRate, false);
		HandleLootingItem(ItemId);
	}
}

void AProjectUMCharacter::StopLootingItem()
{
	bIsUsingItem = false;
}

void AProjectUMCharacter::HandleLootingItem_Implementation(int32 ItemId)
{
	if (!LootingInventory) return;

	UProjectUMItem* LootedItem = LootingInventory->GetItemById(ItemId);
	if (!LootedItem) {
		return;
	}
	FString msg = "Looting ITEM";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, msg);
	Inventory->AddItem(LootedItem);
	LootingInventory->RemoveItem(LootedItem);

	BroadcastInventory();
	BroadcastNpcLoot(LootingInventory->GetAllInventoryItemIds());
}