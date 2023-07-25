// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#include "ManusEditor.h"
#include "Manus.h"
#include "ManusSettings.h"
#include "ManusEditorUserSettings.h"
#include "ManusComponent.h"
#include "ManusSkeleton.h"
#include "ManusEditorStyle.h"
#include "AnimGraphNode_ManusLiveLinkPose.h"
#include "AssetTools/ManusSkeletonAssetActions.h"
#include "Customization/ManusSkeletonDetailCustomization.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsModule.h"
#include "AssetToolsModule.h"
#include "LevelEditor.h"

#include <SocketSubsystem.h>
#include <IPAddress.h>

#include "CoreSdk.h"

IMPLEMENT_MODULE(FManusEditorModule, ManusEditor)
DEFINE_LOG_CATEGORY(LogManusEditor);
#define LOCTEXT_NAMESPACE "FManusEditorModule"

void FManusEditorModule::StartupModule()
{
	UE_LOG(LogManusEditor, Log, TEXT("Started the Manus editor module"));

	FManusEditorStyle::Initialize();
    OnToolbarRefreshHostsButtonClicked();

	RegisterSettings();
	RegisterCustomizations();
	RegisterAssets();    
}

void FManusEditorModule::ShutdownModule()
{
	UE_LOG(LogManusEditor, Log, TEXT("Shut down the Manus editor module"));

	FManusEditorStyle::Shutdown();

	UnregisterSettings();
	UnregisterCustomizations();
	UnregisterAssets();
}

void FManusEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Manus",
			LOCTEXT("ManusSettingsName", "Manus"),
			LOCTEXT("ManusSettingsDescription", "Configure the Manus plugin."),
			GetMutableDefault<UManusSettings>()
		);
		SettingsModule->RegisterSettings("Project", "Plugins", "Manus Editor",
			LOCTEXT("ManusEditorUserSettingsName", "Manus Editor"),
			LOCTEXT("ManusEditorUserSettingsDescription", "Configure the Editor settings of the Manus plugin."),
			GetMutableDefault<UManusEditorUserSettings>()
		);
	}
}

void FManusEditorModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Manus");
		SettingsModule->UnregisterSettings("Project", "Plugins", "Manus Editor");
	}
}

void FManusEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(UManusSkeleton::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FManusSkeletonDetailCustomization::MakeInstance));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> ManusToolBarExtender = MakeShareable(new FExtender); 

		PluginCommands = MakeShareable(new FUICommandList);

#if ENGINE_MAJOR_VERSION == 5 
		ManusToolBarExtender->AddToolBarExtension("Play", EExtensionHook::After, nullptr,
#else
		ManusToolBarExtender->AddToolBarExtension("Compile", EExtensionHook::After, nullptr,
#endif

			FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
			{
                Builder.AddSeparator();

				// going to leave this bit due to the horrid unreal docs.
				// @param	InAction					UI action that sets the enabled state for this combo button
				// @param	InMenuContentGenerator		Delegate that generates a widget for this combo button's menu content.  Called when the menu is summoned.
				// @param	InLabelOverride				Optional label override.  If omitted, then the action's label will be used instead.
				// @param	InToolTipOverride			Optional tool tip override.	 If omitted, then the action's label will be used instead.
				// @param	InIconOverride				Optional icon to use for the tool bar image.  If omitted, then the action's icon will be used instead.
				// @param	bInSimpleComboBox			If true, the icon and label won't be displayed // this is critical for UE 5.0 because default minimal ui, argh.
				// @param	InTutorialHighlightName		Name to identify this widget and highlight during tutorials

				Builder.AddComboButton(
					FUIAction(),
					FOnGetContent::CreateRaw(this, &FManusEditorModule::FillToolbarComboButton, PluginCommands),
					LOCTEXT("ManusToolbarButtonHost", "Manus Host"),
					LOCTEXT("ManusToolbarButtonTooltip", "The current host to connect to."), 
					TAttribute<FSlateIcon>::Create(&GetToolbarHostsButtonIcon),
					false
				);
			}));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ManusToolBarExtender);

	}
}

