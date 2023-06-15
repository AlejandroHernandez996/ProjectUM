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
#include "ProjectUMGameMode.h"
#include "Components/SphereComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "ProjectUMGameState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"


//////////////////////////////////////////////////////////////////////////
// AProjectUMCharacter

AProjectUMCharacter::AProjectUMCharacter()
{
	// Set size for collision capsule
	PrimaryActorTick.bCanEverTick = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rat

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && HasAuthority())
	{
		// Create and attach hitboxes for the hands
		UBoxComponent* LeftHandHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHandHitbox"));
		LeftHandHitbox->SetupAttachment(MeshComponent, "hand_l");
		LeftHandHitbox->SetCollisionProfileName("Hitbox");
		LeftHandHitbox->SetBoxExtent(FVector(10.0f));
		LeftHandHitbox->SetGenerateOverlapEvents(false);
		LeftHandHitbox->SetVisibility(true);
		HitboxAppendageMap.Add("hand_l", LeftHandHitbox);

		UBoxComponent* RightHandHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandHitbox"));
		RightHandHitbox->SetupAttachment(MeshComponent, "hand_r");
		RightHandHitbox->SetCollisionProfileName("Hitbox");
		RightHandHitbox->SetBoxExtent(FVector(10.0f));
		RightHandHitbox->SetGenerateOverlapEvents(false);
		RightHandHitbox->SetVisibility(true);
		HitboxAppendageMap.Add("hand_r", RightHandHitbox);

		// Create and attach hitboxes for the feet
		UBoxComponent* LeftFootHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFootHitbox"));
		LeftFootHitbox->SetupAttachment(MeshComponent, "foot_l");
		LeftFootHitbox->SetCollisionProfileName("Hitbox");
		LeftFootHitbox->SetBoxExtent(FVector(10.0f));
		LeftFootHitbox->SetGenerateOverlapEvents(false);
		LeftFootHitbox->SetVisibility(true);
		HitboxAppendageMap.Add("foot_l", LeftFootHitbox);

		UBoxComponent* RightFootHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFootHitbox"));
		RightFootHitbox->SetupAttachment(MeshComponent, "foot_r");
		RightFootHitbox->SetCollisionProfileName("Hitbox");
		RightFootHitbox->SetBoxExtent(FVector(10.0f));
		RightFootHitbox->SetGenerateOverlapEvents(false);
		RightFootHitbox->SetVisibility(true);
		HitboxAppendageMap.Add("foot_r", RightFootHitbox);
	}
}

// Called every frame
void AProjectUMCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Regenerate the block meter
	if (!bIsBlocking && BlockMeterCurrent < BlockMeterMax)
	{
		BlockMeterCurrent += DeltaTime * BlockRegenerationRate;
		BlockMeterCurrent = FMath::Clamp(BlockMeterCurrent, 0.0f, BlockMeterMax);
	}
}


void AProjectUMCharacter::Dance_Implementation() {
	PlayProjectUMCharacterAnimMontage(DanceMontage, "start_1");
}


// Called after constructor
void AProjectUMCharacter::BeginPlay() {
	Super::BeginPlay();
	UpdateHealthAndBlockMeter();

}


//////////////////////////////////////////////////////////////////////////
// Input

void AProjectUMCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AProjectUMCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AProjectUMCharacter::CrouchStarted);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AProjectUMCharacter::CrouchStopped);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AProjectUMCharacter::StartDash);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AProjectUMCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AProjectUMCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AProjectUMCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AProjectUMCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Primary Attack", IE_Pressed, this, &AProjectUMCharacter::StartPrimaryAttack);

	DECLARE_DELEGATE_OneParam(SkillBindDelegate, int32);
	PlayerInputComponent->BindAction<SkillBindDelegate>("Skill_1", IE_Pressed, this, &AProjectUMCharacter::StartSkill, 1);
	PlayerInputComponent->BindAction<SkillBindDelegate>("Skill_2", IE_Pressed, this, &AProjectUMCharacter::StartSkill, 2);
	PlayerInputComponent->BindAction<SkillBindDelegate>("Skill_3", IE_Pressed, this, &AProjectUMCharacter::StartSkill, 3);
	PlayerInputComponent->BindAction<SkillBindDelegate>("Skill_4", IE_Pressed, this, &AProjectUMCharacter::StartSkill, 4);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AProjectUMCharacter::StartBlock);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AProjectUMCharacter::StopBlock);

	PlayerInputComponent->BindAction("Dance", IE_Pressed, this, &AProjectUMCharacter::Dance);
}

void AProjectUMCharacter::PlayProjectUMCharacterAnimMontage_Implementation(UAnimMontage* AnimMontage, FName StartSectionName) {
	PlayAnimMontage(AnimMontage, 1.0f, StartSectionName);
}

void AProjectUMCharacter::PlayAnimationMontageWithOverride_Implementation(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		FMontageBlendSettings BlendSettings;
		BlendSettings.BlendMode = EMontageBlendMode::Standard;
		BlendSettings.BlendProfile = nullptr;
		BlendSettings.Blend.BlendOption = EAlphaBlendOption::HermiteCubic;

		AnimInstance->Montage_PlayWithBlendSettings(Montage, BlendSettings);
	}
}

