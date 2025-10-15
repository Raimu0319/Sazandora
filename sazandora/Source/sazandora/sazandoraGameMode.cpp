// Copyright Epic Games, Inc. All Rights Reserved.

#include "sazandoraGameMode.h"
#include "Main_Character.h"
#include "UObject/ConstructorHelpers.h"

AsazandoraGameMode::AsazandoraGameMode()
{
	DefaultPawnClass = AMain_Character::StaticClass();
}
