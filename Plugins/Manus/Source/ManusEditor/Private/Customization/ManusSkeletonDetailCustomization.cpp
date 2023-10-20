// Copyright 2015-2020 Manus

#include "ManusSkeletonDetailCustomization.h"

#include "ManusBlueprintTypes.h"
#include "ManusSkeleton.h"
#include "ManusTools.h"

#include "CoreSdk.h"

#include "Engine/SkeletalMesh.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SButton.h"

#include <Runtime/Core/Public/Misc/MessageDialog.h>
#include <Runtime/Slate/Public/Widgets/Input/SComboBox.h>

#include "ManusSkeleton.h"
#include <Manus/Public/ManusTypeInitializers.h>
#include <Manus/Public/ManusConvert.h>
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>

#include <fstream>
#include <iostream>
#include <locale>
#include <codecvt>

#include <filesystem>

#define LOCTEXT_NAMESPACE "ManusSkeletonDetailCustomization"

FString FManusSkeletonDetailCustomization::HandMotionToString(EManusHandMotion p_EnumValue) const
{                 
    static_assert(TIsEnumClass< EManusHandMotion >::Value, "'EManusHandMotion' template parameter to EnumToString must be a valid UEnum");
    return StaticEnum< EManusHandMotion >()->GetNameStringByIndex((int32)p_EnumValue);
}

EManusHandMotion FManusSkeletonDetailCustomization::HandMotionToEnum(FString p_MotionTypeString)
{
    if (p_MotionTypeString.Compare(HandMotionToString(EManusHandMotion::IMU), ESearchCase::IgnoreCase) == 0) return EManusHandMotion::IMU;
    if (p_MotionTypeString.Compare(HandMotionToString(EManusHandMotion::Tracker), ESearchCase::IgnoreCase) == 0) return EManusHandMotion::Tracker;
    if (p_MotionTypeString.Compare(HandMotionToString(EManusHandMotion::Tracker_RotationOnly), ESearchCase::IgnoreCase) == 0) return EManusHandMotion::Tracker_RotationOnly;
    if (p_MotionTypeString.Compare(HandMotionToString(EManusHandMotion::Auto), ESearchCase::IgnoreCase) == 0) return EManusHandMotion::Auto;
    return EManusHandMotion::None;
}

/// @brief to support the global hand motion type setting, we get the hand motion type of the first hand we encounter in the chains
/// @return the hand motion type
EManusHandMotion FManusSkeletonDetailCustomization::GetFirstHandMotionType() const
{
    UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

    if (t_Skeleton && t_Skeleton->ChainsIndexMap.Num()>0)
    {
        TArray<FName> t_Keys;
        t_Skeleton->ChainsIndexMap.GetKeys(t_Keys);

        for (int i = 0; i < t_Keys.Num(); i++)
        {
            if (t_Skeleton->ChainsIndexMap[t_Keys[i]].DataType == EManusChainType::Hand)
            {
                return t_Skeleton->ChainsIndexMap[t_Keys[i]].Settings.Hand.HandMotion;
            }
        }
    }
    return EManusHandMotion::None;
}

void FManusSkeletonDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

    m_HandMotionOptions.Add(MakeShareable(new FString(HandMotionToString(EManusHandMotion::None))));
    m_HandMotionOptions.Add(MakeShareable(new FString(HandMotionToString(EManusHandMotion::IMU))));
    m_HandMotionOptions.Add(MakeShareable(new FString(HandMotionToString(EManusHandMotion::Tracker))));
    m_HandMotionOptions.Add(MakeShareable(new FString(HandMotionToString(EManusHandMotion::Tracker_RotationOnly))));
    m_HandMotionOptions.Add(MakeShareable(new FString(HandMotionToString(EManusHandMotion::Auto))));
    
	// All categories
	TArray<FName> CategoryNames;
	InDetailBuilder.GetCategoryNames(CategoryNames);
	for (int i = 0; i < CategoryNames.Num(); i++)
	{
		if (i == 0)
		{
			// Hide top category
			InDetailBuilder.HideCategory(CategoryNames[i]);
		}
		else
		{
			IDetailCategoryBuilder& Category = InDetailBuilder.EditCategory(CategoryNames[i]);
			TArray<TSharedRef<IPropertyHandle>> Properties;
			Category.GetDefaultProperties(Properties);
			for (int PropertyIndex = 0; PropertyIndex < Properties.Num(); ++PropertyIndex)
			{
				TSharedPtr<IPropertyHandle> PropertyHandle = Properties[PropertyIndex];

				if (PropertyHandle.IsValid())
				{
					FProperty* Property = PropertyHandle->GetProperty();
					
					Category.AddProperty(PropertyHandle);
					
					if (Property->GetFName() == FName(TEXT("SkeletalMesh")))
					{
                        /* until further notice. this option is now hidden.
						Category.AddCustomRow(LOCTEXT("Setup", "Setup"))
						.ValueContent()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("AutomaticSetup", "Automatically setup Chains"))
								.OnClicked(this, &FManusSkeletonDetailCustomization::OnAutomaticSetupClicked)
							]
						];
                        */
						Category.AddCustomRow(LOCTEXT("Setup", "Setup"))
							.ValueContent()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("SendDevelopmentDashboard", "Send to Manus DevTool"))
							.OnClicked(this, &FManusSkeletonDetailCustomization::OnSendToDDClicked)
							]
							];


						Category.AddCustomRow(LOCTEXT("Setup", "Export to MSKL"))
							.ValueContent()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("ExportSkeleton", "Export to MSKL file"))
							.OnClicked(this, &FManusSkeletonDetailCustomization::OnExportClicked)
							]
							];

						Category.AddCustomRow(LOCTEXT("Setup", "Import from MSKL"))
							.ValueContent()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("ImportSkeleton", "Import from MSKL file"))
							.OnClicked(this, &FManusSkeletonDetailCustomization::OnImportClicked)
							]
							];
                        Category.AddCustomRow(LOCTEXT("Hand Motion Type", "Set overal hand motion type"))
                            .ValueContent()
                            [
                                SNew(SComboBox<FComboItemType>)
                                .OptionsSource(&m_HandMotionOptions)
                                .OnSelectionChanged(this, &FManusSkeletonDetailCustomization::OnHandMotionSelected)
                                .OnGenerateWidget(this, &FManusSkeletonDetailCustomization::MakeWidgetForHandMotionOptions)
                                .InitiallySelectedItem(m_HandMotionOptions[(int)GetFirstHandMotionType()])
                                [
                                    SNew(STextBlock)
                                    .Text(this, &FManusSkeletonDetailCustomization::GetCurrentHandMotionItemLabel)
                                ]
                            ];
					}
				}
			}
		}
	}
}

void FManusSkeletonDetailCustomization::ForceRefresh()
{
	if (DetailBuilder)
	{        
		DetailBuilder->ForceRefreshDetails();
	}
}

