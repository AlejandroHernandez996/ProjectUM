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
#include "ProjectUMInventoryComponent.h"

//////////////////////////////////////////////////////////////////////////
// AProjectUMCharacter

AProjectUMCharacter::AProjectUMCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

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

	EquipRate = .5f;
	bIsEquipping = false;

	UseItemRate = 1.0f;
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
	Inventory->SetIsReplicated(true);

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

	if (GetLocalRole() == ROLE_Authority)
	{
		FistComponent->OnComponentHit.AddDynamic(this, &AProjectUMCharacter::OnAttackHit);
		FistComponent->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapEnd);
		FistComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapBegin);
	}
	
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
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AProjectUMCharacter::CrouchStarted);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AProjectUMCharacter::CrouchStopped);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AProjectUMCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AProjectUMCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AProjectUMCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AProjectUMCharacter::LookUpAtRate);

	// Handle firing projectiles
	PlayerInputComponent->BindAction("Skill 1", IE_Pressed, this, &AProjectUMCharacter::StartFire);

	// Handle attacking projectiles
	PlayerInputComponent->BindAction("Primary Attack", IE_Pressed, this, &AProjectUMCharacter::StartAttack);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AProjectUMCharacter::SpawnItems);


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
	if (AttackType == AttackTypeEnum::FIST) {
		FistComponent->SetCollisionProfileName("NoCollision");
	}
	else {
		EquippedWeapon->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
	}
}

void AProjectUMCharacter::HandleAttack_Implementation()
{
	if (!EquippedWeapon && AttackType == AttackTypeEnum::MELEE_WEAPON) {
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ATTACK"));
	if (AttackType == AttackTypeEnum::FIST) {
		FistComponent->SetCollisionProfileName("Weapon");
	}
	else {
		EquippedWeapon->GetWeaponComponent()->SetCollisionProfileName("Weapon");
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
	if (!EquippedWeapon && AttackType == AttackTypeEnum::MELEE_WEAPON) {
		return;
	}
	if (OtherActor->GetName() != this->GetName())
	{
		if (AttackType == AttackTypeEnum::FIST) {
			UGameplayStatics::ApplyDamage(OtherActor, 5.0f, GetInstigator()->Controller, this, UDamageType::StaticClass());
		}
		else {
			UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetInstigator()->Controller, this, UDamageType::StaticClass());
		}

	}
}

void AProjectUMCharacter::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!EquippedWeapon && AttackType == AttackTypeEnum::MELEE_WEAPON) {
		return;
	}
	if (AttackType == AttackTypeEnum::FIST) {
		FistComponent->SetCollisionProfileName("NoCollision");
	}
	else {
		EquippedWeapon->GetWeaponComponent()->SetCollisionProfileName("NoCollision");
	}
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

	if (EquipSlot == EEquippableSlotsEnum::HAND) {
		HandleEquipWeapon();
	}
	else {
		HandleEquipArmor(EquipSlot);
	}
}

void AProjectUMCharacter::HandleEquipArmor(EEquippableSlotsEnum EquipSlot) {
	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	FString msg = "HANDLING EQUIP ARMOR";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	if (EquipSlot == EEquippableSlotsEnum::HEAD) {
		// attach collision components to sockets based on transformation definitions

		EquippedHead = GetWorld()->SpawnActor<AProjectUMEquipment>(HeadClass);

		if (EquippedHead != nullptr) {
			EquippedHead->GetCapsuleComponent()->SetupAttachment(RootComponent);
			EquippedHead->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
			EquippedHead->GetCapsuleComponent()->AttachToComponent(GetMesh(), AttachmentRules, "head");
		}
	}
	else if (EquipSlot == EEquippableSlotsEnum::CHEST) {
		EquippedChest = GetWorld()->SpawnActor<AProjectUMEquipment>(ChestClass);

		if (EquippedChest != nullptr) {
			EquippedHead->GetCapsuleComponent()->SetupAttachment(RootComponent);
			EquippedChest->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
			EquippedChest->GetCapsuleComponent()->AttachToComponent(GetMesh(), AttachmentRules, "spine_05");
		}
	}
	else if (EquipSlot == EEquippableSlotsEnum::LEGS) {
		EquippedLegs = GetWorld()->SpawnActor<AProjectUMEquipment>(LegsClass);

		if (EquippedLegs != nullptr) {
			EquippedLegs->GetCapsuleComponent()->SetupAttachment(RootComponent);
			EquippedLegs->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
			EquippedLegs->GetCapsuleComponent()->AttachToComponent(GetMesh(), AttachmentRules, "pelvis");
		}
	}
	else {
		return;
	}
	
}


