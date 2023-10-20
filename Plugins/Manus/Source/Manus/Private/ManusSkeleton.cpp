// Copyright Epic Games, Inc. All Rights Reserved.
// Copyright 2015-2022 Manus

#include "ManusSkeleton.h"
#include "CoreSdk.h"
#include "Manus.h"
#include "ManusTools.h"
#include "ManusConvert.h"
#include "ManusSettings.h"
#include "ManusComponent.h"
#include <Manus/Public/ManusTypeInitializers.h>
#include <regex>

#include "Runtime/Engine/Classes/Animation/Skeleton.h"

UManusSkeleton::UManusSkeleton()
{
	SkeletalMesh = NULL;

	ScaleToUser = false;

	TargetName = L"";
	NodesSetupMap.Empty();

	ChainsIndexMap.Empty();
	TargetType = EManusSkeletonTargetType::UserIndexData;

	TargetUserData.userID = 0;
	TargetUserIndexData.userIndex = 0;
	TargetAnimationData.id = FString("");
	GloveData.gloveID = 0;
}

USkeleton* UManusSkeleton::GetSkeleton()
{
	if (SkeletalMesh)
	{
        return SkeletalMesh->GetSkeleton();
    }
	return NULL;
}

const TArray<FMeshBoneInfo>& UManusSkeleton::GetRawRefBoneInfo()
{
    return SkeletalMesh->GetRefSkeleton().GetRawRefBoneInfo();
}

const TArray<FTransform>& UManusSkeleton::GetRefBonePose()
{
    return SkeletalMesh->GetRefSkeleton().GetRefBonePose();
}

void UManusSkeleton::OnManusSkeletonChanged()
{
	TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
	if (ManusLocalLiveLinkSource.IsValid())
	{
		TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
		ManusLocalLiveLinkSource->InitSkeletons(true, t_ManusLiveLinkUsers);
	}
}

void UManusSkeleton::PostLoad()
{
	Super::PostLoad();
	// update property list for UI ?
	UpdateProperties();

}
void UManusSkeleton::UpdateProperties()
{
	if (SkeletalMesh)
	{
		if (TargetName.IsEmpty())
		{
			TargetName = GetSkeleton()->GetFName().ToString();
		}

		// build up the nodes.
		if (NodesSetupMap.Num() == 0)
		{
            const TArray<FMeshBoneInfo>& t_BoneInfo = GetRawRefBoneInfo();
            const TArray<FTransform>& t_BonePosesByIndex = GetRefBonePose();
			for (int i = 0; i < (int)t_BoneInfo.Num(); i++)
			{
				uint32_t t_Parent = (uint32_t)t_BoneInfo[i].ParentIndex; // due to conversion we need to make this point to itself
				if (t_BoneInfo[i].ParentIndex == -1) t_Parent = i;

				NodeSetup t_OutputNode = ManusTools::CreateNodeSetup(i,
														 			 t_Parent,
														 			 t_BonePosesByIndex[i],
																	 t_BoneInfo[i].Name);

				FManusNodeSetup t_StoredNode;
				ManusConvert::SDKManusNodeSetupToBP(t_StoredNode, t_OutputNode);
				NodesSetupMap.Add(t_BoneInfo[i].Name, t_StoredNode);
			}
		}
		// chains are done based upon the nodes. but either they are already pre-saved. or we need to generate them
		// if we need to generate them, we need a connection to the core. which we do not always have.
		// so there will need to be checks etc for this 
	}
}

// todo still used? think so but needs more verification.
#if WITH_EDITOR
void UManusSkeleton::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	
	FProperty* TailProperty = PropertyChangedEvent.PropertyChain.GetTail()->GetValue();
	FName TailPropertyName = TailProperty->GetFName();

	if (TailPropertyName == FName("SkeletalMesh"))
	{
		LoadNewNodes();

		// Update the skeleton in every Manus component using it
		TSharedPtr<FManusLiveLinkSource> ManusLocalLiveLinkSource = StaticCastSharedPtr<FManusLiveLinkSource>(FManusModule::Get().GetLiveLinkSource(EManusLiveLinkSourceType::Local));
		if (ManusLocalLiveLinkSource.IsValid())
		{
			const TArray<FManusLiveLinkUser>& ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
			for (int i = 0; i < ManusLiveLinkUsers.Num(); i++)
			{
				if (FManusModule::Get().ManusLiveLinkUsers.IsValidIndex(i))
				{
					FManusLiveLinkUser& ManusLiveLinkUser = FManusModule::Get().ManusLiveLinkUsers[i];
					for (int j = 0; j < ManusLiveLinkUser.ObjectsUsingUser.Num(); j++)
					{
						if (ManusLiveLinkUser.ObjectsUsingUser[j].IsValid())
						{
							UManusComponent* ManusComponent = Cast<UManusComponent>(ManusLiveLinkUser.ObjectsUsingUser[j].Get());
							if (ManusComponent)
							{
								ManusComponent->RefreshManusSkeleton();
							}
						}
					}
				}
			}
		}
	}

	OnManusSkeletonChanged();	
}
#endif //WITH_EDITOR

