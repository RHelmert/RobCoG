// Copyright 2015-2022 Manus

#include "ManusLiveLinkSourceFactory_Local.h"
#include "Manus.h"

#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"

#define LOCTEXT_NAMESPACE "FManusModule"

FText UManusLiveLinkSourceFactory_Local::GetSourceDisplayName() const
{
	return FText::FromString("Manus Source");
}

FText UManusLiveLinkSourceFactory_Local::GetSourceTooltip() const
{
	return FText::FromString("Manus Source");
}

UManusLiveLinkSourceFactory_Local::EMenuType UManusLiveLinkSourceFactory_Local::GetMenuType() const
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		if (!FManusModule::Get().IsLiveLinkSourceValid(EManusLiveLinkSourceType::Local) || !LiveLinkClient.HasSourceBeenAdded(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local)))
		{
			return EMenuType::MenuEntry;
		}
	}
	return EMenuType::Disabled;
}

TSharedPtr<ILiveLinkSource> UManusLiveLinkSourceFactory_Local::CreateSource(const FString& ConnectionString) const
{
	return FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local);
}


#undef LOCTEXT_NAMESPACE
