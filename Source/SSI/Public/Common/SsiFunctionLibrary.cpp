#include "SsiPrivatePCH.h"
#include "SsiSettings.h"
#include "SsiFunctionLibrary.h"
#include "oscpack/osc/OscOutboundPacketStream.h"

void USsiFunctionLibrary::PushBool(const TArray<FOscDataElemStruct> & input, bool Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBool(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::PushFloat(const TArray<FOscDataElemStruct> & input, float Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetFloat(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::PushInt(const TArray<FOscDataElemStruct> & input, int32 Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetInt(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::PushString(const TArray<FOscDataElemStruct> & input, FName Value, TArray<FOscDataElemStruct> & output)
{
    if(Value.GetDisplayNameEntry()->IsWide())
    {
        const auto tmp = Value.GetPlainNameString();
        UE_LOG(LogSSI, Error, TEXT("Invalid string argument \"%s\": ASCII only"), *tmp);
        return;
    }

    output = input;
    FOscDataElemStruct elem;
    elem.SetString(Value);
    output.Add(elem);
}

void USsiFunctionLibrary::PushBlob(const TArray<FOscDataElemStruct> & input, const TArray<uint8> & Value, TArray<FOscDataElemStruct> & output)
{
    output = input;
    FOscDataElemStruct elem;
    elem.SetBlob(Value);
    output.Add(elem);
}

namespace
{
    bool isValidAddress(const FName & Address)
    {
        if(!Address.IsValid())
        {
            UE_LOG(LogSSI, Error, TEXT("Empty OSC address"));
            return false;
        }

        if(Address.GetDisplayNameEntry()->IsWide())
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogSSI, Error, TEXT("Invalid OSC address \"%s\": ASCII only"), *tmp);
            return false;
        }

        if(Address.GetPlainANSIString()[0] != '/')
        {
            const auto tmp = Address.GetPlainNameString();
            UE_LOG(LogSSI, Error, TEXT("Invalid OSC address \"%s\": must start with '/'"), *tmp);
            return false;
        }

        return true;
    }

    void appendMessage(osc::OutboundPacketStream & output, FName Address, const TArray<FOscDataElemStruct> & Data)
    {
        output << osc::BeginMessage(Address.GetPlainANSIString());
        if(output.State() != osc::SUCCESS)
        {
            return;
        }

        for(const auto & elem : Data)
        {
            if(elem.IsFloat())
            {
                output << (float)elem.AsFloatValue();
            }
            else if(elem.IsInt())
            {
                output << (int32)elem.AsIntValue();
            }
            else if(elem.IsBool())
            {
                output << elem.AsBoolValue();
            }
            else if(elem.IsString())
            {
                output << elem.AsStringValue().GetPlainANSIString();
            }
            else if(elem.IsBlob())
            {
                const TArray<uint8> & value = elem.AsBlobValue();
                output << osc::Blob(value.GetData(), value.Num());
            }

            if(output.State() != osc::SUCCESS)
            {
                return;
            }
        }
        output << osc::EndMessage;
    }

    TArray<uint8> GlobalBuffer(TArray<uint8>(), 1024);
}

void USsiFunctionLibrary::SendEvent(const TArray<FOscDataElemStruct> & Data, int32 TargetIndex)
{
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

    osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
    check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

	output << osc::BeginMessage("/evnt");
	output << "Sender ID";
	output << "Event ID";
	output << -1; // timestamp
	output << 0; // duration
	output << 0; // state
	output << 1; // n_events (string/float pairs)

	output << "Key";
	output << 3.14159f;

	output << osc::EndMessage;

	if (output.State() != osc::SUCCESS)
	{
		return;
	}

    if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
    {
        GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
        SendEvent(Data, TargetIndex);  // try again
        return;
    }

    if(output.State() == osc::SUCCESS)
    {
        GetMutableDefault<USsiSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
    }
    else
    {
        UE_LOG(LogSSI, Error, TEXT("OSC Send Message Error: %s"), osc::errorString(output.State()));
    }
}

void USsiFunctionLibrary::SendOscBundle(const TArray<FOscMessageStruct> & Messages, int32 TargetIndex)
{
    static_assert(sizeof(uint8) == sizeof(char), "Cannot cast uint8 to char");

    osc::OutboundPacketStream output((char *)GlobalBuffer.GetData(), GlobalBuffer.Max());
    check(reinterpret_cast<const void *>(GlobalBuffer.GetData()) == reinterpret_cast<const void *>(output.Data()));

    output << osc::BeginBundle();
    for(const auto & message : Messages)
    {
        if(!isValidAddress(message.Address))
        {
            return;
        }

        appendMessage(output, message.Address, message.Data);

        if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
        {
            GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
            SendOscBundle(Messages, TargetIndex);  // try again
            return;
        }
        if(output.State() != osc::SUCCESS)
        {
            UE_LOG(LogSSI, Error, TEXT("OSC Send Bundle Error: %s"), osc::errorString(output.State()));
            return;
        }
    }
    output << osc::EndBundle;

    if(output.State() == osc::OUT_OF_BUFFER_MEMORY_ERROR)
    {
        GlobalBuffer.Reserve(GlobalBuffer.Max() * 2);  // not enough memory: double the size
        SendOscBundle(Messages, TargetIndex);  // try again
        return;
    }

    if(output.State() == osc::SUCCESS)
    {
        GetMutableDefault<USsiSettings>()->Send(GlobalBuffer.GetData(), output.Size(), TargetIndex);
    }
    else
    {
        UE_LOG(LogSSI, Error, TEXT("OSC Send Bundle Error: %s"), osc::errorString(output.State()));
    }
}

int32 USsiFunctionLibrary::AddSendOscTarget(FString IpPort)
{
    return GetMutableDefault<USsiSettings>()->GetOrAddSendTarget(IpPort);
}
