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
#include "ProjectUMCharacterStatsStruct.h"
#include "ProjectUMLootableProp.h"
#include "ProjectUMItemGenerator.h"
#include "ChannableInterface.h"
#include "ProjectUMResource.h"
#include "Engine/GameInstance.h"
#include "ProjectUMInventoryComponent.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "ProjectUMAssetCache.h"
#include "ProjectUMGameState.h"
#include "ProjectUMBow.h"
#include "Components/StaticMeshComponent.h"


//////////////////////////////////////////////////////////////////////////
// AProjectUMCharacter

AProjectUMCharacter::AProjectUMCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	PrimaryActorTick.bCanEverTick = true;

	InitStats();

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
	GetCharacterMovement()->MaxWalkSpeed = 750.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f; // The camera follows at this distance behind the character	
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
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::SHIELD, "hand_r");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::HEAD, "head");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::CHEST, "spine_05");
	EquipSlotSkeletonMapping.Add(EEquippableSlotsEnum::LEGS, "pelvis");

}

void AProjectUMCharacter::InitStats() {

	//Initialize the player's Health
	BaseHealth = 100.0f;
	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::HEALTH, BaseHealth);
	MaxStatsMap.Add(ECharacterStatEnum::HEALTH, MaxHealth);
	CurrentStatsMap.Add(ECharacterStatEnum::HEALTH, CurrentHealth);


	BaseMana = 100.0f;
	MaxMana = 100.0f;
	CurrentMana = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::MANA, BaseMana);
	MaxStatsMap.Add(ECharacterStatEnum::MANA, MaxMana);
	CurrentStatsMap.Add(ECharacterStatEnum::MANA, CurrentMana);

	BaseAgility = 100.0f;
	MaxAgility = 100.0f;
	CurrentAgility = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::AGILITY, BaseAgility);
	MaxStatsMap.Add(ECharacterStatEnum::AGILITY, MaxAgility);
	CurrentStatsMap.Add(ECharacterStatEnum::AGILITY, CurrentAgility);

	BaseWisdom = 100.0f;
	MaxWisdom = 100.0f;
	CurrentWisdom = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::WISDOM, BaseWisdom);
	MaxStatsMap.Add(ECharacterStatEnum::WISDOM, MaxWisdom);
	CurrentStatsMap.Add(ECharacterStatEnum::WISDOM, CurrentWisdom);

	BaseIntellect = 100.0f;
	CurrentIntellect = 100.0f;
	MaxIntellect = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::INTELLECT, BaseIntellect);
	MaxStatsMap.Add(ECharacterStatEnum::INTELLECT, CurrentIntellect);
	CurrentStatsMap.Add(ECharacterStatEnum::INTELLECT, MaxIntellect);

	BaseStrength = 100.0f;
	CurrentStrength = 100.0f;
	MaxStrength = 100.0f;
	BaseStatsMap.Add(ECharacterStatEnum::STRENGTH, BaseStrength);
	MaxStatsMap.Add(ECharacterStatEnum::STRENGTH, CurrentStrength);
	CurrentStatsMap.Add(ECharacterStatEnum::STRENGTH, MaxStrength);
}

// Called every frame
void AProjectUMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown(FKey("LeftMouseButton")))
	{
		HoldPrimaryInput();
	}

	if (GetWorld()->GetFirstPlayerController()->WasInputKeyJustReleased(FKey("LeftMouseButton")))
	{
		ReleasePrimaryInput();
	}

}

void AProjectUMCharacter::Dance_Implementation() {
	PlayProjectUMCharacterAnimMontage(DanceMontage);
}

void AProjectUMCharacter::HoldPrimaryInput_Implementation() {
	if (Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)
		&& Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)->WeaponType == EProjectUMWeaponType::BOW)
	{
		IChannableInterface* ChannableObject = Cast<IChannableInterface>(EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND));
		ChannableObject->Execute_Channel(ChannableObject->_getUObject());
	}
}

void AProjectUMCharacter::ReleasePrimaryInput_Implementation() {
	if (Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)
		&& Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)->WeaponType == EProjectUMWeaponType::BOW)
	{
		IChannableInterface* ChannableObject = Cast<IChannableInterface>(EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND));
		ChannableObject->Execute_Release(ChannableObject->_getUObject());
	}
}

