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
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output);

    /// Add a floating point value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output);

    /// Add a integer value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output);

    /// Add a string value to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output);

    /// Add a blob to an OSC message.
    UFUNCTION(BlueprintPure, Category="SSI|Streams", meta=(AutoCreateRefTerm = "input"))
    static void StreamPushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output);

	UFUNCTION(BlueprintPure, Category="SSI|Events", meta=(AutoCreateRefTerm = "input"))
	static void EventPushFloat(const TArray<FOscDataElemStruct> & input, FName Key, float Value, TArray<FOscDataElemStruct> & output);

    /**
     *  @brief Send an SSI event.
     *  @param Address OSC address.
     *  @param Data result of successive PushFloat/Int/String/etc.
     *  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	 *
	 * sender_name and event_name are unused in the SSI SocketEventReader.
     */
    UFUNCTION(BlueprintCallable, Category="SSI|Events", meta=(AutoCreateRefTerm = "Data"))
    static void SendEvent(/*const FString sender_name, const FString event_name, */const TArray<FOscDataElemStruct> & Data, int32 TargetIndex, int32 timestamp = -1, int32 duration = 0, int32 state = 0);

	/**
	*  @brief Send an SSI event.
	*  @param Address OSC address.
	*  @param Data result of successive PushFloat/Int/String/etc.
	*  @param TargetIndex index of the destination, -1 for all destinations. (SendTarget list of the plugin settings)
	*
	* sender_name and event_name are unused in the SSI SocketEventReader.
	*/
	UFUNCTION(BlueprintCallable, Category="SSI|Events", meta = (AutoCreateRefTerm="Data"))
	static void SendMessage(/*const FString sender_name, const FString event_name, */const FString message, int32 TargetIndex, int32 timestamp = -1, int32 duration = 0);

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
    UFUNCTION(BlueprintCallable, Category="SSI")
    static int32 AddSendOscTarget(FString IpPort = "127.0.0.1:8000");
};