bool FManusSkeletonDetailCustomization::SetupSkeletonNodesChains(UManusSkeleton* p_Skeleton, uint32_t& p_SkeletonSetupIndex)
{
	// setup skeleton
	p_SkeletonSetupIndex = p_Skeleton->GetTemporarySkeletonIndex();
	if (p_SkeletonSetupIndex == UINT32_MAX)
	{
		EManusRet t_Ret = p_Skeleton->SetupSkeleton(p_SkeletonSetupIndex);
		if (t_Ret == EManusRet::ArgumentSizeMismatch)
		{
			FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Max amount of temporary skeletons reached. Would you like to clear the existing temporary skeletons to make room for new ones?");
			if (FMessageDialog::Open(EAppMsgType::OkCancel, DialogText) == EAppReturnType::Ok)
			{
				t_Ret = CoreSdk::ClearAllTemporarySkeletons();
				if (t_Ret != EManusRet::Success)
				{
					UE_LOG(LogManusEditor, Error, TEXT("Failed to Clear All Temporary Skeletons %s."));
					return false; // aborted
				}
				p_Skeleton->ClearAllTemporarySkeletonIndexes();

				t_Ret = p_Skeleton->SetupSkeleton(p_SkeletonSetupIndex);
				if (t_Ret != EManusRet::Success)
				{
					UE_LOG(LogManusEditor, Error, TEXT("Failed to Setup Skeleton. %s."), *p_Skeleton->GetName());
					return false; // aborted
				}
			}
		}
		else if(t_Ret != EManusRet::Success)
		{
			UE_LOG(LogManusEditor, Error, TEXT("Failed to setup temporary skeleton. %s."), *p_Skeleton->GetName());
			return false;
		}
	}
	else 
	{
		SkeletonSetupInfo t_SKL;
		ManusTypeInitializer::SkeletonSetupInfo_Init(&t_SKL);
		bool t_Res = p_Skeleton->ToSkeletonSetup(t_SKL);
		if (!t_Res)
		{
			return false;
		}
		EManusRet t_Ret = CoreSdk::OverwriteSkeletonSetup(p_SkeletonSetupIndex, t_SKL);
		if (t_Ret != EManusRet::Success)
		{
			UE_LOG(LogManusEditor, Error, TEXT("Failed to Overwrite Skeleton Setup. %s."), *p_Skeleton->GetName());
			return false;
		}
	}

	// setup skeleton with the nodes. 
	TArray<FName> t_OutKeys;
	p_Skeleton->NodesSetupMap.GetKeys(t_OutKeys);

	if (p_Skeleton->LoadExistingNodes(p_SkeletonSetupIndex) != true)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to Add Node To temporary skeleton. %s."), *p_Skeleton->GetName());
		return false; // aborted
	}

	if (p_Skeleton->ChainsIndexMap.Num() > 0)
	{
		// we don't clear old chains. so if any are present. we upload those too.
		p_Skeleton->LoadChains(p_SkeletonSetupIndex, false);
	}
	return true;
}

UManusSkeleton* FManusSkeletonDetailCustomization::GetCurrentSkeleton() const
{
	TArray<TWeakObjectPtr<UObject>> t_SelectedObjectsList;
	UManusSkeleton* t_Skeleton = NULL;
	DetailBuilder->GetObjectsBeingCustomized(t_SelectedObjectsList);
	if (t_SelectedObjectsList.Num() != 1) return NULL;

	return Cast<UManusSkeleton>(t_SelectedObjectsList[0].Get());
}

FReply FManusSkeletonDetailCustomization::OnReloadNodesClicked()
{
	UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

	if (!t_Skeleton)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to find temporary skeleton."));
		return FReply::Handled();
	}

	t_Skeleton->LoadNewNodes();
    
    ForceRefresh();	

	return FReply::Handled();
}

FReply FManusSkeletonDetailCustomization::OnExportClicked()
{
	if (CoreSdk::CheckConnection() != EManusRet::Success)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Manus Core is not connected.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		return FReply::Handled();
	}

	UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

	if (!t_Skeleton)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to find skeleton."));
		return FReply::Handled();
	}

	if (t_Skeleton->NodesSetupMap.Num() == 0)
	{
		UE_LOG(LogManusEditor, Error, TEXT("skeleton has no nodes. nothing to save."));
		return FReply::Handled();
	}

    if (t_Skeleton->TargetType == EManusSkeletonTargetType::Invalid)
    {
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Target Type set. Save cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
        return FReply::Handled(); // aborted
    }

    if (t_Skeleton->SkeletonType == EManusSkeletonType::Invalid)
    {
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Type set. Save cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
    }

	std::wstring t_Wide = GetFileNameFromDialog(true, t_Skeleton->TargetName);
	if (t_Wide.size() == 0)
	{
		// nothing selected. so abort.
		return FReply::Handled();
	}

	// setup skeleton
	uint32_t t_SkeletonSetupIndex = 0;
	if (!SetupSkeletonNodesChains(t_Skeleton, t_SkeletonSetupIndex))
	{
		return FReply::Handled();
	}

	uint32_t t_SessionId = 0;
	CoreSdk::GetSessionId(t_SessionId);

	// save the temporary skeleton 
	if (CoreSdk::SaveTemporarySkeleton(t_SkeletonSetupIndex, t_SessionId, false) != EManusRet::Success)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to save temporary skeleton. %s."), *t_Skeleton->GetName());
		return FReply::Handled(); // aborted
	}

	// now compress the temporary skeleton data and get the size of the compressed data:
	uint32_t t_TemporarySkeletonLengthInBytes;

	if (CoreSdk::CompressTemporarySkeletonAndGetSize(t_SkeletonSetupIndex, t_SessionId, t_TemporarySkeletonLengthInBytes) != EManusRet::Success)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to compress temporary skeleton and get size. %s."), *t_Skeleton->GetName());
		return FReply::Handled(); // aborted
	}
	unsigned char* t_TemporarySkeletonData = new unsigned char[t_TemporarySkeletonLengthInBytes];

	// get the array of bytes with the compressed temporary skeleton data, remember to always call function CoreSdk_CompressTemporarySkeletonAndGetSize
	// before trying to get the compressed temporary skeleton data
	if (CoreSdk::GetCompressedTemporarySkeletonData(t_TemporarySkeletonData, t_TemporarySkeletonLengthInBytes) != EManusRet::Success)
	{
        delete[] t_TemporarySkeletonData;
		UE_LOG(LogManusEditor, Error, TEXT("Failed to get compressed temporary skeleton data. %s."), *t_Skeleton->GetName());
		return FReply::Handled(); // aborted
	}

	std::ofstream t_File = std::ofstream(t_Wide, std::ofstream::binary);
	t_File.write((char*)t_TemporarySkeletonData, t_TemporarySkeletonLengthInBytes);
	t_File.close();
    delete[] t_TemporarySkeletonData;
	return FReply::Handled();
}

