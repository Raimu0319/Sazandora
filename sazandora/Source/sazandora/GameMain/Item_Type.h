// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define D_MAX_ITEM_TYPE			(5)
#define D_MAX_BUY_LISTSIZE		(3)

	// アイテムタイプ
UENUM(BlueprintType)
enum class E_ITEM_TYPE : uint8
{
	E_NONE     UMETA(DisplayName = "None"),
	E_JUICE	   UMETA(DisplayName = "None"),
	E_HAMBRGER UMETA(DisplayName = "None"),
	E_DONUT	   UMETA(DisplayName = "None"),
	E_POPCORN  UMETA(DisplayName = "None")
};

/**
 * 
 */
class SAZANDORA_API Item_Type
{
public:
	Item_Type();
	~Item_Type();
};
