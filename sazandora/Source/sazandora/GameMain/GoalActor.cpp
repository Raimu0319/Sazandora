// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalActor.h"

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
}

// Called every frame
void AGoalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}