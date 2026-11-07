#pragma once
#include "Commandlets/Commandlet.h"
#include "ActionTransitionGraphCommandlet.generated.h"

UCLASS()
class UActionTransitionGraphCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UActionTransitionGraphCommandlet();
    virtual int32 Main(const FString& Params) override;
};