bool UManusSkeleton::LoadExistingNodes(uint32_t  p_SkeletonSetupIndex)
{
	TArray<FName> t_OutKeys;
	NodesSetupMap.GetKeys(t_OutKeys);

	for (int i = 0; i < t_OutKeys.Num(); i++)
	{
		NodeSetup t_OutputNode;
		ManusTypeInitializer::NodeSetup_Init(&t_OutputNode);
		ManusConvert::BpManusNodeSetupToSDK(t_OutputNode, NodesSetupMap[t_OutKeys[i]]);
		
		if (CoreSdk::AddNodeToSkeletonSetup(p_SkeletonSetupIndex, t_OutputNode) != EManusRet::Success)
		{
			return false;
		}
	}
	return true;
}

bool UManusSkeleton::LoadNewNodes()
{
	EManusRet t_Ret = EManusRet::Success;
	if (GetSkeleton() == NULL) return false;

	const TArray<FMeshBoneInfo>& t_BoneInfo = GetRawRefBoneInfo();
	const TArray<FTransform>& t_BonePosesByIndex = GetRefBonePose();

	NodesSetupMap.Reset();

	for (int i = 0; i < (int)t_BoneInfo.Num(); i++)
	{
		uint32_t t_Parent = (uint32_t)t_BoneInfo[i].ParentIndex; // due to conversion we need to make this point to itself
		if (t_BoneInfo[i].ParentIndex == -1) t_Parent = i;
		// use the boneinfo to obtain hierarchy info			

		NodeSetup t_OutputNode = ManusTools::CreateNodeSetup(i,
			t_Parent,
			t_BonePosesByIndex[i],
			t_BoneInfo[i].Name);

		FManusNodeSetup t_StoredNode;
		ManusConvert::SDKManusNodeSetupToBP(t_StoredNode, t_OutputNode);
		NodesSetupMap.Add(t_BoneInfo[i].Name, t_StoredNode);
	}
	return true;
}

bool UManusSkeleton::LoadNewNodes(uint32_t  p_SkeletonSetupIndex)
{
	EManusRet t_Ret = EManusRet::Success;
	const TArray<FMeshBoneInfo>& t_BoneInfo = GetRawRefBoneInfo();
	const TArray<FTransform>& t_BonePosesByIndex = GetRefBonePose();

	for (int i = 0; i < (int)t_BoneInfo.Num(); i++)
	{
		uint32_t t_Parent = (uint32_t)t_BoneInfo[i].ParentIndex; // due to conversion we need to make this point to itself
		if (t_BoneInfo[i].ParentIndex == -1) t_Parent = i;
		// use the boneinfo to obtain hierarchy info			

		NodeSetup t_OutputNode = ManusTools::CreateNodeSetup(i,
			t_Parent,
			t_BonePosesByIndex[i],
			t_BoneInfo[i].Name);

		FManusNodeSetup t_StoredNode;
		ManusConvert::SDKManusNodeSetupToBP(t_StoredNode, t_OutputNode);
		NodesSetupMap.Add(t_BoneInfo[i].Name, t_StoredNode);

		t_Ret = CoreSdk::AddNodeToSkeletonSetup(p_SkeletonSetupIndex, t_OutputNode);

		if (t_Ret != EManusRet::Success)
		{
			return false;
		}
	}
	return true;
}