// Called after constructor
void AProjectUMCharacter::BeginPlay() {
	Super::BeginPlay();

	const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	FistComponent->AttachToComponent(GetMesh(), AttachmentRules, "hand_l");
	FistComponent->SetHiddenInGame(false);

	if (GetLocalRole() == ROLE_Authority) {
		FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &AProjectUMCharacter::OnResponseReceived);
		Request->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/inventory/JANDRO");
		Request->SetVerb("GET");
		Request->ProcessRequest();
	}
}

void AProjectUMCharacter::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	FString ITEM = FString(TEXT("Item"));

	FInventoryJsonStruct InventoryStruct;
	FJsonObjectConverter::JsonObjectToUStruct(ResponseObj.Get()->GetObjectField(ITEM).ToSharedRef(), &InventoryStruct);
	
	UProjectUMAssetCache* AssetCache = GetWorld()->GetGameState<AProjectUMGameState>()->AssetCache;
	for (auto& ParsedJsonItem : InventoryStruct.items) {
		for (auto& Item : AssetCache->ItemCache) {
			if (Item && Item->ItemId == ParsedJsonItem.item_id) {
				UProjectUMItem* NewItem = DuplicateObject(Item, nullptr);
				NewItem->ItemRarity = UProjectUMItem::RarityStringToEnum(ParsedJsonItem.rarity);
				for (auto& kvp : ParsedJsonItem.stats) {
					NewItem->Stats.Add(UProjectUMItem::CharacterStatStringToEnum(kvp.Key), kvp.Value);
				}
				NewItem->StackSize = ParsedJsonItem.quantity;
				Inventory->AddItem(NewItem);
				break;
			}
		}
	}
	
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

	PlayerInputComponent->BindAction("Dance", IE_Pressed, this, &AProjectUMCharacter::Dance);


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
	DOREPLIFETIME(AProjectUMCharacter, BaseHealth);

	DOREPLIFETIME(AProjectUMCharacter, CurrentAgility);
	DOREPLIFETIME(AProjectUMCharacter, MaxAgility);
	DOREPLIFETIME(AProjectUMCharacter, BaseAgility);

	DOREPLIFETIME(AProjectUMCharacter, CurrentStrength);
	DOREPLIFETIME(AProjectUMCharacter, MaxStrength);
	DOREPLIFETIME(AProjectUMCharacter, BaseStrength);

	DOREPLIFETIME(AProjectUMCharacter, CurrentMana);
	DOREPLIFETIME(AProjectUMCharacter, MaxMana);
	DOREPLIFETIME(AProjectUMCharacter, BaseMana);

	DOREPLIFETIME(AProjectUMCharacter, CurrentIntellect);
	DOREPLIFETIME(AProjectUMCharacter, MaxIntellect);
	DOREPLIFETIME(AProjectUMCharacter, BaseIntellect);

	DOREPLIFETIME(AProjectUMCharacter, CurrentWisdom);
	DOREPLIFETIME(AProjectUMCharacter, MaxWisdom);
	DOREPLIFETIME(AProjectUMCharacter, BaseWisdom);

}

void AProjectUMCharacter::OnRep_CurrentHealth()
{
	OnCurrentStatUpdate(ECharacterStatEnum::HEALTH, CurrentHealth);
	OnHealthUpdate();
}

