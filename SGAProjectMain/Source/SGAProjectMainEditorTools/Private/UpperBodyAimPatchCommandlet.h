#pragma once
#include "Commandlets/Commandlet.h"
#include "UpperBodyAimPatchCommandlet.generated.h"
UCLASS()
class UUpperBodyAimPatchCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UUpperBodyAimPatchCommandlet();
    virtual int32 Main(const FString& Params) override;
};