bool UManusSkeleton::LoadSkeletonFromCore(uint32_t p_SkeletonSetupIndex)
{
	// get the skeleton info
    SkeletonSetupInfo t_SkeletonSetup;
    ManusTypeInitializer::SkeletonSetupInfo_Init(&t_SkeletonSetup);
    if (CoreSdk::GetSkeletonSetupInfo(p_SkeletonSetupIndex, &t_SkeletonSetup) != EManusRet::Success)
    {
        UE_LOG(LogManus, Error, TEXT("Failed to get info about skeleton setup. %s."), *GetSkeleton()->GetName());
        return false;
    }

	SkeletonSetupArraySizes t_SkeletonInfo;

	if (CoreSdk::GetSkeletonSetupArraySizes(p_SkeletonSetupIndex, t_SkeletonInfo) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get info about skeleton. %s."), *GetSkeleton()->GetName());
		return false;
	}

	ChainSetup* t_Chains = new ChainSetup[t_SkeletonInfo.chainsCount];
	for (uint32_t i = 0; i < t_SkeletonInfo.chainsCount; i++)
	{
		ManusTypeInitializer::ChainSetup_Init(&t_Chains[i]);
	}

	// now get the chain data
	if (CoreSdk::GetSkeletonSetupChains(p_SkeletonSetupIndex, t_Chains) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get chains about skeleton. %s."), *GetSkeleton()->GetName());
		delete[] t_Chains;
		return false;
	}

	// and nodes
	NodeSetup* t_Nodes = new NodeSetup[t_SkeletonInfo.nodesCount];
	for (uint32_t i = 0; i < t_SkeletonInfo.nodesCount; i++)
	{
		ManusTypeInitializer::NodeSetup_Init(&t_Nodes[i]);
	}

	if (CoreSdk::GetSkeletonSetupNodes(p_SkeletonSetupIndex, t_Nodes) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get nodes about skeleton. %s."), *GetSkeleton()->GetName());
		delete[] t_Chains;
		delete[] t_Nodes;
		return false;
	}

	const TArray<FMeshBoneInfo>& t_BoneInfo = GetRawRefBoneInfo();
	
    // first check if it is even valid

    if (t_SkeletonInfo.nodesCount != t_BoneInfo.Num())
    {
        UE_LOG(LogManus, Error, TEXT("Failed to get valid nodes about skeleton. %s."), *GetSkeleton()->GetName());
        delete[] t_Chains;
        delete[] t_Nodes;
        return false;
    }
    for (uint32_t i = 0; i < t_SkeletonInfo.nodesCount; i++)
    {
        if (t_Nodes[i].id >= (uint32_t)(t_BoneInfo.Num()))
        {
            UE_LOG(LogManus, Error, TEXT("Failed to get valid nodes about skeleton. %s."), *GetSkeleton()->GetName());
            delete[] t_Chains;
            delete[] t_Nodes;
            return false;
        }
    }


	NodesSetupMap.Reset();

	for (uint32_t i = 0; i < t_SkeletonInfo.nodesCount; i++)
	{
		FManusNodeSetup t_Node;
		ManusConvert::SDKManusNodeSetupToBP(t_Node, t_Nodes[i]);
		NodesSetupMap.Add(t_BoneInfo[t_Node.Id].Name, t_Node);
	}

	ChainsIndexMap.Reset();

	for (uint32_t i = 0; i < t_SkeletonInfo.chainsCount; i++)
	{
		FManusChainSetup t_Chain;
		ManusConvert::SDKManusChainSetupToBp(t_Chain, t_Chains[i]);
        
        ChainsIndexMap.Add(GenerateChainName(t_Chain), t_Chain);
	}

    if (t_SkeletonSetup.type == SkeletonType::SkeletonType_Body) SkeletonType = EManusSkeletonType::Body;
    else if (t_SkeletonSetup.type == SkeletonType::SkeletonType_Both) SkeletonType = EManusSkeletonType::Both;
    else if (t_SkeletonSetup.type == SkeletonType::SkeletonType_Hand) SkeletonType = EManusSkeletonType::Hand;
    else SkeletonType = EManusSkeletonType::Body;
    ScaleToUser = t_SkeletonSetup.settings.scaleToTarget;
    UseEndPointApproximations = t_SkeletonSetup.settings.useEndPointApproximations;
    TargetName = FString(UTF8_TO_TCHAR(t_SkeletonSetup.name));
    // other settings are currently ignored.    

	delete[] t_Chains;
	delete[] t_Nodes;

	return true;
}

