// Copyright 2015-2020 Manus

#include "ManusLiveLinkSourceFactory_Replicated.h"
#include "Manus.h"

#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"

#define LOCTEXT_NAMESPACE "FManusModule"

FText UManusLiveLinkSourceFactory_Replicated::GetSourceDisplayName() const
{
	return FText::FromString("Manus Replicated Source");
}

FText UManusLiveLinkSourceFactory_Replicated::GetSourceTooltip() const
{
	return FText::FromString("Manus Replicated Source");
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
UManusLiveLinkSourceFactory_Replicated::EMenuType UManusLiveLinkSourceFactory_Replicated::GetMenuType() const
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		if (!FManusModule::Get().IsLiveLinkSourceValid(EManusLiveLinkSourceType::Replicated) || !LiveLinkClient.HasSourceBeenAdded(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated)))
		{
			return EMenuType::MenuEntry;
		}
	}
	return EMenuType::Disabled;
}

TSharedPtr<ILiveLinkSource> UManusLiveLinkSourceFactory_Replicated::CreateSource(const FString& ConnectionString) const
{
	return FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated);
}
#else
TSharedPtr<SWidget> UManusLiveLinkSourceFactory_Replicated::CreateSourceCreationPanel()
{
	if (!ActiveSourceEditor.IsValid())
	{
		SAssignNew(ActiveSourceEditor, SLiveLinkManusSourceEditor);
	}
	return ActiveSourceEditor;
}

TSharedPtr<ILiveLinkSource> UManusLiveLinkSourceFactory_Replicated::OnSourceCreationPanelClosed(bool bCreateSource)
{
	TSharedPtr<ILiveLinkSource> NewSource = nullptr;

	if (bCreateSource && ActiveSourceEditor.IsValid())
	{
		NewSource = FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Replicated);
	}
	ActiveSourceEditor = nullptr;

	return NewSource;
}
#endif

#undef LOCTEXT_NAMESPACE
