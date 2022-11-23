// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LiveLinkSourceFactory.h"
#include "ManusLiveLinkSource.h"
#include "ManusLiveLinkSourceFactory_Local.generated.h"

UCLASS()
class UManusLiveLinkSourceFactory_Local : public ULiveLinkSourceFactory
{
	GENERATED_BODY()

public:
	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 23
	virtual EMenuType GetMenuType() const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;
#else
	virtual TSharedPtr<SWidget> CreateSourceCreationPanel() override;
	virtual TSharedPtr<ILiveLinkSource> OnSourceCreationPanelClosed(bool bMakeSource) override;

	TSharedPtr<SLiveLinkManusSourceEditor> ActiveSourceEditor;
#endif
};