FName UManusSkeleton::GenerateChainName(FManusChainSetup& p_Chain)
{
    // generate name based on number, type and side. 
    FString t_ChainName = FString::FromInt(p_Chain.Id) + L": ";

    if (p_Chain.Side == EManusSide::Center) t_ChainName += L"Center - ";
    else if (p_Chain.Side == EManusSide::Left) t_ChainName += L"Left - ";
    else if (p_Chain.Side == EManusSide::Right) t_ChainName += L"Right - ";
    else t_ChainName += L"Side_Invalid - ";

    if (p_Chain.Type == EManusChainType::Arm) t_ChainName += L"Arm";
    else if (p_Chain.Type == EManusChainType::FingerIndex) t_ChainName += L"FingerIndex";
    else if (p_Chain.Type == EManusChainType::FingerMiddle) t_ChainName += L"FingerMiddle";
    else if (p_Chain.Type == EManusChainType::FingerPinky) t_ChainName += L"FingerPinky";
    else if (p_Chain.Type == EManusChainType::FingerRing) t_ChainName += L"FingerRing";
    else if (p_Chain.Type == EManusChainType::FingerThumb) t_ChainName += L"FingerThumb";
    else if (p_Chain.Type == EManusChainType::Foot) t_ChainName += L"Foot";
    else if (p_Chain.Type == EManusChainType::Hand) t_ChainName += L"Hand";
    else if (p_Chain.Type == EManusChainType::Head) t_ChainName += L"Head";
    else if (p_Chain.Type == EManusChainType::Leg) t_ChainName += L"Leg";
    else if (p_Chain.Type == EManusChainType::Neck) t_ChainName += L"Neck";
    else if (p_Chain.Type == EManusChainType::Pelvis) t_ChainName += L"Pelvis";
    else if (p_Chain.Type == EManusChainType::Shoulder) t_ChainName += L"Shoulder";
    else if (p_Chain.Type == EManusChainType::Spine) t_ChainName += L"Spine";
    else if (p_Chain.Type == EManusChainType::Toe) t_ChainName += L"Toe";
    else t_ChainName += L"Invalid_ChainType";
    
    return FName(t_ChainName);
}

// if no chains are there yet. for the manus assets we can auto allocate them.
// for other assets use the Developer dashboard to generate it.
bool UManusSkeleton::AllocateChains( uint32_t  p_SkeletonSetupIndex)
{
	// allocate chains for skeleton 
	
	if (CoreSdk::AllocateChainsForSkeletonSetup(p_SkeletonSetupIndex) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to Add Node To Skeleton Setup. %s."), *GetSkeleton()->GetName());
		return false;
	}

	// get the skeleton info
	SkeletonSetupArraySizes t_SkeletonInfo;
	
	if (CoreSdk::GetSkeletonSetupArraySizes(p_SkeletonSetupIndex, t_SkeletonInfo) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get info about skeleton. %s."), *GetSkeleton()->GetName());
		return false;
	}

	ChainSetup* t_Chains = new ChainSetup[t_SkeletonInfo.chainsCount];
	for (uint32_t i = 0; i < t_SkeletonInfo.chainsCount; i++)
	{
		ManusTypeInitializer::ChainSetup_Init(&t_Chains[i]);
	}

	// now get the chain data
	if (CoreSdk::GetSkeletonSetupChains(p_SkeletonSetupIndex, t_Chains) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get chains about skeleton. %s."), *GetSkeleton()->GetName());
		delete[] t_Chains;
		return false;
	}

	// and nodes
	NodeSetup* t_Nodes = new NodeSetup[t_SkeletonInfo.nodesCount];
	
	if (CoreSdk::GetSkeletonSetupNodes(p_SkeletonSetupIndex, t_Nodes) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to get nodes about skeleton. %s."), *GetSkeleton()->GetName());
		delete[] t_Chains;
		delete[] t_Nodes;
		return false;
	}

	// but since we want to cleanly load the skeleton without holding everything up
	// we need to set its side first
	for (uint32_t i = 0; i < t_SkeletonInfo.chainsCount; i++)
	{
		// store the chains into the skeleton too!
		FManusChainSetup t_Chain;
		ManusConvert::SDKManusChainSetupToBp(t_Chain, t_Chains[i]);
		ChainsIndexMap.Add(GenerateChainName(t_Chain), t_Chain);
	}
	// cleanup
	delete[] t_Chains;
	delete[] t_Nodes;

	return true;
}

/// @brief load the chains into the skeleton ready for sending it to manus core. (but not yet sent)
void UManusSkeleton::LoadChains(uint32_t  p_SkeletonSetupIndex, bool p_Overridetrackers)
{
	TArray<FName> t_Keys;
	ChainsIndexMap.GetKeys(t_Keys);

	for (int i = 0; i < t_Keys.Num(); i++)
	{
		ChainSetup t_Chain;
		ManusTypeInitializer::ChainSetup_Init(&t_Chain);
		ManusConvert::BpManusChainSetupToSDK(t_Chain, ChainsIndexMap[t_Keys[i]], p_Overridetrackers);
		CoreSdk::AddChainToSkeletonSetup(p_SkeletonSetupIndex, t_Chain);
	}
}