std::wstring FManusSkeletonDetailCustomization::FStringToWstring(FString p_String)
{
	std::string t_Narrow = std::string(TCHAR_TO_UTF8(*p_String));

	#pragma warning(push)
	#pragma warning(disable : 4996) // for this wstring conversion no suitable replacement has been built yet by the c++ committe. so this is a valid exclusion!
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> t_Converter;
	std::wstring t_Wide = t_Converter.from_bytes(t_Narrow);
	#pragma warning(pop)
	return t_Wide;
}

std::wstring FManusSkeletonDetailCustomization::GetFileNameFromDialog(bool p_Save, FString p_FileName )
{
	// start with file dialog
    FString t_DefaultPath = FPlatformProcess::UserDir(); 

    FString t_ExtendedPath = t_DefaultPath + FString(L"ManusTemporarySkeleton");
    if (!FPaths::DirectoryExists(t_ExtendedPath))
    {
        std::wstring t_Path = FStringToWstring(t_ExtendedPath);
        std::filesystem::create_directory(t_Path);
    }

	const FString t_FileTypes = FString("Manus Skeleton (*.mskl)|*.mskl");
	
	TArray<FString> t_OutFileNames;
	void* t_ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* t_DesktopPlatform = FDesktopPlatformModule::Get();
	if (t_DesktopPlatform)
	{
		uint32 t_SelectionFlag = 1; //A value of 0 represents single file selection while a value of 1 represents multiple file selection

		if (p_Save)
		{
			const FString t_FileName = p_FileName + FString(".mskl");
			const FString t_DialogTitle = FString("Export to MSKL");
			t_DesktopPlatform->SaveFileDialog(t_ParentWindowPtr, t_DialogTitle, t_ExtendedPath, FString(""), t_FileTypes, t_SelectionFlag, t_OutFileNames);
		}
		else
		{
			const FString t_DialogTitle = FString("Import from MSKL");
			t_DesktopPlatform->OpenFileDialog(t_ParentWindowPtr, t_DialogTitle, t_ExtendedPath, FString(""), t_FileTypes, t_SelectionFlag, t_OutFileNames);
		}
		
	}
	if (t_OutFileNames.Num() != 1 || t_OutFileNames[0].Len() == 0)
	{
		// nothing selected. so abort.
		return std::wstring(L"");
	}
	
	FString t_finalPath = t_OutFileNames[0];
	if (p_Save)
	{	
		// verify it has an extension. otherwise add it
		if (!t_finalPath.EndsWith(".mskl", ESearchCase::IgnoreCase))
		{
			t_finalPath.Append(".mskl");
		}
	}
	return FStringToWstring(t_finalPath);
}