void FManusEditorModule::UnregisterCustomizations()
{
	if (UObjectInitialized() && !IsEngineExitRequested())
	{
		FPropertyEditorModule* PropertyEditorModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		if (PropertyEditorModule)
		{
			PropertyEditorModule->UnregisterCustomClassLayout(UManusSkeleton::StaticClass()->GetFName());
		}

		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditor.GetToolBarExtensibilityManager().Reset();
		}
	}
}

void FManusEditorModule::RegisterAssets()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	ManusSkeletonAssetActions = new FManusSkeletonAssetActions;
	AssetTools.RegisterAssetTypeActions(MakeShareable(ManusSkeletonAssetActions));
}

void FManusEditorModule::UnregisterAssets()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::Get().GetModulePtr<FAssetToolsModule>("AssetTools");
    if (AssetToolsModule)
    {
        AssetToolsModule->Get().UnregisterAssetTypeActions(ManusSkeletonAssetActions->AsShared());
    }
}

FSlateIcon FManusEditorModule::GetToolbarHostsButtonIcon()
{
    if (FManusModule::Get().IsActive())
    {
        return FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.ManusOn");
    }
    else
    {
        return FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.ManusOff");
    }
}

void FManusEditorModule::SwitchHost(FString p_NewHost)
{
    if (FManusModule::Get().IsActive()) // if it was trying to connect. lets stop that 
    {
        if (m_CurrentHostSelected.Compare(p_NewHost) != 0)
        {
            // it was already connected to another host, so disconnect and then reconnect
            OnToggleConnectClicked();
            OnToolbarRefreshHostsButtonClicked();
            m_CurrentHostSelected = p_NewHost;
            OnToggleConnectClicked();
        }
    }
    else // we're not connected so just connect
    {
        m_CurrentHostSelected = p_NewHost;
        OnToggleConnectClicked();
    }
}

void FManusEditorModule::OnToggleConnectClicked()
{
    FManusModule::Get().SetActive(!FManusModule::Get().IsActive());
    if (!FManusModule::Get().SetManusCoreIP(m_CurrentHostSelected)) // just a simple quick connect to current selection
    {
        // ip may be set. but no connection was made
        if (FManusModule::Get().IsActive()) // if it was trying to connect. lets stop that 
        {
            FManusModule::Get().SetActive(false);
        }
    }
}

void FManusEditorModule::OnToolbarRefreshHostsButtonClicked()
{
    if ((CoreSdk::IsInitialized() == EManusRet::Success) &&
        (CoreSdk::CheckConnection() == EManusRet::Success)) return; // we are currently connected. do not let it interfere with current connection

    m_HostValues.Reset();
    m_CleanedHostValues.Reset();

    // Init Core SDK
    CoreSdk::Initialize();

    FManusModule::Get().GetRemoteHosts(m_HostValues);
  
    for (int i = 0; i < m_HostValues.Num(); i+=2) // the list always produces a multiple of 2 values.
    {
        FString t_Filtered = m_HostValues[i];
        t_Filtered.RemoveFromEnd(FString(".localdomain")); // cleanup
        t_Filtered += FString("  |  ")+ m_HostValues[i+1];
        m_CleanedHostValues.Add(t_Filtered);
    }

    m_LocalIndex = -1;
  
    FString t_HostName;
    if (ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetHostName(t_HostName))
    {
		// now check if this was found in the host list
		for (int i = 0; i < m_HostValues.Num(); i++)
		{
			FString t_Filtered = m_HostValues[i];
			t_Filtered.RemoveFromEnd(FString(".localdomain")); // cleanup
			if (t_Filtered.Compare(t_HostName) == 0)
			{
				m_LocalIndex = i;
				if (m_CurrentHostSelected.IsEmpty())
				{
					m_CurrentHostSelected = m_HostValues[i];
				}
			}
		}
    }
    if (m_HostValues.Num() > 0 && m_CurrentHostSelected.IsEmpty()) // no local hosts. so grab the first valid one.
    {
        m_CurrentHostSelected = m_HostValues[0];
    }
}