void AProjectUMCharacter::OnRep_MaxHealth()
{
	OnMaxStatUpdate(ECharacterStatEnum::HEALTH, MaxHealth);
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

void AProjectUMCharacter::OnCurrentStatUpdate(ECharacterStatEnum Stat, float Value)
{
	CurrentStatsMap.Add(Stat, Value);
	OnCurrentStatUpdateClient(Stat, Value);
}

void AProjectUMCharacter::OnMaxStatUpdate(ECharacterStatEnum Stat, float Value)
{
	MaxStatsMap.Add(Stat, Value);
	OnMaxStatUpdateClient(Stat, Value);
}

void AProjectUMCharacter::OnCurrentStatUpdateClient_Implementation(ECharacterStatEnum Stat, float Value) {
	CurrentStatsMap.Add(Stat, Value);
}

void AProjectUMCharacter::OnMaxStatUpdateClient_Implementation(ECharacterStatEnum Stat, float Value) {
	MaxStatsMap.Add(Stat, Value);
}

void AProjectUMCharacter::OnRep_MaxMana()
{
	OnMaxStatUpdate(ECharacterStatEnum::MANA, MaxMana);
}

void AProjectUMCharacter::OnRep_CurrentMana()
{
	OnCurrentStatUpdate(ECharacterStatEnum::MANA, CurrentMana);
}

void AProjectUMCharacter::OnRep_CurrentAgility()
{
	OnCurrentStatUpdate(ECharacterStatEnum::AGILITY, CurrentAgility);
}
void AProjectUMCharacter::OnRep_CurrentStrength()
{
	OnCurrentStatUpdate(ECharacterStatEnum::STRENGTH, CurrentStrength);
}
void AProjectUMCharacter::OnRep_CurrentWisdom()
{
	OnCurrentStatUpdate(ECharacterStatEnum::WISDOM, CurrentWisdom);
}
void AProjectUMCharacter::OnRep_CurrentIntellect()
{
	OnCurrentStatUpdate(ECharacterStatEnum::INTELLECT, CurrentIntellect);
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

void AProjectUMCharacter::SetCurrentStat(ECharacterStatEnum Stat, float Value)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		switch (Stat) {
		case ECharacterStatEnum::HEALTH:
			CurrentHealth = Value;
			OnCurrentStatUpdate(ECharacterStatEnum::HEALTH, Value);
		case ECharacterStatEnum::MANA:
			 CurrentMana = Value;
			 OnCurrentStatUpdate(ECharacterStatEnum::MANA, Value);
		case ECharacterStatEnum::AGILITY:
			 CurrentAgility = Value;
			 OnCurrentStatUpdate(ECharacterStatEnum::AGILITY, Value);
		case ECharacterStatEnum::STRENGTH:
			 CurrentStrength = Value;
			 OnCurrentStatUpdate(ECharacterStatEnum::STRENGTH, Value);
		case ECharacterStatEnum::WISDOM:
			 CurrentWisdom = Value;
			 OnCurrentStatUpdate(ECharacterStatEnum::WISDOM, Value);
		case ECharacterStatEnum::INTELLECT:
			 CurrentIntellect = Value;
			 OnCurrentStatUpdate(ECharacterStatEnum::INTELLECT, Value);
		}
	}
}

void AProjectUMCharacter::SetMaxStat(ECharacterStatEnum Stat, float Value)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		switch (Stat) {
		case ECharacterStatEnum::HEALTH:
			MaxHealth = Value;
			OnMaxStatUpdate(ECharacterStatEnum::HEALTH, Value);
			OnHealthUpdate();
		case ECharacterStatEnum::MANA:
			 MaxMana = Value;
			 OnMaxStatUpdate(ECharacterStatEnum::MANA, Value);
		case ECharacterStatEnum::AGILITY:
			 MaxAgility = Value;
			 OnMaxStatUpdate(ECharacterStatEnum::AGILITY, Value);
		case ECharacterStatEnum::STRENGTH:
			 MaxStrength = Value;
			 OnMaxStatUpdate(ECharacterStatEnum::STRENGTH, Value);
		case ECharacterStatEnum::WISDOM:
			 MaxWisdom = Value;
			 OnMaxStatUpdate(ECharacterStatEnum::WISDOM, Value);
		case ECharacterStatEnum::INTELLECT:
			 MaxIntellect = Value;
			 OnMaxStatUpdate(ECharacterStatEnum::INTELLECT, Value);
		}
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
	AttackedCharactersSet.Reset();
}

void AProjectUMCharacter::HandleAttack_Implementation()
{
	if (Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)
		&& Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND)->WeaponType == EProjectUMWeaponType::BOW)
	{
		return;
	}

	UAnimMontage* AttackMontage = MeleeAttackMontage;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ATTACK"));
	if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)){
		FistComponent->SetCollisionProfileName("Weapon");
	}
	else {
		AProjectUMEquipment* Weapon = EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND);
		Weapon->GetHitboxComponent()->SetCollisionProfileName("Weapon");
		UAnimMontage* WeaponAttackMontage = Inventory->GetEquipmentItem(EEquippableSlotsEnum::HAND)->AttackMontage;
		if (WeaponAttackMontage) {
			AttackMontage = WeaponAttackMontage;
		}
	}
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(AttackingTimer, this, &AProjectUMCharacter::StopAttack, AttackRate, false);
	PlayProjectUMCharacterAnimMontage(AttackMontage);
}

void AProjectUMCharacter::PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage) {
	PlayAnimMontage(AnimMontage, 1.0f, FName("start_1"));
}

void AProjectUMCharacter::OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
}