TSharedRef<SWidget> FManusSkeletonDetailCustomization::MakeWidgetForHandMotionOptions(FComboItemType InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText FManusSkeletonDetailCustomization::GetCurrentHandMotionItemLabel() const
{
    return FText::FromString(HandMotionToString(GetFirstHandMotionType()));
}

void FManusSkeletonDetailCustomization::OnHandMotionSelected(FComboItemType p_NewValue, ESelectInfo::Type)
{
    // ok we got a new value, adjust 1 or 2 hands in the skeleton for their chain settings
    UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

    if (!t_Skeleton)
    {
        UE_LOG(LogManusEditor, Error, TEXT("Failed to find matching temporary skeleton."));
        return;
    }
    
    if ((t_Skeleton->NodesSetupMap.Num() != 0) ||
        (t_Skeleton->ChainsIndexMap.Num() != 0))
    {
        EManusHandMotion t_MotionType = HandMotionToEnum(*p_NewValue);
        // we got valid data

        TArray<FName> t_Keys;
        t_Skeleton->ChainsIndexMap.GetKeys(t_Keys);

        for (int i = 0; i < t_Keys.Num(); i++)
        {
            if (t_Skeleton->ChainsIndexMap[t_Keys[i]].DataType == EManusChainType::Hand)
            {
                t_Skeleton->ChainsIndexMap[t_Keys[i]].Settings.Hand.HandMotion = t_MotionType;
            }
        }

        if (CoreSdk::CheckConnection() == EManusRet::Success)
        {            
            DetailBuilder->ForceRefreshDetails(); // ah there is a connection, update everything related.
        }
    }
    return;
}

FReply FManusSkeletonDetailCustomization::OnImportClicked()
{
	if (CoreSdk::CheckConnection() != EManusRet::Success)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Manus Core is not connected.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		return FReply::Handled();
	}

	UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

	if (!t_Skeleton)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to find temporary skeleton."));
		return FReply::Handled();
	}

	if (t_Skeleton->GetSkeleton() == NULL)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no mesh assigned. Import cancelled.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return FReply::Handled(); // aborted
	}

	if (t_Skeleton->TargetType == EManusSkeletonTargetType::Invalid)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Target Type set. Import cancelled.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return FReply::Handled(); // aborted
	}

	if (t_Skeleton->SkeletonType == EManusSkeletonType::Invalid)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Type set. Import cancelled.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return FReply::Handled(); // aborted
	}

	if ((t_Skeleton->NodesSetupMap.Num() != 0) ||
		(t_Skeleton->ChainsIndexMap.Num() != 0))
	{
		// ask if we want to continue
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "All previous nodes and chains will be deleted when loading, proceed?");
		if (FMessageDialog::Open(EAppMsgType::OkCancel, DialogText) == EAppReturnType::Cancel)
		{
			return FReply::Handled(); // aborted
		}
		// don't clean chains and nodes juuuust yet.(in case of failure)
	}

	std::wstring t_Wide = GetFileNameFromDialog(false, t_Skeleton->TargetName);
	if (t_Wide.size() == 0)
	{
		// nothing selected. so abort.
		return FReply::Handled();
	}

	std::ifstream t_File = std::ifstream(t_Wide, std::ifstream::binary);

	if (!t_File)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to read from client file, the file does not exist in the mentioned directory. %s."), *t_Skeleton->GetName());
		return FReply::Handled();
	}

	// get file dimension
	t_File.seekg(0, t_File.end);
	int t_FileLength = (int)t_File.tellg();
	t_File.seekg(0, t_File.beg);

	// get temporary skeleton data from file
	unsigned char* t_TemporarySkeletonData = new unsigned char[t_FileLength];
	t_File.read((char*)t_TemporarySkeletonData, t_FileLength);
	t_File.close();

	// save the zipped temporary skeleton information, they will be used internally for sending the data to Core
	uint32_t t_TemporarySkeletonLengthInBytes = t_FileLength;

	if (t_TemporarySkeletonData == nullptr)
	{
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Failed to read the compressed temporary skeleton data from file. Import cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		UE_LOG(LogManusEditor, Error, TEXT("Failed to read the compressed temporary skeleton data from file. %s."), *t_Skeleton->GetName());
		delete[] t_TemporarySkeletonData;
		return FReply::Handled();
	}

	// ok we got the data
	// time to preemptively clear nodes and chains
    TMap<FName, FManusNodeSetup> t_NodesSetupMap;
    TMap< FName, FManusChainSetup> t_ChainsIndexMap;
    t_NodesSetupMap = t_Skeleton->NodesSetupMap;
    t_ChainsIndexMap = t_Skeleton->ChainsIndexMap;

	t_Skeleton->NodesSetupMap.Reset();
	t_Skeleton->ChainsIndexMap.Reset();

	// setup skeleton
	uint32_t t_SkeletonSetupIndex = t_Skeleton->GetTemporarySkeletonIndex();
	if (t_SkeletonSetupIndex == UINT32_MAX)
	{
		if (t_Skeleton->SetupSkeleton(t_SkeletonSetupIndex) != EManusRet::Success)
		{
            FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Failed to setup temporary skeleton. Import cancelled.");
            FMessageDialog::Open(EAppMsgType::Ok, DialogText);

			UE_LOG(LogManusEditor, Error, TEXT("Failed to setup temporary skeleton. %s."), *t_Skeleton->GetName());
			delete[] t_TemporarySkeletonData;

            t_Skeleton->NodesSetupMap = t_NodesSetupMap;
            t_Skeleton->ChainsIndexMap = t_ChainsIndexMap;

			return FReply::Handled();
		}
	}

	uint32_t t_SessionId = 0;
	CoreSdk::GetSessionId(t_SessionId);

	if(CoreSdk::GetTemporarySkeletonFromCompressedData(t_SkeletonSetupIndex, t_SessionId, t_TemporarySkeletonData, t_TemporarySkeletonLengthInBytes) != EManusRet::Success)
	{
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Failed to load temporary skeleton data from client file in Manus Core. Import cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		UE_LOG(LogManusEditor, Error, TEXT("Failed to load temporary skeleton data from client file in Core. %s."), *t_Skeleton->GetName());
		delete[] t_TemporarySkeletonData;
        t_Skeleton->NodesSetupMap = t_NodesSetupMap;
        t_Skeleton->ChainsIndexMap = t_ChainsIndexMap;
        CoreSdk::ClearTemporarySkeleton(t_SkeletonSetupIndex, t_SessionId);
        t_Skeleton->SetTemporarySkeletonIndex(UINT32_MAX);
		return FReply::Handled();
	}

	delete[] t_TemporarySkeletonData;

	if (!t_Skeleton->LoadSkeletonFromCore(t_SkeletonSetupIndex))
	{
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Failed to load temporary skeleton data from client file in Manus Core.Please check if this file is valid for this skeleton. Import cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		// log error. loading failed.
		UE_LOG(LogManusEditor, Error, TEXT("Failed to load temporary skeleton data from client file in Core. Please check if this file is valid for this skeleton %s."), *t_Skeleton->GetName());

        t_Skeleton->NodesSetupMap = t_NodesSetupMap;
        t_Skeleton->ChainsIndexMap = t_ChainsIndexMap; 
        CoreSdk::ClearTemporarySkeleton(t_SkeletonSetupIndex, t_SessionId);
        t_Skeleton->SetTemporarySkeletonIndex(UINT32_MAX);
		return FReply::Handled();
	}
	UE_LOG(LogManusEditor, Log , TEXT("succefully loaded temporary skeleton data from client file in Core. %s."), *t_Skeleton->GetName());

    ForceRefresh();

	return FReply::Handled();
}


