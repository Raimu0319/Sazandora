// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "sazandoraCharacter.h"
#include "UObject/ConstructorHelpers.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("C:/Sazandora/sazandora/Source/sazandora/Public/Main_Character.h"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
