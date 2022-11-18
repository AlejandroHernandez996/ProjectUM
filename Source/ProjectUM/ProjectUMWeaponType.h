#pragma once

#include "CoreMinimal.h"
#include "ProjectUMWeaponType.generated.h"

UENUM(BlueprintType)
enum class EProjectUMWeaponType : uint8
{
	SWORD,
	BOW,
	STAFF,
	NONE
};