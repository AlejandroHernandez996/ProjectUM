// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableObjectInterface.h"
#include "CharacterStatEnum.h"
#include "ProjectUMInventoryComponent.h"
#include "ProjectUMCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcCorpseInteracted, const TArray<FItemStruct>&, _Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNpcCorpseInteractedOpenLootWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitLootRange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryOpenDisplayItems, const TArray<FItemStruct>&, _InventoryItems, const TArray<FItemStruct>&, _EquippedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryOpen);

UCLASS(config=Game)
class AProjectUMCharacter : public ACharacter, public IInteractableObjectInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iventory", meta = (AllowPrivateAccess = "true"))
		class UProjectUMInventoryComponent* LootingInventory;

	struct FProjectUMCharacterStatsStruct* Stats;

	TMap<ECharacterStatEnum, float> MaxStatsMap;

	TMap<ECharacterStatEnum, float> CurrentStatsMap;

	TMap<ECharacterStatEnum, float> BaseStatsMap;

	void InitStats();

public:
	UFUNCTION(BlueprintPure)
		float GetCurrentStat(ECharacterStatEnum Stat) {
		return CurrentStatsMap[Stat];
	}

	UFUNCTION(BlueprintPure)
		float GetMaxStat(ECharacterStatEnum Stat) { 
		return MaxStatsMap[Stat];
	}

	UFUNCTION(BlueprintPure)
		float GetBaseStat(ECharacterStatEnum Stat) { return BaseStatsMap.FindRef(Stat); }

public:
	AProjectUMCharacter();

	UFUNCTION()
		void SetLootingInventory(UProjectUMInventoryComponent* LootInventory) {
			LootingInventory = LootInventory;
		}

	UFUNCTION()
		UProjectUMInventoryComponent* GetInventory() { return Inventory; }

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnNpcCorpseInteracted OnNpcCorpseInteracted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
		FOnNpcCorpseInteractedOpenLootWidget OnNpcCorpseInteractedOpenLootWidget;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryOpenDisplayItems OnInventoryOpenDisplayItems;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryOpen OnInventoryOpen;


	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnExitLootRange OnExitLootRange;

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentMaxHealth(float healthValue);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentStat(ECharacterStatEnum Stat, float Value);

	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetMaxStat(ECharacterStatEnum Stat, float Value);

	UFUNCTION()
		void IncreaseStats(TMap<ECharacterStatEnum, float> Stats);
	UFUNCTION()
		void DecreaseStats(TMap<ECharacterStatEnum, float> Stats);

	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Attack")
		FORCEINLINE bool IsAttacking() const { return bIsAttacking; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		class UAnimMontage* MeleeAttackMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* FistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collisions, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* InteractComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Instanced, Category = "Inventory")
		TMap<EEquippableSlotsEnum, AProjectUMEquipment*> EquippedItemMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		TMap<EEquippableSlotsEnum, TSubclassOf<AProjectUMEquipment>> EquipmentClassMap;

	UPROPERTY()
		TMap<EEquippableSlotsEnum, FName> EquipSlotSkeletonMapping;

		TSet<IInteractableObjectInterface*> InteractableObjects;

	UFUNCTION()
		TSubclassOf<AProjectUMEquipment> GetEquipmentClass(EEquippableSlotsEnum EquipmentSlot) { return EquipmentClassMap[EquipmentSlot]; }

	UFUNCTION()
		AProjectUMEquipment* GetEquippedItem(EEquippableSlotsEnum EquipmentSlot) { return EquippedItemMap[EquipmentSlot]; }

	UFUNCTION()
		void OnAttackHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void OnInteractOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnInteractOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void AttachEquipment(TSubclassOf<AProjectUMEquipment> Equipment, EEquippableSlotsEnum EquipSlot);

	UFUNCTION()
	void DeAttachEquipment(EEquippableSlotsEnum EquipSlot);


	UFUNCTION(Server, Reliable, Category = "Inventory")
		void AddItemToInventory(class UProjectUMItem* Item);

	void AddItemToInventory_Implementation(class UProjectUMItem* Item);