void AProjectUMCharacter::HandleEquipWeapon() {
	
		// attach collision components to sockets based on transformation definitions
		const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

		EquippedWeapon = GetWorld()->SpawnActor<AProjectUMWeapon>(WeaponClass);

		if (EquippedWeapon != nullptr) {
			EquippedWeapon->GetCapsuleComponent()->SetupAttachment(RootComponent);
			EquippedWeapon->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
			EquippedWeapon->GetWeaponComponent()->SetCollisionProfileName("NoCollision");
			EquippedWeapon->GetWeaponComponent()->SetNotifyRigidBodyCollision(false);

			EquippedWeapon->GetCapsuleComponent()->AttachToComponent(GetMesh(), AttachmentRules, "hand_l");
			EquippedWeapon->GetWeaponComponent()->SetHiddenInGame(false);
			AttackType = AttackTypeEnum::MELEE_WEAPON;
		}

		if (GetLocalRole() == ROLE_Authority) {
			EquippedWeapon->GetWeaponComponent()->OnComponentHit.AddDynamic(this, &AProjectUMCharacter::OnAttackHit);
			EquippedWeapon->GetWeaponComponent()->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapEnd);
			EquippedWeapon->GetWeaponComponent()->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapBegin);
		}
}

void AProjectUMCharacter::StartUsingItem(UProjectUMItem* Item)
{
	if (Item == nullptr) {
		return;
	}
	if (!bIsUsingItem)
	{
		bIsUsingItem = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(UseItemTimer, this, &AProjectUMCharacter::StopUsingItem, UseItemRate, false);
		HandleUseItemServer(Item);
	}
}

void AProjectUMCharacter::StopUsingItem()
{
		bIsUsingItem = false;
}

void AProjectUMCharacter::HandleUseItemServer_Implementation(UProjectUMItem* Item)
{
		Item->Use(this);
		HandleUseItemClient(Item);
}

void AProjectUMCharacter::HandleUseItemClient_Implementation(UProjectUMItem* Item)
{
	if (GetLocalRole() != ROLE_Authority) {
		Item->Use(this);
	}
}

void AProjectUMCharacter::SpawnItems_Implementation() {
	FString msg = "?";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	for (auto& Item : Inventory->DefaultItems) {
		FString healthMessage = "YO";
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
		Inventory->AddItem(Item);
	}
}

void AProjectUMCharacter::AttachWeapon(TSubclassOf<AProjectUMWeapon> Weapon) {

	WeaponClass = Weapon;
	StartEquipping(EEquippableSlotsEnum::HAND);
}

void AProjectUMCharacter::DeAttachWeapon() {
	if (EquippedWeapon) {
		FString msg = "DESTRUCITON";
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
		EquippedWeapon->Destroy();
		AttackType = AttackTypeEnum::FIST;

	}
}

void AProjectUMCharacter::AttachArmor(TSubclassOf<AProjectUMEquipment> Armor, EEquippableSlotsEnum EquipSlot) {
	FString msg = "ATTACHING ARMOR";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	if (EquipSlot == EEquippableSlotsEnum::HEAD) {
		HeadClass = Armor;
		if (GetLocalRole() == ROLE_Authority && Inventory->HeadSlot)
		{
			SetCurrentMaxHealth(MaxHealth + Inventory->HeadSlot->HealthAmount);
		}
	}
	else if (EquipSlot == EEquippableSlotsEnum::CHEST) {
		ChestClass = Armor;
		if (GetLocalRole() == ROLE_Authority && Inventory->ChestSlot)
		{
			SetCurrentMaxHealth(MaxHealth + Inventory->ChestSlot->HealthAmount);
		}
	}
	else if (EquipSlot == EEquippableSlotsEnum::LEGS) {
		LegsClass = Armor;
		if (GetLocalRole() == ROLE_Authority && Inventory->LegsSlot)
		{
			SetCurrentMaxHealth(MaxHealth + Inventory->LegsSlot->HealthAmount);
		}
	}
	else {
		return;
	}
	StartEquipping(EquipSlot);
}

void AProjectUMCharacter::DeAttachArmor(EEquippableSlotsEnum EquipSlot) {
	FString msg = "DETTACHING ARMOR";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	if (EquipSlot == EEquippableSlotsEnum::HEAD && EquippedHead) {
		if (GetLocalRole() == ROLE_Authority && Inventory->HeadSlot)
		{
			SetCurrentMaxHealth(MaxHealth - Inventory->HeadSlot->HealthAmount);
		}
		EquippedHead->Destroy();
	}
	else if (EquipSlot == EEquippableSlotsEnum::CHEST && EquippedChest) {
		if (GetLocalRole() == ROLE_Authority && Inventory->ChestSlot)
		{
			SetCurrentMaxHealth(MaxHealth - Inventory->ChestSlot->HealthAmount);
		}
		EquippedChest->Destroy();
	}
	else if (EquipSlot == EEquippableSlotsEnum::LEGS && EquippedLegs) {
		if (GetLocalRole() == ROLE_Authority && Inventory->LegsSlot)
		{
			SetCurrentMaxHealth(MaxHealth - Inventory->LegsSlot->HealthAmount);
		}
		EquippedLegs->Destroy();
	}
	else {
		return;
	}
}