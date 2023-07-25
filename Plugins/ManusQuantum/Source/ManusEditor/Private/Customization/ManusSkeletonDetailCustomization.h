// Copyright 2015-2020 Manus

#pragma once

#include <string>
#include "IDetailCustomization.h"

#include "CoreMinimal.h"
class UManusSkeleton;
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
	
	FReply OnSendToDDClicked();

	FReply OnReloadNodesClicked();

	FReply OnExportClicked();
	FReply OnImportClicked();

	std::wstring GetFileNameFromDialog(bool p_Save, FString p_FileName);
	std::wstring FStringToWstring(FString p_String);
	UManusSkeleton* GetCurrentSkeleton();
	bool SetupSkeletonNodesChains(UManusSkeleton* p_Skeleton, uint32_t& p_SkeletonSetupIndex);

private:
	IDetailLayoutBuilder* DetailBuilder = nullptr;
};