protected:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void CrouchStarted();

	void CrouchStopped();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_MaxHealth();

	void OnHealthUpdate();

	void OnCurrentStatUpdate(ECharacterStatEnum Stat, float Value);

	void OnMaxStatUpdate(ECharacterStatEnum Stat, float Value);
	
	UFUNCTION(Client, Reliable)
	void OnCurrentStatUpdateClient(ECharacterStatEnum Stat, float Value);

	UFUNCTION(Client, Reliable)
	void OnMaxStatUpdateClient(ECharacterStatEnum Stat, float Value);

	void OnCurrentStatUpdateClient_Implementation(ECharacterStatEnum Stat, float Value);

	void OnMaxStatUpdateClient_Implementation(ECharacterStatEnum Stat, float Value);

	UFUNCTION()
		void OnRep_CurrentMana();

	UFUNCTION()
		void OnRep_MaxMana();

	UFUNCTION()
		void OnRep_CurrentStrength();

	UFUNCTION()
		void OnRep_CurrentAgility();

	UFUNCTION()
		void OnRep_CurrentWisdom();

	UFUNCTION()
		void OnRep_CurrentIntellect();


	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseMaxHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseMana;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseAgility;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseStrength;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseIntellect;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float BaseWisdom;

	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_MaxHealth)
		float MaxHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_MaxMana)
		float MaxMana;							   
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float MaxAgility;						   
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float MaxStrength;						  
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float MaxIntellect;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", Replicated)
		float MaxWisdom;

	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentMana)
		float CurrentMana;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentAgility)
		float CurrentAgility;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentStrength)
		float CurrentStrength;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentIntellect)
		float CurrentIntellect;
	UPROPERTY(EditDefaultsOnly, Category = "Stat", ReplicatedUsing = OnRep_CurrentWisdom)
		float CurrentWisdom;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
		TSubclassOf<class AProjectUmProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float FireRate;

	FTimerHandle FiringTimer;

	bool bIsFiringWeapon;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFire();

	UFUNCTION(Server, Reliable)
		void HandleFire();

	UPROPERTY()
		TSet<FString> AttackedCharactersSet;

	FTimerHandle AttackingTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float AttackRate;

	bool bIsAttacking;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartAttack();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopAttack();

	UFUNCTION(Server, Reliable)
		void HandleAttack();

	void HandleAttack_Implementation();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
		float EquipRate;
	FTimerHandle EquippingTimer;

	bool bIsEquipping;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartEquipping(EEquippableSlotsEnum EquipSlot);

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopEquipping();

	UFUNCTION()
		void HandleEquip(EEquippableSlotsEnum EquipSlot);

	UFUNCTION(NetMulticast, Reliable)
		void PlayProjectUMCharacterAnimMontage(UAnimMontage* AnimMontage);


	UPROPERTY(EditDefaultsOnly, Category = "Item")
		float UseItemRate;

	FTimerHandle UseItemTimer;

	bool bIsUsingItem;

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Item")
		void StartUsingItem(int32 ItemId);

	UFUNCTION(BlueprintCallable, Category = "Item")
		void StopUsingItem();

	UFUNCTION(Server, Reliable, Category = "Item")
		void HandleUseItemServer(int32 ItemId);

	void HandleUseItemServer_Implementation(int32 ItemId);

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Item")
		void StartDroppingItem(int32 ItemId);

	UFUNCTION(BlueprintCallable, Category = "Item")
		void StopDroppingItem();

	UFUNCTION(Server, Reliable, Category = "Item")
		void HandleDropItemServer(int32 ItemId);

	void HandleDropItemServer_Implementation(int32 ItemId);

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Item")
		void StartLootingItem(int32 ItemId);

	UFUNCTION(BlueprintCallable, Category = "Item")
		void StopLootingItem();

	UFUNCTION(Server, Reliable, Category = "Item")
		void HandleLootingItem(int32 ItemId);

	void HandleLootingItem_Implementation(int32 ItemId);

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
		float InteractRate = .25f;

	FTimerHandle InteractTimer;

	bool bIsInteracting;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StartInteracting();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void StopInteracting();

	UFUNCTION(Server, Reliable, Category = "Interaction")
		void HandleInteraction();

	void HandleInteraction_Implementation();

public:
	UFUNCTION(Client, Reliable, Category = "Interaction")
		void BroadcastNpcLoot(const TArray<FItemStruct>& LootItems);

	void BroadcastNpcLoot_Implementation(const TArray<FItemStruct>& LootItems);

	UFUNCTION(Client, Reliable, Category = "Interaction")
		void OpenLoot();

	void OpenLoot_Implementation();

	UFUNCTION(Client, Reliable, Category = "Interaction")
		void CloseLoot();

	void CloseLoot_Implementation();

	UFUNCTION(Server, Reliable, Category = "Interaction")
		void BroadcastInventory();

	void BroadcastInventory_Implementation();

	UFUNCTION(Client, Reliable, Category = "Interaction")
		void BroadcastInventoryToClient(const TArray<FItemStruct>& InventoryItems, const TArray<FItemStruct>& EquippedItems);

	void BroadcastInventoryToClient_Implementation(const TArray<FItemStruct>& InventoryItems, const TArray<FItemStruct>& EquippedItems);

	void OpenInventory();


protected:
	void Interact_Implementation(AProjectUMCharacter* Interactor) override;

	UFUNCTION(Server, Reliable)
		void SpawnItems();

	void SpawnItems_Implementation();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float lastTick;

};

