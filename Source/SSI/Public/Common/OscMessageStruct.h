#pragma once

#include "OscDataElemStruct.h"
#include "OscMessageStruct.generated.h"


USTRUCT(BlueprintType)
struct SSI_API FOscMessageStruct
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Address;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FOscDataElemStruct> Data;
};