void AProjectUMCharacter::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(AttackedCharactersSet.Contains(OtherActor->GetName())) return;

	UProjectUMEquippableItem* HandItem = Inventory->EquipmentMap.FindRef(EEquippableSlotsEnum::HAND);
	AProjectUMResource* ResourceObject = Cast<AProjectUMResource>(OtherActor);
	IProjectUMItemGenerator* ItemGeneratorObject = Cast<IProjectUMItemGenerator>(OtherActor);
	
	if (HandItem && ResourceObject && HandItem->ToolType == ResourceObject->GetToolType() && ItemGeneratorObject) {
		ItemGeneratorObject->Execute_Generate(ResourceObject->_getUObject(), this);
		AttackedCharactersSet.Add(OtherActor->GetName());
		return;
	}

	if (OtherActor->GetName() != this->GetName())
	{
		if (!EquippedItemMap.FindRef(EEquippableSlotsEnum::HAND)) {
			FString msg = "HAND ATTACK";
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
			UGameplayStatics::ApplyDamage(OtherActor, 5.0f, GetInstigator()->Controller, this, UDamageType::StaticClass());
		}
		else {
			FString msg = "SWORD ATTACK";
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
			UGameplayStatics::ApplyDamage(OtherActor, 10.0f + CurrentStrength, GetInstigator()->Controller, this, UDamageType::StaticClass());
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

		bIsEquipping = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(EquippingTimer, this, &AProjectUMCharacter::StopEquipping, EquipRate, false);
		HandleEquip(EquipSlot);
}

void AProjectUMCharacter::StopEquipping()
{
	bIsEquipping = false;
}

void AProjectUMCharacter::HandleEquip(EEquippableSlotsEnum EquipSlot) {
		FString msg = "HANDLING EQUIP ARMOR";
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);

		const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
		UProjectUMEquippableItem* Item = Inventory->GetEquipmentItem(EquipSlot);
		AProjectUMEquipment* EquipmentActor = GetWorld()->SpawnActor<AProjectUMEquipment>(EquipmentClassMap.FindRef(EquipSlot));
		EquipmentActor->SetMesh(Item->PickupMesh, Item->BaseRotation, Item->BaseLocation, Item->BaseScale);
		EquippedItemMap.Add(EquipSlot, EquipmentActor);
		EquipmentActor->GetRootComponent()->AttachToComponent(GetMesh(), AttachmentRules, EquipSlotSkeletonMapping.FindRef(EquipSlot));
		EquipmentActor->OwningActor = this;
		if (EquipSlot == EEquippableSlotsEnum::HAND) {
			EquipmentActor->GetHitboxComponent()->SetCollisionProfileName("NoCollision");
			EquipmentActor->GetHitboxComponent()->SetNotifyRigidBodyCollision(false);
			EquipmentActor->GetHitboxComponent()->SetHiddenInGame(false);
			EquipmentActor->GetHitboxComponent()->OnComponentHit.AddDynamic(this, &AProjectUMCharacter::OnAttackHit);
			EquipmentActor->GetHitboxComponent()->OnComponentEndOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapEnd);
			EquipmentActor->GetHitboxComponent()->OnComponentBeginOverlap.AddDynamic(this, &AProjectUMCharacter::OnAttackOverlapBegin);
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
	Inventory->DropItem(Item);
	BroadcastInventory();
}

void AProjectUMCharacter::SpawnItems_Implementation() {
	for (auto& Item : Inventory->DefaultItems) {
		int rand = FMath::RandRange(0, 4);
		if (rand == 0) Item->ItemRarity = EProjectUMItemRarityEnum::COMMON;
		if (rand == 1) Item->ItemRarity = EProjectUMItemRarityEnum::UNCOMMON;
		if (rand == 2) Item->ItemRarity = EProjectUMItemRarityEnum::RARE;
		if (rand == 3) Item->ItemRarity = EProjectUMItemRarityEnum::EPIC;
		if (rand == 4) Item->ItemRarity = EProjectUMItemRarityEnum::LEGENDARY;
		Inventory->AddItem(Item);
	}
	BroadcastInventory();
}

void AProjectUMCharacter::AttachEquipment(TSubclassOf<AProjectUMEquipment> Equipment, EEquippableSlotsEnum EquipSlot) {
	if (GetLocalRole() == ROLE_Authority) {
		EquipmentClassMap.Add(EquipSlot, Equipment);
		IncreaseStats(Inventory->EquipmentMap.FindRef(EquipSlot)->Stats);
		StartEquipping(EquipSlot);
	}
}

