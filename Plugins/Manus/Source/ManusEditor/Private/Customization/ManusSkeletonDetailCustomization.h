// Copyright 2015-2020 Manus

#pragma once

#include "IDetailCustomization.h"

#include "CoreMinimal.h"

/**
* Customizes a UAnimGraphNode_ManusLiveLinkPose
*/
class FManusSkeletonDetailCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FManusSkeletonDetailCustomization>();
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
	// End IDetailCustomization interface

private:
	void ForceRefresh();
	FReply OnAutomaticSetupClicked();

private:
	IDetailLayoutBuilder* DetailBuilder = nullptr;
};