void AProjectUMCharacter::StartBlock()
{
	HandleStartBlock();
}

void AProjectUMCharacter::StopBlock()
{
	HandleStopBlock();
}

void AProjectUMCharacter::HandleStartBlock_Implementation()
{
	if (!bIsBlocking && !bIsInHitstun)
	{
		bIsBlocking = true;
		PlayAnimationMontageWithOverride(BlockMontage);
	}
}

void AProjectUMCharacter::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == BlockMontage && !bInterrupted)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_SetPlayRate(BlockMontage, 0.0f);
		}
	}
}

void AProjectUMCharacter::HandleStopBlock_Implementation()
{
	if (bIsBlocking)
	{
		bIsBlocking = false;
		PlayAnimationMontageWithOverride(EndBlockMontage);
	}
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

void AProjectUMCharacter::StartJump() {
}

void AProjectUMCharacter::StartDash() {
	HandleDash();
}

void AProjectUMCharacter::StopDash() {
}

void AProjectUMCharacter::HandleDash_Implementation()
{
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
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);

		// Calculate the rotation based on the movement input
		if (!GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero())
		{
			FRotator TargetRotation = GetCharacterMovement()->GetCurrentAcceleration().Rotation();
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
			SetActorRotation(NewRotation);
		}
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
	
	DOREPLIFETIME(AProjectUMCharacter, BlockMeterCurrent);
	DOREPLIFETIME(AProjectUMCharacter, BlockMeterMax);

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
	if (HasAuthority()) {

		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
	else {
		UNiagaraFunctionLibrary::SpawnSystemAttached(SparkEffectComponent, GetMesh(), NAME_None, FVector(0.f, 0.f, 10.0f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
	}

	if (CurrentHealth == 0) {
		Destroy();
	}
	
}

void AProjectUMCharacter::OnRep_CurrentBlockMeter()
{
	OnBlockMeterUpdate();
}

void AProjectUMCharacter::OnRep_MaxBlockMeter()
{
}

void AProjectUMCharacter::OnBlockMeterUpdate()
{
	if (HasAuthority()) {

		FString healthMessage = FString::Printf(TEXT("%s now has %f block meter remaining."), *GetFName().ToString(), BlockMeterCurrent);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
}

void AProjectUMCharacter::SetMaxBlockMeter(float blockMeterValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		BlockMeterMax = FMath::Clamp(blockMeterValue, 0.f, 1000.f);
	}
	if (BlockMeterCurrent > BlockMeterMax) {
		FString fd = FString::Printf(TEXT("%s now has %f health remaining due to being higher than max health."), *GetFName().ToString(), BlockMeterCurrent);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, fd);
		SetCurrentBlockMeter(BlockMeterMax);
	}
}

void AProjectUMCharacter::SetCurrentBlockMeter(float blockMeterValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		BlockMeterCurrent = FMath::Clamp(blockMeterValue, 0.f, BlockMeterMax);
		OnBlockMeterUpdate();
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

float AProjectUMCharacter::TakeDamage(float DamageTaken, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = 0.0f;
	if (bIsBlocking)
	{
		// Reduce the damage taken based on the block meter
		float blockMeterDamageApplied = BlockMeterCurrent - DamageTaken;
		if (blockMeterDamageApplied < 0.0f) {
			damageApplied = CurrentHealth + blockMeterDamageApplied;
			SetCurrentHealth(damageApplied);
		}
		SetCurrentBlockMeter(blockMeterDamageApplied);

		// Decrease the block meter
		if (BlockMeterCurrent <= 0.0f)
		{
			BlockMeterCurrent = 0.0f;
			StopBlock();
		}
	}
	else
	{
		damageApplied = CurrentHealth - DamageTaken;
		SetCurrentHealth(damageApplied);

		if (HitstunIndex >= HitstunMontages.Num()) {
			HitstunIndex = 0;
		}
		PlayAnimationMontageWithOverride(HitstunMontages[HitstunIndex]);
		HitstunIndex++;
		bIsInHitstun = true;
		GetWorldTimerManager().SetTimer(HitstunTimerHandle, this, &AProjectUMCharacter::EndHitstun, HitstunDuration, false);
	}
	UpdateHealthAndBlockMeter();
	return damageApplied;

}

void AProjectUMCharacter::UpdateHealthAndBlockMeter_Implementation() {
	OnHealthAndBlockUpdate.Broadcast({ CurrentHealth, BlockMeterCurrent });
}

void AProjectUMCharacter::StartPrimaryAttack()
{
	HandlePrimaryAttack();
}

void AProjectUMCharacter::StopPrimaryAttack() {

}

void AProjectUMCharacter::HandlePrimaryAttack_Implementation()
{
}

void AProjectUMCharacter::HandleSkill_Implementation(int32 SkillIndex) {

}

void AProjectUMCharacter::StartSkill(int32 SkillIndex)
{
	HandleSkill(SkillIndex);
}

void AProjectUMCharacter::StopSkill(int32 SkillIndex)
{
}

void AProjectUMCharacter::Disconnect() {
	DisconnectClient();
}

void AProjectUMCharacter::DisconnectClient_Implementation() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Loading Map");
	UGameplayStatics::OpenLevel((UObject*)GetGameInstance(), FName(TEXT("MainMenu")));
}