/// @brief load the skeleton with all its data into manus core. (now it is being sent)
bool UManusSkeleton::LoadSkeleton(uint32_t  p_SkeletonSetupIndex, uint32_t &p_ID)
{
	if (ChainsIndexMap.Num() == 0) return false;

	// and load the skeleton.
	p_ID = 0;
	return (CoreSdk::LoadSkeleton(p_SkeletonSetupIndex, p_ID) == EManusRet::Success);
}

/// @brief clear the previously setup skeleton.
/// this may actually become obsolete. as skeletons are cleared on disconnect anyway and you can overwrite an existing skeleton for ease of use (even advisable to do so)
bool UManusSkeleton::ClearSetupSkeleton(uint32_t p_SkeletonSetupIndex)
{
	uint32_t t_SessionId = 0;
	if (CoreSdk::GetSessionId(t_SessionId) != EManusRet::Success)
	{
		return false;
	}
	EManusRet t_Ret = CoreSdk::ClearTemporarySkeleton(p_SkeletonSetupIndex, t_SessionId);
	if (t_Ret != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to clear Skeleton Setup. %s."), *GetSkeleton()->GetName());
		return false; // aborted
	}
	return true;
}

/// @brief Convert skeleton mesh information to skeleton setup
/// TODO TODO this is currently not 100% ok TODO TODO
bool UManusSkeleton::ToSkeletonMeshSetup(uint32 p_SkeletonSetupIndex)
{
#if WITH_EDITOR

    USkeleton* t_Skeleton = SkeletalMesh->GetSkeleton();
    
    if (t_Skeleton)
    {
        USkeletalMesh* t_Mesh = t_Skeleton->FindCompatibleMesh();// there are other ways like  t_Mesh->GetPreviewMesh(); but they result in no meshes in certain cases, so not usable.

        if (t_Mesh)
        {
            FSkeletalMeshModel* t_ImportedModel = t_Mesh->GetImportedModel();
            if (t_ImportedModel && t_ImportedModel->LODModels.Num()>0)
            {
                FSkeletalMeshLODModel& t_ImportedModelLod0 = t_ImportedModel->LODModels[0];
                
                // loop through sections
                for (int i = 0; i < t_ImportedModelLod0.Sections.Num(); i++) // models do exist in multiple sections (especially more complex models) so these are needed.
                {
                    uint32 t_MeshSetupIndex;
                    FSkelMeshSection& t_Section = t_ImportedModelLod0.Sections[i];
                    EManusRet t_Result = CoreSdk::AddMeshSetupToSkeletonSetup(p_SkeletonSetupIndex,
                                                         0, // TODO finding the nodeid that is correct is so far unsuccessful. no indicator which is the correct one has been found.
                                                         &t_MeshSetupIndex);

                    if (t_Result != EManusRet::Success)
                    {
                        UE_LOG(LogManus, Error, TEXT("AddMeshSetupToSkeletonSetup returned %u."), t_Result);
                        return false;
                    }

                    UE_LOG(LogManus, Warning, TEXT("adding vertices and triangles for section %u."),i);

                    if (!AddVertices(&t_Section, &t_ImportedModelLod0, p_SkeletonSetupIndex, t_MeshSetupIndex)) return false;

                    if (!AddTriangles(&t_Section, &t_ImportedModelLod0, p_SkeletonSetupIndex, t_MeshSetupIndex)) return false;
                    
                }
                UE_LOG(LogManus, Warning, TEXT("done setting up the vertices for skeleton."));
            }
        }
    }
#endif

    return true;
}

