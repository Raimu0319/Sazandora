// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/GameMain/GoalActor.h"

// Sets default values
AGoalActor::AGoalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;		// サーバー権限をもたせる
	bAlwaysRelevant = true;			// 必須
	bNetLoadOnClient = true;		// マップから必ず読み込む
	NetCullDistanceSquared = 0;		// サーバーで省略されない
	SetReplicateMovement(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	GoalBox = CreateDefaultSubobject<UGoalBoxComponent>(TEXT("GoalBox"));
	GoalBox->SetupAttachment(RootComponent);
	GoalBox->SetIsReplicated(true);

	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	GoalMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMat(TEXT("/Game/ThirdPerson/Material/M_PostProcess_Outline.M_PostProcess_Outline"));
	
	if (MeshObj.Succeeded())
	{
		GoalMesh->SetStaticMesh(MeshObj.Object);
	}

	GoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// コリジョン無効
	GoalMesh->SetRenderInMainPass(false);								// 描画しない
	GoalMesh->SetRenderCustomDepth(is_outline);								// CustomDpthに描画
	GoalMesh->SetCustomDepthStencilValue(2);							// Stencil値

	if (OverlayMat.Succeeded())
	{
		OverlayMaterial = OverlayMat.Object;
	}

	// サイズや可視化の設定
	GoalBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	GoalBox->SetHiddenInGame(true);

	UE_LOG(LogTemp, Warning, TEXT("GoalActor Constructor: %s"), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("AGoalActor Constructor!"));

}

// Called when the game starts or when spawned
void AGoalActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] GoalActor BeginPlay: %s"), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] GoalActor BeginPlay: %s"), *GetName());
	}

	//UE_LOG(LogTemp, Warning, TEXT("GoalActor BeginPlay!"));

	if (GoalBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("GoalBox exists. Activating..."));
		GoalBox->Activate(true);
	}

	if (OverlayMaterial)
	{
		GoalMesh->SetOverlayMaterial(OverlayMaterial);
	}
}

// Called every frame
void AGoalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// アウトラインの表示変更処理
void AGoalActor::ChangeOutlineVisibility(bool flg)
{
	// 値に変更があった場合のみ変更
	if (GoalMesh && is_outline != flg)
	{
		GoalMesh->SetRenderCustomDepth(flg);
		is_outline = flg;
		UE_LOG(LogTemp, Log, TEXT("Outline : %s"), flg ? TEXT("true") : TEXT("false"));
	}
}