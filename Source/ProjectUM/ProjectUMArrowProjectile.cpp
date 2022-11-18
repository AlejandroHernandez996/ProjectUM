// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectUMArrowProjectile.h"

AProjectUMArrowProjectile::AProjectUMArrowProjectile() {
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/Models/Arrow.Arrow"));

	//Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.05f));
	}

}