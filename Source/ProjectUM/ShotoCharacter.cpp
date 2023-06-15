// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotoCharacter.h"
#include "ShotoProjectile.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Misc/OutputDeviceDebug.h"

AShotoCharacter::AShotoCharacter() : Super() {
   
    ProjectileClass = AShotoProjectile::StaticClass();

    if (IsLocallyControlled())
    {
        UMeshComponent* MeshComponent = GetMesh(); // Replace with your actual mesh component
        if (MeshComponent)
        {
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            MeshComponent->SetGenerateOverlapEvents(false);
        }

        UCapsuleComponent* Capsule = GetCapsuleComponent(); // Replace with your actual capsule component
        if (Capsule)
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Capsule->SetGenerateOverlapEvents(false);
        }
    }

    // set our turn rate for input
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    TurnRateGamepad = 50.f;

    GetCharacterMovement()->JumpZVelocity = 1400.0f;
    GetCharacterMovement()->AirControl = 0.6f;
    GetCharacterMovement()->MaxWalkSpeed = 1200.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    CurrentHealth = 100.0f;
    MaxHealth = 100.0f;
    BlockMeterCurrent = 100.0f;
    BlockMeterMax = 100.0f;
    DashRate = 1.0f;

    PrimaryAirAttackRate = 0.4f;

    SkillAnimMontageMap.Add(ESkillEnums::SHOTO_DRAGON_PUNCH, DragonPunchMontage);
    SkillAnimMontageMap.Add(ESkillEnums::SHOTO_SPIN_KICK, SpinMontage);
    SkillAnimMontageMap.Add(ESkillEnums::SHOTO_BLAST, BlastMontage);


    SkillCooldowns.Add(ESkillEnums::SHOTO_DRAGON_PUNCH, 1.0f);
    SkillCooldowns.Add(ESkillEnums::SHOTO_SPIN_KICK, 3.0f);
    SkillCooldowns.Add(ESkillEnums::SHOTO_CHARGE_UP, 0.0f);
    SkillCooldowns.Add(ESkillEnums::SHOTO_BLAST, 1.0f);

    SkillOnCooldown.Add(ESkillEnums::SHOTO_DRAGON_PUNCH, false);
    SkillOnCooldown.Add(ESkillEnums::SHOTO_SPIN_KICK, false);
    SkillOnCooldown.Add(ESkillEnums::SHOTO_CHARGE_UP, false);
    SkillOnCooldown.Add(ESkillEnums::SHOTO_BLAST, false);

    SkillIndexToEnumMap.Add(1, ESkillEnums::SHOTO_DRAGON_PUNCH);
    SkillIndexToEnumMap.Add(2, ESkillEnums::SHOTO_SPIN_KICK);
    SkillIndexToEnumMap.Add(3, ESkillEnums::SHOTO_CHARGE_UP);
    SkillIndexToEnumMap.Add(4, ESkillEnums::SHOTO_BLAST);

    FTimerHandle TimerHandle1;
    FTimerHandle TimerHandle2;
    FTimerHandle TimerHandle3;
    FTimerHandle TimerHandle4;
    SkillTimers.Add(ESkillEnums::SHOTO_DRAGON_PUNCH, TimerHandle1);
    SkillTimers.Add(ESkillEnums::SHOTO_SPIN_KICK, TimerHandle2);
    SkillTimers.Add(ESkillEnums::SHOTO_CHARGE_UP, TimerHandle3);
    SkillTimers.Add(ESkillEnums::SHOTO_BLAST, TimerHandle4);

    PrimaryAttackRates = {.3f, .3f, .5f};


    if (HasAuthority()) {
        USkeletalMeshComponent* SkeletalMeshComponent = GetMesh(); // Assuming you have a skeletal mesh component for the character
        FName RightHandSocketName = FName("hand_r"); // Replace with the actual socket name

        DragonPunchHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("DragonPunch_Hitbox"));
        SkillHurtboxes.Add(ESkillEnums::SHOTO_DRAGON_PUNCH, DragonPunchHitbox);
        FVector BoxExtent(2.0f, 2.0f, 2.0f);
        DragonPunchHitbox->SetBoxExtent(BoxExtent);

        DragonPunchHitbox->AttachToComponent(SkeletalMeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, RightHandSocketName);
        DragonPunchHitbox->SetGenerateOverlapEvents(false);
        DragonPunchHitbox->SetHiddenInGame(false);
        DragonPunchHitbox->SetVisibility(false);

        DragonPunchHitbox->OnComponentBeginOverlap.AddDynamic(this, &AShotoCharacter::HandleDragonPunchOverlap);
        HitboxAppendageMap["hand_l"]->OnComponentBeginOverlap.AddDynamic(this, &AShotoCharacter::HandlePunchOverlap);
        HitboxAppendageMap["hand_r"]->OnComponentBeginOverlap.AddDynamic(this, &AShotoCharacter::HandlePunchOverlap);
        HitboxAppendageMap["foot_l"]->OnComponentBeginOverlap.AddDynamic(this, &AShotoCharacter::HandleKickOverlap);
        HitboxAppendageMap["foot_r"]->OnComponentBeginOverlap.AddDynamic(this, &AShotoCharacter::HandleKickOverlap);

        HitboxAppendageMap["hand_l"]->SetHiddenInGame(false);
        HitboxAppendageMap["hand_r"]->SetHiddenInGame(false);
        HitboxAppendageMap["foot_l"]->SetHiddenInGame(false);
        HitboxAppendageMap["foot_r"]->SetHiddenInGame(false);

        HitstunDuration = 1.0f;
    }



}

