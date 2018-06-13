#include "SsiPrivatePCH.h"
#include "SsiSettings.h"

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
        FEditorDelegates::BeginPIE.AddRaw(this, &FSsiModule::OnBeginPIE);
        FEditorDelegates::EndPIE.AddRaw(this, &FSsiModule::OnEndPIE);
#endif
        
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
    }

    bool HandleSettingsSaved()
    {
        UE_LOG(LogSSI, Display, TEXT("Update settings"));

        auto settings = GetMutableDefault<USsiSettings>();

        // send settings
        settings->InitSendTargets();

        return true;
    }

private:
#if SSI_EDITOR_BUILD
    void OnBeginPIE(bool isSimulating)
    {
        auto settings = GetMutableDefault<USsiSettings>();
    }

	void OnEndPIE(bool isSimulating)
    {
    }
#endif

};


IMPLEMENT_MODULE(FSsiModule, SSI);
DEFINE_LOG_CATEGORY(LogSSI);


#undef LOCTEXT_NAMESPACE
