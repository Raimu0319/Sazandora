// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC_Character.h"
#include "GameFramework/PlayerController.h"
#include "Main_Character.h"

// Sets default values
ANPC_Character::ANPC_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TalkRange = CreateDefaultSubobject<USphereComponent>(TEXT("Spheres"));
	TalkRange->SetupAttachment(RootComponent);
	TalkRange->SetSphereRadius(200.0f);
	TalkRange->SetCollisionProfileName(TEXT("Trigger"));

	// Overlapイベントをバインド
	TalkRange->OnComponentBeginOverlap.AddDynamic(this, &ANPC_Character::OnPlayerEnterRange);
	TalkRange->OnComponentEndOverlap.AddDynamic(this, &ANPC_Character::OnPlayerLeaveRange);

	// meshをセット
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshObj(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny"));
	if (MeshObj.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshObj.Object);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
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
void ANPC_Character::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActer,
	UPrimitiveComponent* OtherComo, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActer->IsA(AMain_Character::StaticClass()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Player is in"));
	}
}

void ANPC_Character::OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(AMain_Character::StaticClass()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Player is out"));
	}

}