void AShotoCharacter::Tick(float DeltaTime) {
    if (PrimaryAirAttackCounter > 0 && !GetCharacterMovement()->IsFalling()) {
        PrimaryAirAttackCounter = 0;
        bIsPrimaryAttacking = false;
    }
}

void AShotoCharacter::StartJump() {
    if (bIsPrimaryAttacking || bIsInHitstun) return;
    Jump();
}

void AShotoCharacter::HandleDash_Implementation()
{
    if (bIsDashing || bIsPrimaryAttacking || bIsInHitstun)
        return;

    bIsDashing = true;

    // Cancel the character's current velocity
    GetCharacterMovement()->StopMovementImmediately();

    // Get the character's movement input vector
    FVector MovementInput = GetLastMovementInputVector();
    MovementInput.Z = 0.0f; // Ignore vertical input

    // Calculate the dash direction
    FVector DashDirection;
    if (MovementInput.SizeSquared() > SMALL_NUMBER)
    {
        DashDirection = MovementInput.GetSafeNormal();
    }
    else
    {
        DashDirection = GetActorForwardVector();
    }

    // Apply the dash impulse to the character's movement
    float DashImpulseStrength = 2000.0f; // Adjust the dash impulse strength as desired
    FVector DashImpulse = DashDirection * DashImpulseStrength;
    GetCharacterMovement()->AddImpulse(DashImpulse, true);

    // Play the dash animation montage
    PlayAnimationMontageWithOverride(DashMontage);

    // Set up a timer to stop the dash after the specified duration
    GetWorldTimerManager().SetTimer(DashTimer, this, &AShotoCharacter::StopDash, DashRate, false);
}

void AShotoCharacter::StopDash() {
    bIsDashing = false;
}