TSharedRef<SWidget> FManusEditorModule::FillToolbarComboButton(TSharedPtr<class FUICommandList> Commands)
{
    OnToolbarRefreshHostsButtonClicked();
    
    FMenuBuilder MenuBuilder(true, Commands);
    // current host if any
    if (CoreSdk::IsInitialized() == EManusRet::Success &&
        (CoreSdk::CheckConnection() == EManusRet::Success))
    {
        MenuBuilder.BeginSection("Current Host", LOCTEXT("CurrentHostMenuSectionName", "Current selected host"));
        {
            FSlateIcon t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.GreenDot");
            
            FString t_Filtered = m_CurrentHostSelected;
            t_Filtered.RemoveFromEnd(FString(".localdomain")); // cleanup

            MenuBuilder.AddMenuEntry(
                FText::FromString(t_Filtered),
                LOCTEXT("SetColorTooltip", "Current selected host."),
                t_Icon,
                FUIAction()
            );
        }
        MenuBuilder.EndSection();
    }

    // local host if any
    if (m_LocalIndex != -1) 
    {
        MenuBuilder.BeginSection("Local Host", LOCTEXT("LocalHostMenuSectionName", "Local host"));
        {

            FSlateIcon t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.dot");
            if ((CoreSdk::IsInitialized() == EManusRet::Success) &&
                (CoreSdk::CheckConnection() == EManusRet::Success) &&
                (m_CurrentHostSelected.Compare(m_HostValues[m_LocalIndex]) == 0))
            {
                t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.connected");
            }

            MenuBuilder.AddMenuEntry(
                FText::FromString(m_CleanedHostValues[m_LocalIndex/2]),
                LOCTEXT("SetColorTooltip", "Set the manus host to this address."),
                t_Icon,
                FUIAction(FExecuteAction::CreateLambda([=]
            {
                SwitchHost(m_HostValues[m_LocalIndex]);
             }))
            );
        }
        MenuBuilder.EndSection();
    }

    // remote host (usually multiple)
    if ((m_HostValues.Num() > 1) ||
        ((m_HostValues.Num() > 1) && m_LocalIndex != -1))
    {
        MenuBuilder.BeginSection("Remote Hosts", LOCTEXT("RemoteHostsMenuSectionName", "Remote hosts"));
        {
            for (int i = 0; i < m_HostValues.Num(); i+=2)
            {
                if (i == m_LocalIndex) continue;

                FSlateIcon t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.dot");
                if ((CoreSdk::IsInitialized() == EManusRet::Success) &&
                    (CoreSdk::CheckConnection() == EManusRet::Success) &&
                    (m_CurrentHostSelected.Compare(m_HostValues[i]) == 0))
                {
                    t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.connected");
                }

                MenuBuilder.AddMenuEntry(
                    FText::FromString(m_CleanedHostValues[i/2]),
                    LOCTEXT("SetColorTooltip", "Set the manus host to this address."),
                    t_Icon,
                    FUIAction(FExecuteAction::CreateLambda([=]
                {
                    SwitchHost(m_HostValues[i]);
                }))
                );
            }
        }
        MenuBuilder.EndSection();
    }

    // refresh option
    if ((CoreSdk::IsInitialized() != EManusRet::Success) ||
        (CoreSdk::CheckConnection() != EManusRet::Success))
    {
        MenuBuilder.BeginSection("Refresh Hosts", LOCTEXT("RefreshHostsMenuSectionName", "Refresh hosts"));
        {
            FSlateIcon t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.Refresh");
            MenuBuilder.AddMenuEntry(
                FText::FromString("Refresh host list"),
                LOCTEXT("SetColorTooltip", "Refresh host list."),
                t_Icon,
                FUIAction(FExecuteAction::CreateLambda([=]
            {
                OnToolbarRefreshHostsButtonClicked();
            }))
            );
        }
        MenuBuilder.EndSection();
    }

    // disconnect option (if we are connected)
    if (CoreSdk::IsInitialized() == EManusRet::Success &&
        (CoreSdk::CheckConnection() == EManusRet::Success))
    {
        MenuBuilder.BeginSection("Disconnect Host", LOCTEXT("DisconnectHostMenuSectionName", "Disconnect host"));
        {
            FSlateIcon t_Icon = FSlateIcon(FManusEditorStyle::GetStyleSetName(), "ToolbarIcon.disconnect");
            MenuBuilder.AddMenuEntry(
                FText::FromString("Disconnect from host"),
                LOCTEXT("SetColorTooltip", "Disconnect from host."),
                t_Icon,
                FUIAction(FExecuteAction::CreateLambda([=]
            {
                OnToggleConnectClicked();
            }))
            );
        }
        MenuBuilder.EndSection();
    }

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
