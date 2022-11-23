// Copyright 2015-2020 Manus

#pragma once

#include "IDetailCustomization.h"

#include "CoreMinimal.h"

/**
* Customizes a UAnimGraphNode_ManusLiveLinkPose
*/
class FManusAnimGraphNodeDetailCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShared<FManusAnimGraphNodeDetailCustomization>();
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
	// End IDetailCustomization interface

private:
	void ForceRefresh();
	FReply OnEditManusLiveLinkUsersClicked();
	FReply OnEditManusSkeletonClicked();

	IDetailLayoutBuilder* DetailBuilder = nullptr;
};
