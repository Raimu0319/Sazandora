// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_Character.h"
#include "Main_Character.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ANPC_Character::ANPC_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 会話範囲の設定（スフィアコリジョン）
	TalkRange = CreateDefaultSubobject<USphereComponent>(TEXT("Spheres"));
	TalkRange->SetupAttachment(RootComponent);
	TalkRange->SetSphereRadius(200.0f);
	TalkRange->SetCollisionProfileName(TEXT("Trigger"));

	// スフィアコリジョンのOverlapイベントをバインド
	TalkRange->OnComponentBeginOverlap.AddDynamic(this, &ANPC_Character::OnPlayerEnterRange);
	TalkRange->OnComponentEndOverlap.AddDynamic(this, &ANPC_Character::OnPlayerLeaveRange);

	// 会話範囲にプレイヤーがいるかどうか
	Is_Talk_Flg = false;

	// 販売するアイテム
	e_mytype = E_ITEM_TYPE::E_NONE;

	// meshをセット
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);						//meshの設定
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));	//座標の設定
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));	//回転の設定
	}
}

// Called when the game starts or when spawned
void ANPC_Character::BeginPlay()
{
	Super::BeginPlay();


	
}

// Called every frame
void ANPC_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 会話開始関数（プレイヤーが近づいたときなどに呼ぶ）
void ANPC_Character::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Playerが会話範囲に入ったかどうか(OtherActerがAMain_Characterクラスと同じか調べてる）
	if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
	{
		Is_Talk_Flg = true;

		player->Set_NPC_Pointer(this);
		player->Set_Talk_Flg(this->Is_Talk_Flg);
	}
}

void ANPC_Character::OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// playerが会話範囲から出たかどうか(OtherActerがAMain_Characterクラスと同じか調べてる）
	if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is out"));

		Is_Talk_Flg = false;

		player->Set_NPC_Pointer(nullptr);
		player->Set_Talk_Flg(this->Is_Talk_Flg);
	}
}

void ANPC_Character::Talk_Event(TArray<E_ITEM_TYPE> p_type)
{
	// テキストの表示
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Player is in"));
}