#if WITH_EDITOR
bool UManusSkeleton::AddTriangles(FSkelMeshSection* p_Section, FSkeletalMeshLODModel* p_ImportedModelLod, uint32 p_SkeletonSetupIndex, uint32 p_MeshSetupIndex)
{
    TArray<FSoftSkinVertex>& t_SoftVerts = p_Section->SoftVertices;

    Triangle t_Triangle;
    for (uint32 j = 0; j < p_Section->NumTriangles; j++)
    {
        t_Triangle.vertexIndex1 = p_ImportedModelLod->IndexBuffer[p_Section->BaseIndex + j * 3] - p_Section->BaseVertexIndex;
        t_Triangle.vertexIndex2 = p_ImportedModelLod->IndexBuffer[p_Section->BaseIndex + j * 3 + 1] - p_Section->BaseVertexIndex;
        t_Triangle.vertexIndex3 = p_ImportedModelLod->IndexBuffer[p_Section->BaseIndex + j * 3 + 2] - p_Section->BaseVertexIndex;

        EManusRet t_Result = CoreSdk::AddTriangleToMeshSetup(p_SkeletonSetupIndex, p_MeshSetupIndex, t_Triangle);
        if (t_Result != EManusRet::Success)
        {
            UE_LOG(LogManus, Error, TEXT("AddTriangleToMeshSetup returned %u."), t_Result);
            return false;
        }
    }
    return true;
}

bool UManusSkeleton::AddVertices(FSkelMeshSection* p_Section, FSkeletalMeshLODModel* p_ImportedModelLod, uint32 p_SkeletonSetupIndex, uint32 p_MeshSetupIndex )
{   
    TArray<Vertex> t_Vertices;
    for (int32 t_VertIndex = 0; t_VertIndex < p_Section->SoftVertices.Num(); t_VertIndex++)
    {
        Vertex t_Vertex;
        t_Vertex.position.x = p_Section->SoftVertices[t_VertIndex].Position.X;
        t_Vertex.position.y = p_Section->SoftVertices[t_VertIndex].Position.Y;
        t_Vertex.position.z = p_Section->SoftVertices[t_VertIndex].Position.Z;

        t_Vertex.weightsCount = 0;
        
        // clear stuff before.
        for (int j = 0; j < MAX_BONE_WEIGHTS_PER_VERTEX; j++)
        {
            t_Vertex.weights[j].weightValue = 0.0f;
            t_Vertex.weights[j].nodeID = 0;
        }

        // first store all the id's in a map with weights and sort them from big to small. we are only interested in top 4, due to conversions issues to unity. (unity supports 4 tops )
        std::map<uint32, float> t_Weights;
        for (int j = 0; j < p_Section->MaxBoneInfluences; j++)
        {
            if (p_Section->SoftVertices[t_VertIndex].InfluenceWeights[j] == 0) break;

            if (p_Section->SoftVertices[t_VertIndex].InfluenceBones[j] == 0)
            {
                int h = 0;
            }

            t_Vertex.weightsCount++;
            uint32 t_NodeId = p_Section->BoneMap[p_Section->SoftVertices[t_VertIndex].InfluenceBones[j]]; // this is the part i suspect is an issue
            // the problem is. the node ids in the influencebones need to be remapped via the bonemap. 
            // doing this already greatly improved the model.
            // but it still seems like 1 (not qutie root) bone is out of whack and all its subsequent bones get distorted along with this one. this may or may not be related to the setupmesh nodeid (see calling function)
            // however even trying a reverse lookup and other variations and checking both the p_Section and p_ImportedModelLod nothing pops out as beign the cause of this, or to help adjust for this.
            // it may even be related to a different mesh. however. there are no other meshes.



            float t_Weight = (p_Section->SoftVertices[t_VertIndex].InfluenceWeights[j] / 255.0f);
            t_Weights[t_NodeId] = t_Weight;
        }

        // now sort by weight
        {
            struct WeightComparison // simple struct for sorting the vector
            {
                typedef std::pair<uint32, float> WeightType;
                bool operator ()(WeightType const& a, WeightType const& b) const
                {
                    return a.second > b.second; // higher values first.
                }
            };

            std::vector<std::pair<uint32, float> > t_WeightsCopy(t_Weights.begin(), t_Weights.end());
            sort(t_WeightsCopy.begin(), t_WeightsCopy.end(), WeightComparison());
            // now from the sorted list only grab the first 4.
            for (int j = 0; j < MAX_BONE_WEIGHTS_PER_VERTEX; j++)
            {
                if (j == t_Vertex.weightsCount) break; // only weights we got. not 0 weights.
                t_Vertex.weights[j].weightValue = t_WeightsCopy[j].second;
                t_Vertex.weights[j].nodeID = t_WeightsCopy[j].first;
            }
            if (t_Vertex.weightsCount > MAX_BONE_WEIGHTS_PER_VERTEX) t_Vertex.weightsCount = MAX_BONE_WEIGHTS_PER_VERTEX;
        }

        // normalize because unreal supports more weights then unity, this causes issues we need to adjust
        if ((t_Vertex.weightsCount == MAX_BONE_WEIGHTS_PER_VERTEX) && // if less are used, then there is nothing to normalize, as they should already be so. at the border of the limit it gets iffy
            (p_Section->MaxBoneInfluences> MAX_BONE_WEIGHTS_PER_VERTEX))
        {
            float t_TotalWeight = 0.0f;
            for (int j = 0; j < MAX_BONE_WEIGHTS_PER_VERTEX; j++)
            {
                t_TotalWeight += t_Vertex.weights[j].weightValue;
            }
            float t_Modifier = 1.0f / t_TotalWeight;
            for (int j = 0; j < MAX_BONE_WEIGHTS_PER_VERTEX; j++)
            {
                t_Vertex.weights[j].weightValue *= t_Modifier;
            }
        }
        t_Vertices.Add(t_Vertex);
    }
    // now due 1 massive loop call to store the vertices.
    for (int32 t_VertIndex = 0; t_VertIndex < t_Vertices.Num(); t_VertIndex++)
    {
        EManusRet t_Result = CoreSdk::AddVertexToMeshSetup(p_SkeletonSetupIndex, p_MeshSetupIndex, t_Vertices[t_VertIndex]);
        if (t_Result != EManusRet::Success)
        {
            UE_LOG(LogManus, Error, TEXT("AddVertexToMeshSetup returned %u."), t_Result);
            return false;
        }
    }
    return true;
}
#endif // with editor

