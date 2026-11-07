#pragma once

#include "Commandlets/Commandlet.h"
#include "CharacterAnimationAuditCommandlet.generated.h"

/** Read-only by default. Explicit repair switches serialize only to Saved staging. */
UCLASS()
class UCharacterAnimationAuditCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UCharacterAnimationAuditCommandlet();
    virtual int32 Main(const FString& Params) override;
};