void AShotoCharacter::HandlePrimaryAttack_Implementation()
{
    Super::HandlePrimaryAttack_Implementation();

    if (bIsPrimaryAttacking || bIsInHitstun) return;
    bIsPrimaryAttacking = true;

    UBoxComponent* Hitbox = nullptr;
    if (GetCharacterMovement()->IsFalling()) {
        if (PrimaryAirAttackCounter % 2 == 0) {
            PlayAnimationMontageWithOverride(PrimaryAirAttackMontageMirror);
            Hitbox = HitboxAppendageMap["foot_r"];
        }
        else {
            PlayAnimationMontageWithOverride(PrimaryAirAttackMontage);
            Hitbox = HitboxAppendageMap["foot_l"];
        }
        PrimaryAirAttackCounter++;
        GetWorldTimerManager().SetTimer(PrimaryAttackTimer, this, &AShotoCharacter::StopPrimaryAttack, PrimaryAirAttackRate, false);
    }
    else {
        if (PrimaryAttackIndex >= PrimaryAttackMontages.Num()) {
            PrimaryAttackIndex = 0;
        }
        PlayAnimationMontageWithOverride(PrimaryAttackMontages[PrimaryAttackIndex]);
        GetWorldTimerManager().SetTimer(PrimaryAttackTimer, this, &AShotoCharacter::StopPrimaryAttack, PrimaryAttackRates[PrimaryAttackIndex], false);

        if (PrimaryAttackIndex == 2) {
            Hitbox = HitboxAppendageMap["foot_l"];

        }
        if (PrimaryAttackIndex == 1) {
            Hitbox = HitboxAppendageMap["hand_r"];

        }
        if (PrimaryAttackIndex == 0) {
            Hitbox = HitboxAppendageMap["hand_l"];

        }
        PrimaryAttackIndex++;

    }
    if (Hitbox) {
        // Set the hitbox to its initial size and enable overlap events
        Hitbox->SetBoxExtent(FVector(2.0f));  // Small initial size, adjust as needed
        Hitbox->SetGenerateOverlapEvents(true);


        // Set up a timer to grow the hitbox
        float AnimationDuration = 0.2f;  // Duration over which the hitbox will grow, adjust as needed
        float TickInterval = 0.01f;  // Time between updates, adjust as needed
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindLambda([this, AnimationDuration, TickInterval, Hitbox] {
            // Calculate the current and target sizes
            FVector CurrentSize = Hitbox->GetUnscaledBoxExtent();
            FVector TargetSize(1000.0f);  // Final size of the hitbox, adjust as needed

            // Calculate the amount to grow per tick
            FVector GrowthPerTick = (TargetSize - CurrentSize) * (TickInterval / AnimationDuration);

            // Increase the current size
            CurrentSize += GrowthPerTick;

            // Set the new size
            Hitbox->SetBoxExtent(CurrentSize);

            // If we've reached the target size, disable the timer
            if (CurrentSize.Size() >= TargetSize.Size()) {
                GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
                // Optionally, disable overlap events here if the hitbox should stop detecting overlaps after growing
                //HitBox->SetGenerateOverlapEvents(false);
            }
            });

        // Start the timer
        GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
        Hitbox->SetVisibility(true);
    }
}

void AShotoCharacter::StopPrimaryAttack() {
    bIsPrimaryAttacking = false;
    HitboxAppendageMap["hand_l"]->SetGenerateOverlapEvents(false);
    HitboxAppendageMap["hand_r"]->SetGenerateOverlapEvents(false);
    HitboxAppendageMap["foot_l"]->SetGenerateOverlapEvents(false);
    HitboxAppendageMap["foot_r"]->SetGenerateOverlapEvents(false);
    FVector BoxExtent(10.0f, 10.0f, 10.0f);
    HitboxAppendageMap["hand_l"]->SetBoxExtent(BoxExtent);
    HitboxAppendageMap["hand_r"]->SetBoxExtent(BoxExtent);
    HitboxAppendageMap["foot_l"]->SetBoxExtent(BoxExtent);
    HitboxAppendageMap["foot_r"]->SetBoxExtent(BoxExtent);
}

void AShotoCharacter::HandleSkill_Implementation(int32 SkillIndex)
{
    ESkillEnums SkillEnum = SkillIndexToEnumMap[SkillIndex];
    if (SkillOnCooldown[SkillEnum] || bIsInHitstun) return;
    SkillOnCooldown[SkillEnum] = true;
    GetWorld()->GetTimerManager().SetTimer(SkillTimers[SkillEnum], [this, SkillIndex]() {
        StopSkill(SkillIndex);
        }, SkillCooldowns[SkillEnum], false);
    switch (SkillIndexToEnumMap[SkillIndex])
    {
    case ESkillEnums::SHOTO_DRAGON_PUNCH:
        HandleDragonPunch();
        break;
    case ESkillEnums::SHOTO_SPIN_KICK:
        HandleSpin();
        break;
    case ESkillEnums::SHOTO_BLAST:
        HandleBlast();
        break;
    default:
        break;
    }
}