/// @brief Convert skeleton information to skeleton setup
bool UManusSkeleton::ToSkeletonSetup(SkeletonSetupInfo& t_SKL)
{
	if (TargetName.IsEmpty() && SkeletalMesh != NULL)
	{
		TargetName = SkeletalMesh->GetFName().ToString();
	}

	SkeletonSettings t_Settings;
	ManusTypeInitializer::SkeletonSettings_Init(&t_Settings);
	t_Settings.scaleToTarget = ScaleToUser;// false;
    t_Settings.useEndPointApproximations = UseEndPointApproximations;

	if (TargetType == EManusSkeletonTargetType::AnimationData)
	{
		t_Settings.targetType = SkeletonTargetType::SkeletonTargetType_AnimationData;
		char* t_Id = TCHAR_TO_ANSI(*TargetAnimationData.id);
		strcpy_s(t_Settings.skeletonTargetAnimationData.id, t_Id);
	}
	else if (TargetType == EManusSkeletonTargetType::GloveData)
	{
		t_Settings.targetType = SkeletonTargetType::SkeletonTargetType_GloveData;
		t_Settings.skeletonGloveData.gloveID = GloveData.gloveID;

	}
	else if (TargetType == EManusSkeletonTargetType::UserData)
	{
		t_Settings.targetType = SkeletonTargetType::SkeletonTargetType_UserData;
		t_Settings.skeletonTargetUserData.userID = TargetUserData.userID;

	}
	else if (TargetType == EManusSkeletonTargetType::UserIndexData)
	{
		t_Settings.targetType = SkeletonTargetType::SkeletonTargetType_UserIndexData;
		t_Settings.skeletonTargetUserIndexData.userIndex = TargetUserIndexData.userIndex;
	}
	else 
	{
		UE_LOG(LogManus, Error, TEXT("Skeleton has no defined targettype."));
		return false;
	}
	
	t_SKL.id = ManusSkeletonId;
    if (SkeletonType == EManusSkeletonType::Body)	t_SKL.type = SkeletonType::SkeletonType_Body;
    else if (SkeletonType == EManusSkeletonType::Both)	t_SKL.type = SkeletonType::SkeletonType_Both;
    else if (SkeletonType == EManusSkeletonType::Hand)	t_SKL.type = SkeletonType::SkeletonType_Hand;
    else t_SKL.type = SkeletonType::SkeletonType_Body; // default value
    t_SKL.settings = t_Settings;

	ManusTools::ConvertFStringToUTF8Array(t_SKL.name, TargetName, MAX_NUM_CHARS_IN_SKELETON_NAME);

	return true;
}

/// @brief setup a skeleton for manus core (not yet sending it)
EManusRet UManusSkeleton::SetupSkeleton(uint32_t& p_SkeletonSetupIndex)
{
	// setup base skeleton for Manus core
	p_SkeletonSetupIndex = 0;

	// Convert skeleton information to skeleton setup
	SkeletonSetupInfo t_SKL;
	ManusTypeInitializer::SkeletonSetupInfo_Init(&t_SKL);
	bool t_Res = ToSkeletonSetup(t_SKL);
	if (!t_Res)
	{
		return EManusRet::InvalidArgument;
	}

	EManusRet t_Result =  CoreSdk::CreateSkeletonSetup(t_SKL, p_SkeletonSetupIndex);

    return t_Result;
}

