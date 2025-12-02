// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalBoxComponent.h"
#include "GameFramework/Character.h"
#include "sazandora/sazandoraGameMode.h"
#include "Main_Character.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"

UGoalBoxComponent::UGoalBoxComponent()
{
	/*SetIsReplicated(true);*/
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionObjectType(ECC_WorldStatic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);

	OnComponentBeginOverlap.AddDynamic(this, &UGoalBoxComponent::OnOverlapBegin);
}

void UGoalBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER GoalBox CollisionEnabled=%d ObjectType=%d"),
			(int)GetCollisionEnabled(), (int)GetCollisionObjectType());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client GoalBox CollisionEnabled=%d ObjectType=%d"),
			(int)GetCollisionEnabled(), (int)GetCollisionObjectType());
	}
}

void UGoalBoxComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("OnOverlapBegin called! Authority: %d"), GetOwner()->HasAuthority());

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("this call client"))
			return;
	}

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

					// 探したいPlayerStartのタグ作成	例）CurrentPlayerIndexが0ならStartPoint_0を作成、1ならStartPoint_1になる
					const FName TargetTag = FName(*FString::Printf(TEXT("GoalPoint_%d"), ps->player_number));

					// 自身についているタグが探したいタグと同じか
					if (GetOwner()->ActorHasTag(TargetTag))
					{
						UE_LOG(LogTemp, Warning, TEXT("GoalPoint->Tags.Num() = %d"), GetOwner()->Tags.Num());

						UE_LOG(LogTemp, Error, TEXT("GoalPoint Name = %s"), *GetOwner()->GetName());

						// どのPlayerStartを探しているか、どのPlayerStartをチェックしているかを出力
						UE_LOG(LogTemp, Log, TEXT("Checking GoalPoint at %s. TargetTag: %s"),
							*GetOwner()->GetActorLocation().ToString(),
							*TargetTag.ToString());

						// GameModeのクリアチェックにMyPlayerStateを渡す
						my_gamemode->ClearCheck(ps);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s tag not find...") , TargetTag);
					}

				}
			}
		}
	}
}