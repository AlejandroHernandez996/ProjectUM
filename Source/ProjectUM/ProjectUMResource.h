// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectUMItemGenerator.h"
#include "ToolTypeEnum.h"
#include "ProjectUMResource.generated.h"

UCLASS()
class PROJECTUM_API AProjectUMResource : public AActor, public IProjectUMItemGenerator
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectUMResource();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Resource")
		class UProjectUMItem* OutputItem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Resource")
		int32 Capacity;

	// Sphere component used to test collision.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
		class UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		EToolTypeEnum ToolType = EToolTypeEnum::NONE;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	EToolTypeEnum GetToolType() { return ToolType; }

	void Generate_Implementation(AProjectUMCharacter* CharacterToOutput) override;

};
