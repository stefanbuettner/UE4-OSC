#pragma once

#include "OscDataElemStruct.h"
#include "OscMessageStruct.h"
#include "SsiFunctionLibrary.generated.h"


UCLASS()
class SSI_API USsiFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /// Add a boolean value to an OSC message.
    UFUNCTION(BlueprintPure, Category=SSI, meta=(AutoCreateRefTerm = "input"))
    static void PushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output);

    /// Add a floating point value to an OSC message.
    UFUNCTION(BlueprintPure, Category=SSI, meta=(AutoCreateRefTerm = "input"))
    static void PushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output);

    /// Add a integer value to an OSC message.
    UFUNCTION(BlueprintPure, Category=SSI, meta=(AutoCreateRefTerm = "input"))
    static void PushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output);

    /// Add a string value to an OSC message.
    UFUNCTION(BlueprintPure, Category=SSI, meta=(AutoCreateRefTerm = "input"))
    static void PushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output);

    /// Add a blob to an OSC message.
    UFUNCTION(BlueprintPure, Category=SSI, meta=(AutoCreateRefTerm = "input"))
    static void PushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output);

    /**
     *  @brief Send an OSC message.
     *  @param Address OSC address.
     *  @param Data result of successive PushFloat/Int/String/etc.
     *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
     */
    UFUNCTION(BlueprintCallable, Category=SSI, meta=(AutoCreateRefTerm = "Data"))
    static void SendOsc(FName Address, const TArray<FOscDataElemStruct> & Data, int32 TargetIndex);

    /**
    *  @brief Send several OSC messages in an OSC bundle.
    *  @param Messages of the bundle.
    *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
    */
    UFUNCTION(BlueprintCallable, Category=SSI)
    static void SendOscBundle(const TArray<FOscMessageStruct> & Messages, int32 TargetIndex);

    /**
     *  @brief Add Ip:Port to the available OSC send targets.
     *  @param IpPort "ip:port". e.g. "192.168.0.1:7777"
     *  @return The created TargetIndex to pass to the SendOsc function.
     *
     *  Use this function to add target at runtime. Generally, it is best
     *  to define your targets in the OSC plugin settings.
     *
     *  @see SendOsc
     */
    UFUNCTION(BlueprintCallable, Category=SSI)
    static int32 AddSendOscTarget(FString IpPort = "127.0.0.1:8000");
};
