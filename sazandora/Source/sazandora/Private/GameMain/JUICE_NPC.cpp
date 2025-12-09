// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/JUICE_NPC.h"
#include "../Public/GameMain/MyPlayerState.h"
#include "../Public/GameMain/Main_Character.h"

AJUICE_NPC::AJUICE_NPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 会話範囲にプレイヤーがいるかどうか
	Is_Talk_Flg = false;

	// 販売するアイテム
	e_mytype = E_ITEM_TYPE::E_JUICE;
}

// Called when the game starts or when spawned
void AJUICE_NPC::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AJUICE_NPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
void AJUICE_NPC::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//// サーバーでのみ実行
	//if (!HasAuthority())
	//{
	//	return;
	//}

	//// Playerが会話範囲に入ったかどうか(OtherActerがAMain_Characterクラスと同じか調べてる）
	//if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
	//{
	//	Is_Talk_Flg = true;

	//	player->Set_NPC_Pointer(this);
	//	player->Set_Talk_Flg(this->Is_Talk_Flg);
	//}

	Super::OnPlayerEnterRange(OverlappedComp, OtherActor, OtherComo, OtherBodyIndex, bFromSweep, SweepResult);

}

void AJUICE_NPC::OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//// サーバーでのみ実行
	//if (!HasAuthority())
	//{
	//	return;
	//}

	//// playerが会話範囲から出たかどうか(OtherActerがAMain_Characterクラスと同じか調べてる）
	//if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
	//{
	//	// テキストの表示
	//	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is out"));

	//	Is_Talk_Flg = false;

	//	player->Set_NPC_Pointer(nullptr);
	//	player->Set_Talk_Flg(this->Is_Talk_Flg);
	//}

	Super::OnPlayerLeaveRange(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

}

// 会話イベント
void AJUICE_NPC::Talk_Event(AMain_Character* player)
{
	// プレイヤーの買い物リストを保存
	//TArray<E_ITEM_TYPE> p_buylist = player->Get_ItemList();

	UE_LOG(LogTemp, Warning, TEXT("[NPC Talk_Event] Authority: %d | NetMode: %d"), HasAuthority(), (int)GetNetMode());

	// サーバーのみ実行
	if (!HasAuthority())
	{
		return;
	}

	if (!player)
	{
		UE_LOG(LogTemp, Warning, TEXT("player is nullptr"));
		return;
	}

	AMyPlayerState* player_state = player->GetPlayerState<AMyPlayerState>();

	UE_LOG(LogTemp, Warning, TEXT("UniqueID = %s"),
		*player_state->GetUniqueId().ToString());


	TArray<E_ITEM_TYPE> p_buylist = player_state->player_buy_list;

	//// 数値をFStringに変換
	for (int32 i = 0; i < 3; i++)
	{
		/*FString LogMessage = FString::Printf(TEXT("配列の要素: %d"), p_buylist[i]);
		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);*/
	}

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

				UE_LOG(LogTemp, Log, TEXT("player is buy "));
			}
		}
	}
	else
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is not buy"));

		UE_LOG(LogTemp, Log, TEXT("player not buy "));
	}
}