void AShotoCharacter::HandleDragonPunch_Implementation() {
    float ForwardStrength = 1000.0f;
    float UpwardStrength = 2000.0f;

    // Calculate the movement direction
    FVector LaunchDirection = GetActorForwardVector() * ForwardStrength + FVector::UpVector * UpwardStrength;

    // Launch the character
    LaunchCharacter(LaunchDirection, true, true);

    // Play the animation
    PlayAnimationMontageWithOverride(SkillAnimMontageMap[ESkillEnums::SHOTO_DRAGON_PUNCH]);

    // Set the hitbox to its initial size and enable overlap events
    DragonPunchHitbox->SetBoxExtent(FVector(2.0f));  // Small initial size, adjust as needed
    DragonPunchHitbox->SetGenerateOverlapEvents(true);

    // Set up a timer to grow the hitbox
    float AnimationDuration = 0.5f;  // Duration over which the hitbox will grow, adjust as needed
    float TickInterval = 0.01f;  // Time between updates, adjust as needed
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindLambda([this, AnimationDuration, TickInterval] {
        // Calculate the current and target sizes
        FVector CurrentSize = DragonPunchHitbox->GetUnscaledBoxExtent();
        FVector TargetSize(10000.0f);  // Final size of the hitbox, adjust as needed

        // Calculate the amount to grow per tick
        FVector GrowthPerTick = (TargetSize - CurrentSize) * (TickInterval / AnimationDuration);

        // Increase the current size
        CurrentSize += GrowthPerTick;

        // Set the new size
        DragonPunchHitbox->SetBoxExtent(CurrentSize);

        // If we've reached the target size, disable the timer
        if (CurrentSize.Size() >= TargetSize.Size()) {
            GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
            // Optionally, disable overlap events here if the hitbox should stop detecting overlaps after growing
            //HitBox->SetGenerateOverlapEvents(false);
        }
        });

    // Start the timer
    GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
    DragonPunchHitbox->SetVisibility(true);

}

void AShotoCharacter::HandleSpin_Implementation() {
    PlayAnimationMontageWithOverride(SkillAnimMontageMap[ESkillEnums::SHOTO_SPIN_KICK]);
    // Set the hitbox to its initial size and enable overlap events
    HitboxAppendageMap["hand_l"]->SetBoxExtent(FVector(10.0f));  // Small initial size, adjust as needed
    HitboxAppendageMap["hand_l"]->SetGenerateOverlapEvents(true);
    HitboxAppendageMap["hand_r"]->SetBoxExtent(FVector(10.0f));  // Small initial size, adjust as needed
    HitboxAppendageMap["hand_r"]->SetGenerateOverlapEvents(true);
}

void AShotoCharacter::HandleBlast_Implementation()
{
    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 10.0f;  // Adjust the spawn offset as needed
    FRotator SpawnRotation = GetActorRotation();

    AShotoProjectile* Projectile = GetWorld()->SpawnActor<AShotoProjectile>(ProjectileClass.Get(), SpawnLocation, SpawnRotation);
    if (Projectile)
    {
        UE_LOG(LogTemp, Warning, TEXT("SET SIZE AND SPEED OIF PROJECTILE: %s"), *Projectile->GetName());
    }
    PlayAnimationMontageWithOverride(SkillAnimMontageMap[ESkillEnums::SHOTO_BLAST]);
}

void AShotoCharacter::HandleProjectileSpawn_Implementation() {
    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 10.0f;  // Adjust the spawn offset as needed
    FRotator SpawnRotation = GetActorRotation();

    AShotoProjectile* Projectile = GetWorld()->SpawnActor<AShotoProjectile>(ProjectileClass.Get(), SpawnLocation, SpawnRotation);
    if (Projectile)
    {
        UE_LOG(LogTemp, Warning, TEXT("SET SIZE AND SPEED OIF PROJECTILE: %s"), *Projectile->GetName());
    }
    // Play the blast animation or perform any other necessary actions
}

void AShotoCharacter::StopSkill(int32 SkillIndex) {
    SkillOnCooldown[SkillIndexToEnumMap[SkillIndex]] = false;
    DragonPunchHitbox->SetGenerateOverlapEvents(false);
    DragonPunchHitbox->SetBoxExtent(FVector(2.0f));
    DragonPunchHitbox->SetVisibility(false);
    HitboxAppendageMap["hand_l"]->SetGenerateOverlapEvents(false);
    HitboxAppendageMap["hand_r"]->SetGenerateOverlapEvents(false);
}

