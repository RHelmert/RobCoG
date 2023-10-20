// Copyright 2015-2020 Manus

#pragma once

#include <string>
#include "IDetailCustomization.h"

#include "CoreMinimal.h"
#include <Manus/Public/ManusBlueprintTypes.h>


typedef TSharedPtr<FString> FComboItemType;


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

    void OnHandMotionSelected(FComboItemType p_NewValue, ESelectInfo::Type);
    TSharedRef<SWidget> MakeWidgetForHandMotionOptions(FComboItemType InOption);
    FText GetCurrentHandMotionItemLabel() const;

	std::wstring GetFileNameFromDialog(bool p_Save, FString p_FileName);
	std::wstring FStringToWstring(FString p_String);
	UManusSkeleton* GetCurrentSkeleton() const;
	bool SetupSkeletonNodesChains(UManusSkeleton* p_Skeleton, uint32_t& p_SkeletonSetupIndex);

    FString HandMotionToString(EManusHandMotion p_EnumValue) const;
    EManusHandMotion HandMotionToEnum(FString p_MotionTypeString);
    EManusHandMotion GetFirstHandMotionType() const;
private:
	IDetailLayoutBuilder* DetailBuilder = nullptr;

    TArray<FComboItemType> m_HandMotionOptions;
};
