// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalBoxComponent.h"
#include "GameFramework/Character.h"
#include "sazandora/sazandoraGameMode.h"
#include "Main_Character.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"

UGoalBoxComponent::UGoalBoxComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionObjectType(ECC_WorldStatic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OnComponentBeginOverlap.AddDynamic(this, &UGoalBoxComponent::OnOverlapBegin);
}

void UGoalBoxComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGoalBoxComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 対象のActorがCharacterClassを継承しているか
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Goal reached by %s!"), *OtherActor->GetName());

		// OherActorをMain_Characterに変換
		if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
		{
			// Main_CharacterをMyPlayerControllerに変換
			if (AMyPlayerController* pc = Cast<AMyPlayerController>(player->GetController()))
			{
				// MyPlayerControllerからMyplayerStateに変換
				if (AMyPlayerState* ps = Cast<AMyPlayerState>(pc->PlayerState))
				{
					// GameModeの取得
					AsazandoraGameMode* my_gamemode = GetWorld()->GetAuthGameMode<AsazandoraGameMode>();

					// GameModeのクリアチェックにMyPlayerStateを渡す
					my_gamemode->ClearCheck(ps);
				}
			}
		}
	}
}