void AShotoCharacter::HandleDragonPunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("THIS HAS OVERLAPPED: %s"), *OverlappedComponent->GetName());

    // If the overlapping actor is not this actor, and it is not null, and the overlapped component is the HitBox
    if (OtherActor != this && OtherActor != nullptr && OverlappedComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HitBox overlap with: %s"), *OtherActor->GetName());

        // Apply damage
        ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor);
        if (OtherCharacter)
        {
            UGameplayStatics::ApplyDamage(OtherCharacter, 1.0f, GetController(), this, UDamageType::StaticClass());

            // Calculate knockback direction (away from this character)
            FVector KnockbackDirection = OtherCharacter->GetActorLocation() - GetActorLocation();
            KnockbackDirection.Normalize();

            // Apply knockback forces
            float UpwardKnockbackStrength = 4000.0f;  // Adjust as needed
            float ForwardKnockbackStrength = 1000.0f; // Adjust as needed

            // Apply upward knockback force
            FVector UpwardKnockbackImpulse = FVector::UpVector * UpwardKnockbackStrength;
            OtherCharacter->LaunchCharacter(UpwardKnockbackImpulse, false, true);

            // Delay the forward knockback force for a short duration (adjust as needed)
            float ForwardKnockbackDelay = 0.2f;
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindLambda([=]() {
                // Apply forward knockback force after the delay
                FVector ForwardKnockbackImpulse = KnockbackDirection * ForwardKnockbackStrength;
                OtherCharacter->LaunchCharacter(ForwardKnockbackImpulse, true, true);
                });
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, ForwardKnockbackDelay, false);

            UE_LOG(LogTemp, Warning, TEXT("Applied knockback to: %s"), *OtherActor->GetName());
        }
    }
}


void AShotoCharacter::HandlePunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("THIS  PUNCH HAS OVERLAPPED: %s"), *OverlappedComponent->GetName());

    // If the overlapping actor is not this actor, and it is not null, and the overlapped component is the HitBox
    if (OtherActor != this && OtherActor != nullptr && OverlappedComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HitBox overlap with: %s"), *OtherActor->GetName());

        // Apply damage
        ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor);
        if (OtherCharacter)
        {
            UGameplayStatics::ApplyDamage(OtherCharacter, 1.0f, GetController(), this, UDamageType::StaticClass());
        }
    }
}

void AShotoCharacter::HandleKickOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("THIS KICK HAS OVERLAPPED: %s"), *OverlappedComponent->GetName());

    // If the overlapping actor is not this actor, and it is not null, and the overlapped component is the HitBox
    if (OtherActor != this && OtherActor != nullptr && OverlappedComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("HitBox overlap with: %s"), *OtherActor->GetName());

        // Apply damage
        ACharacter* OtherCharacter = Cast<ACharacter>(OtherActor);
        if (OtherCharacter)
        {
            UGameplayStatics::ApplyDamage(OtherCharacter, 2.0f, GetController(), this, UDamageType::StaticClass());

            // Calculate knockback direction (away from this character)
            FVector KnockbackDirection = OtherCharacter->GetActorLocation() - GetActorLocation();
            KnockbackDirection.Normalize();

            // Apply knockback forces
            float UpwardKnockbackStrength = 0.0f;  // Adjust as needed
            float ForwardKnockbackStrength = 2000.0f; // Adjust as needed

            // Apply upward knockback force
            FVector UpwardKnockbackImpulse = FVector::UpVector * UpwardKnockbackStrength;
            OtherCharacter->LaunchCharacter(UpwardKnockbackImpulse, false, true);

            // Delay the forward knockback force for a short duration (adjust as needed)
            float ForwardKnockbackDelay = 0.2f;
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindLambda([=]() {
                // Apply forward knockback force after the delay
                FVector ForwardKnockbackImpulse = KnockbackDirection * ForwardKnockbackStrength;
                OtherCharacter->LaunchCharacter(ForwardKnockbackImpulse, true, true);
                });
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, ForwardKnockbackDelay, false);

            UE_LOG(LogTemp, Warning, TEXT("Applied knockback to: %s"), *OtherActor->GetName());
        }
    }
}