FReply FManusSkeletonDetailCustomization::OnSendToDDClicked()
{
	if (CoreSdk::CheckConnection() != EManusRet::Success)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Manus Core is not connected.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		return FReply::Handled();
	}

	UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

	if (!t_Skeleton)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to find temporary skeleton."));
		return FReply::Handled();
	}

    if (t_Skeleton->TargetType == EManusSkeletonTargetType::Invalid)
    {
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Target Type set. Send cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
        return FReply::Handled(); // aborted
    }

    if (t_Skeleton->SkeletonType == EManusSkeletonType::Invalid)
    {
        FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Skeleton has no Type set. Send cancelled.");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return FReply::Handled(); // aborted
    }

	if (t_Skeleton->NodesSetupMap.Num() == 0)
	{
		FText DialogTitle = FText::FromString("ManusSkeletonDialogText");
		FText DialogText = FText::FromString("No nodes defined to allocate chains for " + t_Skeleton->GetName());
		FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
		return FReply::Handled(); // aborted
	}

	uint32_t t_SkeletonSetupIndex = 0;
	if (!SetupSkeletonNodesChains(t_Skeleton, t_SkeletonSetupIndex))
	{
		return FReply::Handled();
	}

	uint32_t t_SessionId = 0;
	CoreSdk::GetSessionId(t_SessionId);


    t_Skeleton->ToSkeletonMeshSetup(t_SkeletonSetupIndex); // TODO this is currently disabled. go to the function for more details. so no mesh visualization for the devtool right now.

	if(CoreSdk::SaveTemporarySkeleton(t_SkeletonSetupIndex, t_SessionId, false) != EManusRet::Success)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to save temporary skeleton. %s."), *t_Skeleton->GetName());
		return FReply::Handled(); // aborted
	}

	t_Skeleton->SetTemporarySkeletonIndex(t_SkeletonSetupIndex);

    ForceRefresh();

	return FReply::Handled();
}