void UManusSkeleton::ClearAllTemporarySkeletonIndexes()
{
	TArray<FManusLiveLinkUser>& t_ManusLiveLinkUsers = FManusModule::Get().ManusLiveLinkUsers;
	for (size_t i = 0; i < t_ManusLiveLinkUsers.Num(); i++)
	{
		UManusSkeleton* t_ManusSkeleton = t_ManusLiveLinkUsers[i].ManusSkeleton;
		if (t_ManusSkeleton)
		{
			t_ManusSkeleton->SetTemporarySkeletonIndex(UINT32_MAX);
		}
	}
}

/// @brief get current temporary skeleton id
uint32_t UManusSkeleton::GetTemporarySkeletonIndex()
{
	return m_TemporarySkeletonIndex; 
}

/// @brief set the temporary skeleton id
void UManusSkeleton::SetTemporarySkeletonIndex(uint32_t p_Index)
{
	m_TemporarySkeletonIndex = p_Index;
}

/// @brief retrieve temporary skeleton from manus core.
void UManusSkeleton::RetrieveTemporarySkeleton()
{
	// retrieve the temp skeleton
	uint32_t t_SessionId = 0;
	if (CoreSdk::GetSessionId(t_SessionId) != EManusRet::Success)
	{
		return;
	}
	// get the temporary skeleton
	if (CoreSdk::GetTemporarySkeleton(m_TemporarySkeletonIndex, t_SessionId) != EManusRet::Success)
	{
		return;
	}
    SkeletonSetupInfo t_SkeletonSetup;
    ManusTypeInitializer::SkeletonSetupInfo_Init(&t_SkeletonSetup);
    if (CoreSdk::GetSkeletonSetupInfo(m_TemporarySkeletonIndex, &t_SkeletonSetup) != EManusRet::Success)
    {
        UE_LOG(LogManus, Error, TEXT("Failed to get info about skeleton setup."));
        return ;
    }

	// get the skeleton info (number of nodes and chains for that skeleton)
	SkeletonSetupArraySizes t_SkeletonInfo;
	if(CoreSdk::GetSkeletonSetupArraySizes(m_TemporarySkeletonIndex, t_SkeletonInfo) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to retrieve temporary skeleton sizes."));
		return;
	}

	// now get the chain data
	ChainSetup* t_Chains = new ChainSetup[t_SkeletonInfo.chainsCount];
	if (CoreSdk::GetSkeletonSetupChains(m_TemporarySkeletonIndex, t_Chains) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to retrieve temporary skeleton chains."));
		delete[] t_Chains;
		return;
	}
	NodeSetup* t_Nodes = new NodeSetup[t_SkeletonInfo.nodesCount];
	if (CoreSdk::GetSkeletonSetupNodes(m_TemporarySkeletonIndex, t_Nodes) != EManusRet::Success)
	{
		UE_LOG(LogManus, Error, TEXT("Failed to retrieve temporary skeleton nodes.")); 
		delete[] t_Chains;
		delete[] t_Nodes;
		return;
	}

	ChainsIndexMap.Reset();
	// process the chains
	const TArray<FMeshBoneInfo>& t_BoneInfo = GetRawRefBoneInfo(); // todo ?
	for (uint32_t i = 0; i < t_SkeletonInfo.chainsCount; i++)
	{
		FManusChainSetup t_Chain;
		ManusConvert::SDKManusChainSetupToBp(t_Chain, t_Chains[i]);        
        ChainsIndexMap.Add(GenerateChainName(t_Chain), t_Chain);
	}
	delete[] t_Chains;

	for (uint32_t i = 0; i < t_SkeletonInfo.nodesCount; i++)
	{
		FManusNodeSetup t_Node;
		ManusConvert::SDKManusNodeSetupToBP(t_Node, t_Nodes[i]);
        if (!NodesSetupMap.Contains(FName(t_Node.Name)))
        {
            UE_LOG(LogManus, Error, TEXT("Failed due to new skeleton node detected where it shouldnt. did not add %d"),*t_Node.Name);
            continue;
        }
        NodesSetupMap[FName(t_Node.Name)] = t_Node;
	}
	delete[] t_Nodes;
}

#undef LOCTEXT_NAMESPACE