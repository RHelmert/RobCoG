// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#pragma once

// Set up a Doxygen group.
/** @addtogroup ManusSkeleton
 *  @{
 */

#include "CoreTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimTypes.h"
#include "CoreMinimal.h"
#include <string>

#include "ReferenceSkeleton.h"

#include "Engine/SkeletalMesh.h"

#if WITH_EDITOR
#include "Engine/Public/StaticMeshResources.h"
#include "Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Engine/Public/Rendering/SkeletalMeshLODModel.h"
#endif

#include "ManusSDKTypes.h"
#include "ManusBlueprintTypes.h"
#include "ManusSkeleton.generated.h"



/// @brief This class is the bridge between the %Manus Core animation data and the unreal animation
/// it defines the nodes (bones) and chains (how bones are connected) of a skeleton
/// and offers support functions to send them to %Manus Core and retrieve them.
UCLASS(BlueprintType, hidecategories = (Object))
class MANUS_API UManusSkeleton : public UObject
{
	GENERATED_BODY()

public:
	UManusSkeleton();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
	virtual void PostLoad() override;
#endif
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

public:
    
    /// @brief returns the USkeleton that is associated with this manus skeleton.
    class USkeleton* GetSkeleton();
    
    /// @brief gets the boneinfo of the SkeletalMesh->referenceskeleton (not the same as SkeletalMesh->skeleton->referenceskeleton)
    /// @return  
    const TArray<FMeshBoneInfo>& GetRawRefBoneInfo();
    /// @brief gets the transform of the SkeletalMesh->referenceskeleton (not the same as SkeletalMesh->skeleton->referenceskeleton)
    /// @return 
    const TArray<FTransform>& GetRefBonePose();

	/// @brief event triggered when the data has changed
	void OnManusSkeletonChanged();
    /// @brief  load existing nodes into SDK
	bool LoadExistingNodes(uint32_t  p_SkeletonSetupIndex);
    /// @brief load the unreal skeleton bones and generate new nodes 
	bool LoadNewNodes();
    /// @brief load the unreal skeleton bones and generate new nodes  and then immediately load them into SDK
	bool LoadNewNodes(uint32_t  p_SkeletonSetupIndex);
    /// @brief sends all the nodes to Core to auto allocate chains for them and receive the chains back
	bool AllocateChains(uint32_t  p_SkeletonSetupIndex);
    /// @brief load the chains into the skeleton ready for sending it to %Manus Core. (but not yet sent)
    void LoadChains(uint32_t  p_SkeletonSetupIndex, bool p_Overridetrackers = false);
    /// @brief load the skeleton with all its data into %Manus Core. (now it is being sent)
	bool LoadSkeleton(uint32_t  p_SkeletonSetupIndex, uint32_t& p_ID);
    /// @brief setup a skeleton for %Manus Core (not yet sending it)
	EManusRet SetupSkeleton(uint32_t& p_SkeletonSetupIndex);
    /// @brief Convert skeleton information to skeleton setup
    /// @param p_SkeletonSetupInfo 
    /// @return true when succesfully setup.
    bool ToSkeletonSetup(SkeletonSetupInfo& p_SkeletonSetupInfo);
    /// @brief clear all previously setup skeletons.
    void ClearAllTemporarySkeletonIndexes();
	/// @brief clear a specific skeleton
	/// @param p_SkeletonSetupIndex 
	/// @return true upon success.
	bool ClearSetupSkeleton(uint32_t p_SkeletonSetupIndex);
    /// @brief get the temporary skeleton from %Manus Core and load the chains and nodes.
	bool LoadSkeletonFromCore(uint32_t p_SkeletonSetupIndex);
    /// @brief get current temporary skeleton index
	uint32_t GetTemporarySkeletonIndex();

    /// @brief set the temporary skeleton index
    void SetTemporarySkeletonIndex(uint32_t p_Id);
    /// @brief retrieve temporary skeleton from %Manus Core.
	void RetrieveTemporarySkeleton();

    bool ToSkeletonMeshSetup(uint32 p_SkeletonSetupIndex);

#if WITH_EDITOR

    bool AddVertices(FSkelMeshSection* p_Section, FSkeletalMeshLODModel* p_ImportedModelLod, uint32 p_SkeletonSetupIndex, uint32 p_MeshSetupIndex);
    bool AddTriangles(FSkelMeshSection* p_Section, FSkeletalMeshLODModel* p_ImportedModelLod, uint32 p_SkeletonSetupIndex, uint32 p_MeshSetupIndex);
#endif

public:
    /// @brief The unreal skeletal mesh to use with Manus skeleton.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton")
	class USkeletalMesh* SkeletalMesh;

    /// @brief the skeleton type. this needs to be setup or else it cannot be loaded into %Manus Core
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton type")
    EManusSkeletonType SkeletonType = EManusSkeletonType::Invalid;
		
    /// @brief the skeleton pinch correction. 
    UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Use endpoint approximations"))
    bool UseEndPointApproximations = true;

    /// @brief Option to scale the skeleton to the user, turn off to keep the original sizes 
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Scale skeleton to user"))
	bool ScaleToUser = false;

    /// @brief skeleton name
	UPROPERTY(EditAnywhere, Category = "Manus | Full Body Tracking", meta = (DisplayName = "Target Skeleton Name"))
	FString TargetName = L"";
    /// @brief all the nodes of the skeleton (bones)
	UPROPERTY(EditAnywhere, Category = "Manus | map Skeleton Setup Nodes", meta = (DisplayName = "Skeleton Setup Nodes"))
	TMap<FName,FManusNodeSetup> NodesSetupMap;
    /// @brief all the chains of the skeleton
	UPROPERTY(EditAnywhere, Category = "Manus | map Skeleton Chains", meta = (DisplayName = "Skeleton Chain ID's"))
	TMap< FName, FManusChainSetup> ChainsIndexMap;
    /// @brief defines how this skeleton is targeted to data in %Manus Core.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton target type", meta = (DisplayName = "Target Skeleton type"))
	EManusSkeletonTargetType TargetType = EManusSkeletonTargetType::UserIndexData;
    /// @brief the user ID that the skeleton is assigned to. only 1 of the four targets is used.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton target type", meta = (DisplayName = "Target Skeleton user data"))
	FManusSkeletonTargetUserData TargetUserData;
    /// @brief the user index that the skeleton is assigned to.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton target type", meta = (DisplayName = "Target Skeleton user index data"))
	FManusSkeletonTargetUserIndexData TargetUserIndexData;
    /// @brief the animation name the skeleton is assigned to.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton target type", meta = (DisplayName = "Target Skeleton animation data"))
	FManusSkeletonTargetAnimationData TargetAnimationData;
    /// @brief the glove ID the skeleton is assigned to.
	UPROPERTY(EditAnywhere, Category = "Manus | Skeleton target type", meta = (DisplayName = "Target Skeleton glove data"))
	FManusSkeletonTargetGloveData GloveData;
    /// @brief the skeleton ID as generated by %Manus Core for this skeleton.
	UPROPERTY(VisibleAnywhere, Category = "Manus | Skeleton Id", meta = (DisplayName = "Target Skeleton id"))
	int ManusSkeletonId = 0; 

private:
    /// @brief part of the postload function to update values when the umanusskeleton is loaded/generated.
	void UpdateProperties();
    /// @brief returns a chain name based on its hierarchy. 
    FName GenerateChainName(FManusChainSetup& p_Chain);



    /// @brief temporary skeleton id for internal use only.
    uint32_t m_TemporarySkeletonIndex = UINT32_MAX;

};

// Close the Doxygen group.
/** @} */