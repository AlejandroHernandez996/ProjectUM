// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectUMGameMode.h"
#include "ProjectUMCharacter.h"
#include "ProjectUMGameState.h"
#include "Json.h"
#include "UObject/ConstructorHelpers.h"

AProjectUMGameMode::AProjectUMGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}

void AProjectUMGameMode::BeginPlay()
{
	Super::BeginPlay();

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AProjectUMGameMode::OnResponseReceived);
	Request->SetURL("https://eqsntvit1c.execute-api.us-east-2.amazonaws.com/items/1");
	Request->SetVerb("GET");
	Request->ProcessRequest();

}

void AProjectUMGameMode::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
}