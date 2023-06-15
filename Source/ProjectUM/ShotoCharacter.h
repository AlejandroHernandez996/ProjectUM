// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectUMCharacter.h"
#include "Delegates/Delegate.h"
#include "ShotoCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTUM_API AShotoCharacter : public AProjectUMCharacter
{
	GENERATED_BODY()

public:

	AShotoCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		TArray<class UAnimMontage*> PrimaryAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* PrimaryAirAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* PrimaryAirAttackMontageMirror;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* DragonPunchMontage;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* SpinMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* BlastMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* DashMontage;

	UPROPERTY(VisibleAnywhere)
		UBoxComponent* DragonPunchHitbox;

	void HandlePrimaryAttack_Implementation() override;

	void HandleDash_Implementation() override;

	void StopDash() override;

	void StopPrimaryAttack() override;

	void HandleSkill_Implementation(int32 SkillIndex) override;

	UFUNCTION(Server, Reliable)
		void HandleDragonPunch();

	void HandleDragonPunch_Implementation();

	UFUNCTION(Server, Reliable)
		void HandleSpin();

	void HandleSpin_Implementation();

	UFUNCTION(Server, Reliable)
		void HandleBlast();

	void HandleBlast_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void HandleProjectileSpawn();

	void HandleProjectileSpawn_Implementation();

	void StopSkill(int32 SkillIndex);

	virtual void StartJump() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
		TSubclassOf<class AShotoProjectile> ProjectileClass;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleDragonPunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void HandlePunchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void HandleKickOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	uint32 PrimaryAirAttackCounter;

	float PrimaryAirAttackRate;
	float PrimaryAttackRate;
};