void AProjectUMCharacter::DeAttachEquipment(EEquippableSlotsEnum EquipSlot) {
	if (GetLocalRole() == ROLE_Authority) {
		EquipmentClassMap.FindRef(EquipSlot) = nullptr;
		EquippedItemMap.FindRef(EquipSlot)->Destroy();
		DecreaseStats(Inventory->EquipmentMap.FindRef(EquipSlot)->Stats);
	}
}

void AProjectUMCharacter::IncreaseStats(TMap<ECharacterStatEnum, float> StatsMap) {
	if (GetLocalRole() == ROLE_Authority) {
		for (TPair<ECharacterStatEnum, float>& Kvp : StatsMap)
		{
			if (Kvp.Key != ECharacterStatEnum::HEALTH && Kvp.Key != ECharacterStatEnum::MANA) {
				SetCurrentStat(Kvp.Key, Kvp.Value + CurrentStatsMap.FindRef(Kvp.Key));
			}
			SetMaxStat(Kvp.Key, Kvp.Value + MaxStatsMap.FindRef(Kvp.Key));

		}
	}
}

void AProjectUMCharacter::DecreaseStats(TMap<ECharacterStatEnum, float> StatsMap) {
	if (GetLocalRole() == ROLE_Authority) {
		for (TPair<ECharacterStatEnum, float>& Kvp : StatsMap)
		{
			if (Kvp.Key != ECharacterStatEnum::HEALTH && Kvp.Key != ECharacterStatEnum::MANA) {
				SetCurrentStat(Kvp.Key,CurrentStatsMap.FindRef(Kvp.Key) - Kvp.Value);
			}
			SetMaxStat(Kvp.Key, MaxStatsMap.FindRef(Kvp.Key) - Kvp.Value);

		}
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
	Interactor->BroadcastNpcLoot(Inventory->GetAllInventoryItems());
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

		AProjectUMCharacter* OtherCharacter = Cast<AProjectUMCharacter>(OtherActor);
		if (OtherCharacter && OtherCharacter->Inventory == LootingInventory) {
			LootingInventory = nullptr;
			CloseLoot();
		}
		AProjectUmNpc* OtherNpc = Cast<AProjectUmNpc>(OtherActor);
		if (OtherNpc && OtherNpc->GetInventory() == LootingInventory) {
			LootingInventory = nullptr;
			CloseLoot();
		}

		AProjectUMLootableProp* OtherProp = Cast<AProjectUMLootableProp>(OtherActor);
		if (OtherProp && OtherProp->GetInventory() == LootingInventory) {
			LootingInventory = nullptr;
			CloseLoot();
		}

	}
}

void AProjectUMCharacter::AddItemToInventory_Implementation(UProjectUMItem* Item) {
	Inventory->AddItem(Item);
	BroadcastInventory();
}

void AProjectUMCharacter::BroadcastNpcLoot_Implementation(const TArray<FItemStruct>& Items) {
	OnNpcCorpseInteracted.Broadcast(Items);
}

void AProjectUMCharacter::OpenLoot_Implementation() {
	OnNpcCorpseInteractedOpenLootWidget.Broadcast();
}

void AProjectUMCharacter::CloseLoot_Implementation() {
	FString msg = "Closing loot";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, msg);
	OnExitLootRange.Broadcast();
}

void AProjectUMCharacter::BroadcastInventory_Implementation() {

	BroadcastInventoryToClient(Inventory->GetAllInventoryItems(), Inventory->GetAllEquippedItems());
}

void AProjectUMCharacter::OpenInventory() {
	FString healthMessage = "Opening Iventory";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	OnInventoryOpen.Broadcast();
}

void AProjectUMCharacter::BroadcastInventoryToClient_Implementation(const TArray<FItemStruct>& InventoryItems, const TArray<FItemStruct>& EquippedItems) {
	OnInventoryOpenDisplayItems.Broadcast(InventoryItems, EquippedItems);
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
	BroadcastNpcLoot(LootingInventory->GetAllInventoryItems());
}

void AProjectUMCharacter::Disconnect() {
	DisconnectClient();
}

void AProjectUMCharacter::DisconnectClient_Implementation() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Loading Map");
	UGameplayStatics::OpenLevel((UObject*)GetGameInstance(), FName(TEXT("MainMenu")));
}