#pragma once
#include "Commandlets/Commandlet.h"
#include "WeaponOnlyMontageRepairCommandlet.generated.h"
UCLASS()
class UWeaponOnlyMontageRepairCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UWeaponOnlyMontageRepairCommandlet();
    virtual int32 Main(const FString& Params) override;
};
