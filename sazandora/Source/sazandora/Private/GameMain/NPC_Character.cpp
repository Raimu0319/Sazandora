// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/NPC_Character.h"
#include "../Public/GameMain/Main_Character.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

#define RED_OUTLINE		(1)
#define GREEN_OUTLINE		(2)

// Sets default values
ANPC_Character::ANPC_Character()
{
	// クラスを複製可能にするフラグ
	bReplicates = true;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// カプセルコンポーネントの追加
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(RootComponent);
	Capsule->SetCapsuleRadius(50.0f);
	Capsule->SetCollisionProfileName(TEXT("Trigger"));

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 会話範囲の設定（スフィアコリジョン）
	TalkRange = CreateDefaultSubobject<USphereComponent>(TEXT("Spheres"));
	TalkRange->SetupAttachment(RootComponent);
	TalkRange->SetSphereRadius(80.0f);
	TalkRange->SetCollisionProfileName(TEXT("Trigger"));

	//// スフィアコリジョンのOverlapイベントをバインド
	TalkRange->OnComponentBeginOverlap.AddDynamic(this, &ANPC_Character::OnPlayerEnterRange);
	TalkRange->OnComponentEndOverlap.AddDynamic(this, &ANPC_Character::OnPlayerLeaveRange);
	
	TalkRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TalkRange->SetCollisionObjectType(ECC_WorldDynamic);
	TalkRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	TalkRange->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);		// SetCollisionResponseToChannel(可視化するかどうか、当たり判定の設定)

	// 会話範囲にプレイヤーがいるかどうか
	Is_Talk_Flg = false;

	// 販売するアイテム
	e_mytype = E_ITEM_TYPE::E_NONE;

	// meshをセット
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMat_Red(TEXT("/Game/ThirdPerson/Material/M_PostProcess_Outline.M_PostProcess_Outline"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMat_Green(TEXT("/Game/ThirdPerson/Material/M_PostProcess_Outline_Green.M_PostProcess_Outline_Green"));

	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);						//meshの設定
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));	//座標の設定
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));	//回転の設定
		GetMesh()->SetRenderCustomDepth(false);
		GetMesh()->SetCustomDepthStencilValue(RED_OUTLINE);

		if (OverlayMat_Red.Succeeded())
		{
			OverlayMaterial = OverlayMat_Red.Object;
		}
		if (OverlayMat_Green.Succeeded())
		{
			OverlayMaterial_Is_Talk = OverlayMat_Green.Object;
		}
	}
}

// Called when the game starts or when spawned
void ANPC_Character::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		if (OverlayMaterial)
		{
			GetMesh()->SetOverlayMaterial(OverlayMaterial);
		}
	}
}

void ANPC_Character::ChangeOutlineVisibility(bool flg)
{
	// 値に変更があった場合のみ変更
	if (GetMesh() && is_outline != flg)
	{
		GetMesh()->SetRenderCustomDepth(flg);
		is_outline = flg;
		UE_LOG(LogTemp, Log, TEXT("Outline : %s"), flg ? TEXT("true") : TEXT("false"));
	}
}

// Playerとの会話可能な状態ならアウトラインの色を変更する
void ANPC_Character::Is_TalkCheck(bool flg)
{

	//if (!IsLocallyControlled())
	//{
	//	return;
	//}

	if (flg)
	{
		if (OverlayMaterial_Is_Talk)
		{
			GetMesh()->SetCustomDepthStencilValue(GREEN_OUTLINE);
			UE_LOG(LogTemp, Log, TEXT("SetOverlayMaterial(GREEN_OUTLINE) is Success"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("SetOverlayMaterial(GREEN_OUTLINE) is Failed..."));
		}
	}
	else
	{
		if (OverlayMaterial)
		{
			GetMesh()->SetCustomDepthStencilValue(RED_OUTLINE);
			UE_LOG(LogTemp, Log, TEXT("SetOverlayMaterial(RED_OUTLINE) is Success"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("SetOverlayMaterial(RED_OUTLINE) is Failed..."));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Is_TalkCheck : %s"), flg ? TEXT("true") : TEXT("false"));
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
		UE_LOG(LogTemp, Log, TEXT("Outline is true"));
	}
}

// 会話範囲にいたら
void ANPC_Character::OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// playerが会話範囲から出たかどうか(OtherActerがAMain_Characterクラスと同じか調べてる）
	if (AMain_Character* player = Cast<AMain_Character>(OtherActor))
	{
		// テキストの表示
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is out"));

		Is_Talk_Flg = false;
		UE_LOG(LogTemp, Log, TEXT("Outline is false"));
	}
}

void ANPC_Character::Talk_Event(AMain_Character* player)
{
	// テキストの表示
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Player is in"));
}

// 販売アイテムの取得
E_ITEM_TYPE ANPC_Character::Get_ItemType()
{
	return this->e_mytype;
}

void ANPC_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANPC_Character, Is_Talk_Flg);
}