FReply FManusSkeletonDetailCustomization::OnAutomaticSetupClicked()
{
	if (CoreSdk::CheckConnection() != EManusRet::Success)
	{
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "Manus Core is not connected.");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		return FReply::Handled();
	}

	UManusSkeleton* t_Skeleton = GetCurrentSkeleton();

	if (!t_Skeleton)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to find skeleton."));
		return FReply::Handled();
	}

	if (t_Skeleton->NodesSetupMap.Num() == 0)
	{
		FText DialogTitle = FText::FromString("ManusSkeletonDialogText");
		FText DialogText = FText::FromString("No nodes defined to allocate chains for " + t_Skeleton->GetName());
		FMessageDialog::Open(EAppMsgType::Ok, DialogText, &DialogTitle);
		return FReply::Handled(); // aborted
	}
	if (t_Skeleton->ChainsIndexMap.Num() > 0)
	{
		// ask if we want to continue
		FText DialogText = LOCTEXT("ManusSkeletonDialogText", "All previous chains will be deleted when auto allocating, proceed?");
		if (FMessageDialog::Open(EAppMsgType::OkCancel, DialogText) == EAppReturnType::Cancel)
		{
			return FReply::Handled(); // aborted
		}
		// clean chains
		t_Skeleton->ChainsIndexMap.Reset();
	}

	uint32_t t_SkeletonSetupIndex = 0;
	if (t_Skeleton->SetupSkeleton( t_SkeletonSetupIndex) != EManusRet::Success)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to setup Skeleton for automatic allocation. %s."), *t_Skeleton->GetName());
		return FReply::Handled();
	}

	// setup skeleton with the nodes.
	TArray<FName> t_OutKeys;
	t_Skeleton->NodesSetupMap.GetKeys(t_OutKeys);

	for (int i = 0; i < t_OutKeys.Num(); i++)
	{
		NodeSetup t_OutputNode;
		ManusTypeInitializer::NodeSetup_Init(&t_OutputNode);
		ManusConvert::BpManusNodeSetupToSDK(t_OutputNode, t_Skeleton->NodesSetupMap[t_OutKeys[i]]);
			
		if (CoreSdk::AddNodeToSkeletonSetup(t_SkeletonSetupIndex, t_OutputNode) != EManusRet::Success)
		{
			UE_LOG(LogManusEditor, Error, TEXT("Failed to Add Node To Skeleton Setup. %s."), *t_Skeleton->GetName());
			return FReply::Handled(); // aborted
		}
	}

	if (t_Skeleton->AllocateChains( t_SkeletonSetupIndex) != true)
	{
		UE_LOG(LogManusEditor, Error, TEXT("Failed to auto allocate chains for temporary skeleton. %s."), *t_Skeleton->GetName());
		return FReply::Handled(); // aborted
	}

    ForceRefresh();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
