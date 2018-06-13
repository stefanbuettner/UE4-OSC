#include "SsiPrivatePCH.h"
#include "SsiSettings.h"
#include "OscDispatcher.h"

#if SSI_EDITOR_BUILD
#include "Editor.h"
#endif


#define LOCTEXT_NAMESPACE "FOscModule"


class FSsiModule : public IModuleInterface
{
public:
    virtual void StartupModule( ) override
    {
        if(!FModuleManager::Get().LoadModule(TEXT("Networking")))
        {
            UE_LOG(LogSSI, Error, TEXT("Required module Networking failed to load"));
            return;
        }

#if SSI_EDITOR_BUILD
        //_mustListen = !GIsEditor;  // must not listen now if IsEditor (listen when PIE), else (Standalone Game) listen now
        FEditorDelegates::BeginPIE.AddRaw(this, &FSsiModule::OnBeginPIE);
        FEditorDelegates::EndPIE.AddRaw(this, &FSsiModule::OnEndPIE);
#endif

        _dispatcher = UOscDispatcher::Get();
        
        HandleSettingsSaved();

        // register settings
        auto settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
        if(settingsModule)
        {
            auto settingsSection = settingsModule->RegisterSettings("Project", "Plugins", "SSI",
                LOCTEXT("SsiSettingsName", "SSI"),
                LOCTEXT("SsiSettingsDescription", "Configure the SSI plug-in."),
                GetMutableDefault<USsiSettings>()
                );

            if (settingsSection.IsValid())
            {
                settingsSection->OnModified().BindRaw(this, &FSsiModule::HandleSettingsSaved);
            }
        }
        else
        {
#if SSI_EDITOR_BUILD
            UE_LOG(LogSSI, Warning, TEXT("Settings changed registration failed"));
#endif
        }
        UE_LOG(LogSSI, Display, TEXT("Startup succeed"));
    }

    virtual void ShutdownModule( ) override
    {
        UE_LOG(LogSSI, Display, TEXT("Shutdown"));

        if(_dispatcher.IsValid())
        {
            auto settings = GetMutableDefault<USsiSettings>();
            settings->ClearKeyInputs(*_dispatcher);

            _dispatcher->Stop();
        }
    }

    bool HandleSettingsSaved()
    {
        if(!_dispatcher.IsValid())
        {
            UE_LOG(LogSSI, Warning, TEXT("Cannot update settings"));
            return false;
        }
        
        UE_LOG(LogSSI, Display, TEXT("Update settings"));

        auto settings = GetMutableDefault<USsiSettings>();

        // receive settings
#if SSI_EDITOR_BUILD
        //if(_mustListen)
        //{
        //    Listen(settings);
        //}
#else
        Listen(settings);
#endif

        // send settings
        settings->InitSendTargets();

        // input settings
        settings->UpdateKeyInputs(*_dispatcher);

        return true;
    }

    //void Listen(USSISettings * settings)
    //{
    //    FIPv4Address receiveAddress(0);
    //    uint32_t receivePort;
    //    if(USSISettings::Parse(settings->ReceiveFrom, &receiveAddress, &receivePort, USSISettings::ParseOption::OptionalAddress))
    //    {
    //        _dispatcher->Listen(receiveAddress, receivePort, settings->MulticastLoopback);
    //    }
    //    else
    //    {
    //        UE_LOG(LogOSC, Error, TEXT("Fail to parse receive address: %s"), *settings->ReceiveFrom);
    //    }
    //}

private:
#if SSI_EDITOR_BUILD
    void OnBeginPIE(bool isSimulating)
    {
        //_mustListen = true;

        check(_dispatcher.IsValid())
        auto settings = GetMutableDefault<USsiSettings>();
        //Listen(settings);
    }

	void OnEndPIE(bool isSimulating)
    {
        //_mustListen = false;
        _dispatcher->Stop();
    }

    //bool _mustListen;
#endif

private:
    
    TWeakObjectPtr<UOscDispatcher> _dispatcher;
};


IMPLEMENT_MODULE(FSsiModule, SSI);
DEFINE_LOG_CATEGORY(LogSSI);


#undef LOCTEXT_NAMESPACE
