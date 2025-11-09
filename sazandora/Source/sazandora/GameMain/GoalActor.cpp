// Fill out your copyright notice in the Description page of Project Settings.


#include "GoalActor.h"

// Sets default values
AGoalActor::AGoalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	GoalBox = CreateDefaultSubobject<UGoalBoxComponent>(TEXT("GoalBox"));
	GoalBox->SetupAttachment(RootComponent);

	// サイズや可視化の設定
	GoalBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	GoalBox->SetHiddenInGame(true);

}

// Called when the game starts or when spawned
void AGoalActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

