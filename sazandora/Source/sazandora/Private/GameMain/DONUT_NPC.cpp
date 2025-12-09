// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/DONUT_NPC.h"
#include "../Public/GameMain/MyPlayerState.h"
#include "../Public/GameMain/Main_Character.h"

ADONUT_NPC::ADONUT_NPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 会話範囲にプレイヤーがいるかどうか
	Is_Talk_Flg = false;

	// 販売するアイテム
	e_mytype = E_ITEM_TYPE::E_DONUT;
}

// Called when the game starts or when spawned
void ADONUT_NPC::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADONUT_NPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
void ADONUT_NPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnPlayerEnterRange(OverlappedComp, OtherActor, OtherComo, OtherBodyIndex, bFromSweep, SweepResult);
}

void ADONUT_NPC::OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnPlayerLeaveRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void ADONUT_NPC::Talk_Event(AMain_Character* player)
{
	// プレイヤーの買い物リストを保存
	//TArray<E_ITEM_TYPE> p_buylist = player->Get_ItemList();
	
	// サーバーのみ実行
	if (!HasAuthority())
	{
		return;
	}

	AMyPlayerState* player_state = player->GetPlayerState<AMyPlayerState>();

	TArray<E_ITEM_TYPE> p_buylist = player_state->player_buy_list;

	// 自分が販売しているアイテムがあるかどうか
	if (p_buylist.Contains(e_mytype))
	{
		//ある場合は何番目にあるか探す
		for (int32 i = 0; i < p_buylist.Num(); i++)
		{
			// 同じアイテムを見つけたら
			if (p_buylist[i] == e_mytype)
			{
				player_state->Buy_Item(i, true);

				// テキストの表示
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is buy"));
			}
		}
	}
	else
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is not buy"));
	}

}