// Copyright 2015-2022 Manus

#include "ManusConvert.h"
#include "Manus.h"
#include <Manus/Public/ManusTools.h>

// Convert an SDK hand type to a blueprint-compatible hand type.
EManusRet ManusConvert::SdkHandTypeToBp(Side Input, EManusHandType &Output)
{
	switch (Input)
	{
	case Side::Side_Left:
		Output = EManusHandType::Left;

		return EManusRet::Success;

	case Side::Side_Right:
		Output = EManusHandType::Right;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK HandType value to a blueprint-compatible HandType value. The value was %d."), static_cast<int>(Input));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert a blueprint-compatible hand type to an SDK hand type.
EManusRet ManusConvert::BpHandTypeToSdk(EManusHandType Input, Side&Output)
{
	switch (Input)
	{
	case EManusHandType::Left:
		Output = Side::Side_Left;

		return EManusRet::Success;

	case EManusHandType::Right:
		Output = Side::Side_Right;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised blueprint-compatible HandType value to an SDK HandType value. The value was %d."), static_cast<int>(Input));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert an SDK tracker type to a blueprint-compatible hand type.
EManusRet ManusConvert::SdkTrackerTypeToBpHandType(uint32_t Input, EManusHandType& Output)
{
	// See the file Tracker.pb.h for the "TrackerType" enum values used as SDK tracker types.
	switch (Input)
	{
	case 3:
		Output = EManusHandType::Left;
		return EManusRet::Success;

	case 4:
		Output = EManusHandType::Right;
		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK tracker type value to a BP ManusHandType value. The value was %d."), (int)Input);
		return EManusRet::InvalidArgument;
	}
	return EManusRet::Error;
}

// Convert a blueprint-compatible hand type to an SDK tracker type.
EManusRet ManusConvert::BpHandTypeToSdkTrackerType(EManusHandType Input, uint32_t& Output)
{
	// See the file Tracker.pb.h for the "TrackerType" enum values used as SDK tracker types.
	switch (Input)
	{
	case EManusHandType::Left:
		Output = 3;
		return EManusRet::Success;

	case EManusHandType::Right:
		Output = 4;
		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised BP ManusHandType value to an SDK tracker type value. The value was %d."), (int)Input);
		return EManusRet::InvalidArgument;
	}
	return EManusRet::Error;
}


// Convert an SDK hand type to a blueprint-compatible hand type.
EManusRet ManusConvert::SdkDeviceTypeToBp(DeviceFamilyType p_Input, EManusGloveType& p_Output)
{
	switch (p_Input)
	{
		case DeviceFamilyType::DeviceFamilyType_Quantum:
			p_Output = EManusGloveType::PrimeMeta;
			return EManusRet::Success;
		case DeviceFamilyType::DeviceFamilyType_Prime1:
			p_Output = EManusGloveType::PrimeOne;
			return EManusRet::Success;
		case DeviceFamilyType::DeviceFamilyType_Prime2:
			p_Output = EManusGloveType::PrimeTwo;
			return EManusRet::Success;
		case DeviceFamilyType::DeviceFamilyType_PrimeX:
			p_Output = EManusGloveType::PrimeX;
			return EManusRet::Success;

		default:
			UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK DeviceFamilyType value to a blueprint-compatible EManusGloveType value. The value was %d."), static_cast<int>(p_Input));
			return EManusRet::InvalidArgument;
	}
	return EManusRet::Error;
}

// convert a blueprint NodeSetup to SDK version
void ManusConvert::BpManusNodeSetupToSDK(NodeSetup& p_OutputNode, const FManusNodeSetup& p_InputNode)
{
    p_OutputNode.id = (uint32_t)p_InputNode.Id;
    
    ManusTools::ConvertFStringToUTF8Array(p_OutputNode.name, p_InputNode.Name, MAX_NUM_CHARS_IN_NODE_NAME);
        
	p_OutputNode.parentID = (uint32_t)p_InputNode.ParentID;
	// settings
	p_OutputNode.settings.usedSettings = (NodeSettingsFlag)p_InputNode.Settings.UsedSettings; // bit of a hacky convert. but as long as both enums are in sync. this is fine. (in b4 it blew up) 

	if (p_OutputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_IK) p_OutputNode.settings.ik.ikAim = p_InputNode.Settings.Ik.IkAim;
	if (p_OutputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_Foot) p_OutputNode.settings.foot.heightFromGround = p_InputNode.Settings.Foot.HeightFromGround;
	if (p_OutputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_Leaf)
	{
		p_OutputNode.settings.leaf.direction.x = p_InputNode.Settings.Leaf.Direction.X;
		p_OutputNode.settings.leaf.direction.y = p_InputNode.Settings.Leaf.Direction.Y;
		p_OutputNode.settings.leaf.direction.z = p_InputNode.Settings.Leaf.Direction.Z;
		p_OutputNode.settings.leaf.length = p_InputNode.Settings.Leaf.Length;
	}
	if (p_OutputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_RotationOffset)
	{
		p_OutputNode.settings.rotationOffset.value.x = p_InputNode.Settings.RotationOffset.Value.X;
		p_OutputNode.settings.rotationOffset.value.y = p_InputNode.Settings.RotationOffset.Value.Y;
		p_OutputNode.settings.rotationOffset.value.z = p_InputNode.Settings.RotationOffset.Value.Z;
		p_OutputNode.settings.rotationOffset.value.w = p_InputNode.Settings.RotationOffset.Value.W;
	}

	if (p_InputNode.Type == EManusNodeType::Joint) p_OutputNode.type = NodeType::NodeType_Joint;
	else if (p_InputNode.Type == EManusNodeType::Mesh) p_OutputNode.type = NodeType::NodeType_Mesh;
	else p_OutputNode.type = NodeType::NodeType_Invalid;

	p_OutputNode.transform.position.x = p_InputNode.Transform.GetLocation().X;
	p_OutputNode.transform.position.y = p_InputNode.Transform.GetLocation().Y;
	p_OutputNode.transform.position.z = p_InputNode.Transform.GetLocation().Z;

	p_OutputNode.transform.scale.x = p_InputNode.Transform.GetScale3D().X;
	p_OutputNode.transform.scale.y = p_InputNode.Transform.GetScale3D().Y;
	p_OutputNode.transform.scale.z = p_InputNode.Transform.GetScale3D().Z;

	p_OutputNode.transform.rotation.x = p_InputNode.Transform.GetRotation().X;
	p_OutputNode.transform.rotation.y = p_InputNode.Transform.GetRotation().Y;
	p_OutputNode.transform.rotation.z = p_InputNode.Transform.GetRotation().Z;
	p_OutputNode.transform.rotation.w = p_InputNode.Transform.GetRotation().W;
}

void ManusConvert::SDKManusNodeSetupToBP(FManusNodeSetup& p_OutputNode, const NodeSetup& p_InputNode)
{
	p_OutputNode.Id = p_InputNode.id;

    // convert utf8 array to fstring
    FUTF8ToTCHAR t_ToTCharConverter(p_InputNode.name);
    p_OutputNode.Name = FString(t_ToTCharConverter.Get());
	
	p_OutputNode.ParentID = p_InputNode.parentID;
	// settings
	p_OutputNode.Settings.UsedSettings = (int)p_InputNode.settings.usedSettings; // bit of a hacky convert. but as long as both enums are in sync. this is fine. (in b4 it blew up) 

	if (p_InputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_IK) p_OutputNode.Settings.Ik.IkAim = p_InputNode.settings.ik.ikAim;
	if (p_InputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_Foot) p_OutputNode.Settings.Foot.HeightFromGround = p_InputNode.settings.foot.heightFromGround;
	if (p_InputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_Leaf)
	{
		p_OutputNode.Settings.Leaf.Direction.X = p_InputNode.settings.leaf.direction.x;
		p_OutputNode.Settings.Leaf.Direction.Y = p_InputNode.settings.leaf.direction.y;
		p_OutputNode.Settings.Leaf.Direction.Z = p_InputNode.settings.leaf.direction.z;
		p_OutputNode.Settings.Leaf.Length = p_InputNode.settings.leaf.length;
	}
	if (p_InputNode.settings.usedSettings & NodeSettingsFlag::NodeSettingsFlag_RotationOffset)
	{
		p_OutputNode.Settings.RotationOffset.Value.X = p_InputNode.settings.rotationOffset.value.x;
		p_OutputNode.Settings.RotationOffset.Value.Y = p_InputNode.settings.rotationOffset.value.y;
		p_OutputNode.Settings.RotationOffset.Value.Z = p_InputNode.settings.rotationOffset.value.z;
		p_OutputNode.Settings.RotationOffset.Value.W = p_InputNode.settings.rotationOffset.value.w;
	}

	if (p_InputNode.type == NodeType::NodeType_Joint) p_OutputNode.Type = EManusNodeType::Joint;
	else if (p_InputNode.type == NodeType::NodeType_Mesh) p_OutputNode.Type = EManusNodeType::Mesh;
	else p_OutputNode.Type = EManusNodeType::Invalid;

	p_OutputNode.Transform.SetLocation(FVector(p_InputNode.transform.position.x,
											   p_InputNode.transform.position.y,
											   p_InputNode.transform.position.z));

	p_OutputNode.Transform.SetScale3D(FVector(p_InputNode.transform.scale.x,
											  p_InputNode.transform.scale.y,
											  p_InputNode.transform.scale.z));

	p_OutputNode.Transform.SetRotation(FQuat(p_InputNode.transform.rotation.x,
											 p_InputNode.transform.rotation.y,
											 p_InputNode.transform.rotation.z,
											 p_InputNode.transform.rotation.w));
}

void ManusConvert::BpManusChainSetupToSDK(ChainSetup &p_OutputChain, const FManusChainSetup &p_InputChain, bool p_Overridetrackers)
{
	p_OutputChain.id = (uint32_t)p_InputChain.Id;
	p_OutputChain.type = ChainTypeToSDK(p_InputChain.Type);
	p_OutputChain.dataType = ChainTypeToSDK(p_InputChain.DataType);
	p_OutputChain.nodeIdCount = p_InputChain.NodeIds.Num();
	for (int i = 0; i < p_InputChain.NodeIds.Num(); i++)
	{
		p_OutputChain.nodeIds[i] = p_InputChain.NodeIds[i];
	}
	p_OutputChain.dataIndex = p_InputChain.DataIndex;
	p_OutputChain.side = BpSideToSdk(p_InputChain.Side);
	BpChainSettingsToSDK(p_InputChain.Settings, p_OutputChain.settings, p_Overridetrackers);
}

void ManusConvert::SDKManusChainSetupToBp(FManusChainSetup& p_OutputChain, const ChainSetup& p_InputChain)
{
	p_OutputChain.Id = (uint32_t)p_InputChain.id;
	p_OutputChain.Type = ChainTypeToBp(p_InputChain.type);
	p_OutputChain.DataType = ChainTypeToBp(p_InputChain.dataType);
	for (uint32_t i = 0; i < p_InputChain.nodeIdCount; i++)
	{
		p_OutputChain.NodeIds.Add( p_InputChain.nodeIds[i]);
	}
	p_OutputChain.DataIndex = p_InputChain.dataIndex;
	p_OutputChain.Side = SDKSideToBp(p_InputChain.side);
	SDKChainSettingsToBp(p_InputChain.settings, p_OutputChain.Settings);
}

void ManusConvert::BpVectorToSDK(const FVector& p_Input, ManusVec3& p_Output)
{
	p_Output.x = p_Input.X;
	p_Output.y = p_Input.Y;
	p_Output.z = p_Input.Z;
}

void ManusConvert::SDKVectorToBp(const ManusVec3& p_Input, FVector& p_Output)
{
	p_Output.X = p_Input.x;
	p_Output.Y = p_Input.y;
	p_Output.Z = p_Input.z;
}

EManusHandMotion ManusConvert::SDKHandMotionToBp(const HandMotion& p_Input)
{
	if (p_Input == HandMotion::HandMotion_IMU) return EManusHandMotion::IMU;
	if (p_Input == HandMotion::HandMotion_Tracker) return EManusHandMotion::Tracker;
    if (p_Input == HandMotion::HandMotion_Tracker_RotationOnly) return EManusHandMotion::Tracker_RotationOnly;
    if (p_Input == HandMotion::HandMotion_Auto) return EManusHandMotion::Auto;
    return EManusHandMotion::None;
}

HandMotion ManusConvert::BpHandMotionToSDK(const EManusHandMotion& p_Input)
{
	if (p_Input == EManusHandMotion::IMU) return HandMotion::HandMotion_IMU;
	if (p_Input == EManusHandMotion::Tracker) return HandMotion::HandMotion_Tracker;
    if (p_Input == EManusHandMotion::Tracker_RotationOnly) return HandMotion::HandMotion_Tracker_RotationOnly;
    if (p_Input == EManusHandMotion::Auto) return HandMotion::HandMotion_Auto;
	return HandMotion::HandMotion_None;
}

void ManusConvert::BpChainSettingsToSDK(const FManusChainSettings& p_Input, ChainSettings& p_Output, bool p_Overridetrackers)
{
	switch (p_Input.UsedSettings)
	{
		case EManusChainType::Arm:
		{
			p_Output.usedSettings = ChainType::ChainType_Arm;
			p_Output.arm.armLengthMultiplier = p_Input.Arm.ArmLengthMultiplier;
			BpVectorToSDK( p_Input.Arm.ArmRotationOffset, p_Output.arm.armRotationOffset);
			
			p_Output.arm.elbowRotationOffset = p_Input.Arm.ElbowRotationOffset;
			BpVectorToSDK(p_Input.Arm.PositionMultiplier, p_Output.arm.positionMultiplier);
			
			BpVectorToSDK(p_Input.Arm.PositionOffset, p_Output.arm.positionOffset);
			break;
		}
		case EManusChainType::FingerIndex:
		{
			p_Output.usedSettings = ChainType::ChainType_FingerIndex;
			p_Output.finger.handChainId = (uint32_t)p_Input.Finger.HandChainId;
			p_Output.finger.metacarpalBoneId = (uint32_t)p_Input.Finger.MetacarpalBoneId;
			p_Output.finger.useLeafAtEnd = p_Input.Finger.UseLeafAtEnd;
			break;
		}
		case EManusChainType::FingerMiddle:
		{
			p_Output.usedSettings = ChainType::ChainType_FingerMiddle;
			p_Output.finger.handChainId = (uint32_t)p_Input.Finger.HandChainId;
			p_Output.finger.metacarpalBoneId = (uint32_t)p_Input.Finger.MetacarpalBoneId;
			p_Output.finger.useLeafAtEnd = p_Input.Finger.UseLeafAtEnd;
			break;
		}
		case EManusChainType::FingerPinky:
		{
			p_Output.usedSettings = ChainType::ChainType_FingerPinky;
			p_Output.finger.handChainId = (uint32_t)p_Input.Finger.HandChainId;
			p_Output.finger.metacarpalBoneId = (uint32_t)p_Input.Finger.MetacarpalBoneId;
			p_Output.finger.useLeafAtEnd = p_Input.Finger.UseLeafAtEnd;
			break;
		}
		case EManusChainType::FingerRing:
		{
			p_Output.usedSettings = ChainType::ChainType_FingerRing;
			p_Output.finger.handChainId = (uint32_t)p_Input.Finger.HandChainId;
			p_Output.finger.metacarpalBoneId = (uint32_t)p_Input.Finger.MetacarpalBoneId;
			p_Output.finger.useLeafAtEnd = p_Input.Finger.UseLeafAtEnd;
			break;
		}
		case EManusChainType::FingerThumb:
		{
			p_Output.usedSettings = ChainType::ChainType_FingerThumb;
			p_Output.finger.handChainId = (uint32_t)p_Input.Finger.HandChainId;
			p_Output.finger.metacarpalBoneId = (uint32_t)p_Input.Finger.MetacarpalBoneId;
			p_Output.finger.useLeafAtEnd = p_Input.Finger.UseLeafAtEnd;
			break;
		}
		case EManusChainType::Foot:
		{
			p_Output.usedSettings = ChainType::ChainType_Foot;
			p_Output.foot.toeChainIdsUsed = p_Input.Foot.ToeChainIds.Num();
			for (int i = 0; i < p_Input.Foot.ToeChainIds.Num(); i++)
			{
				p_Output.foot.toeChainIds[i] = p_Input.Foot.ToeChainIds[i];
			}
			break;
		}
		case EManusChainType::Hand:
		{
			p_Output.usedSettings = ChainType::ChainType_Hand;
			p_Output.hand.fingerChainIdsUsed = p_Input.Hand.FingerChainIds.Num();
			for (int i = 0; i < p_Input.Hand.FingerChainIds.Num(); i++)
			{
				p_Output.hand.fingerChainIds[i] = p_Input.Hand.FingerChainIds[i];
			}
			p_Output.hand.handMotion = BpHandMotionToSDK(p_Input.Hand.HandMotion);
            if (p_Overridetrackers)
            {
                p_Output.hand.handMotion = HandMotion::HandMotion_Tracker; // override specifically for the demo
            }

			break;
		}
		case EManusChainType::Head:
		{
			p_Output.usedSettings = ChainType::ChainType_Head;
			p_Output.head.headPitchOffset = p_Input.Head.HeadPitchOffset;
			p_Output.head.headTiltOffset = p_Input.Head.HeadTiltOffset;
			p_Output.head.headYawOffset = p_Input.Head.HeadYawOffset;
			p_Output.head.useLeafAtEnd = p_Input.Head.UseLeafAtEnd;
			break;
		}
		case EManusChainType::Leg:
		{
			p_Output.usedSettings = ChainType::ChainType_Leg;
			p_Output.leg.footForwardOffset = p_Input.Leg.FootForwardOffset;
			p_Output.leg.footSideOffset = p_Input.Leg.FootSideOffset;
			p_Output.leg.kneeRotationOffset = p_Input.Leg.KneeRotationOffset;
			p_Output.leg.reverseKneeDirection = p_Input.Leg.ReverseKneeDirection;
			break;
		}
		case EManusChainType::Neck:
		{
			p_Output.usedSettings = ChainType::ChainType_Neck;
			p_Output.neck.neckBendOffset = p_Input.Neck.NeckBendOffset;
			break;
		}
		case EManusChainType::Pelvis:
		{
			p_Output.usedSettings = ChainType::ChainType_Pelvis;
			p_Output.pelvis.hipBendOffset = p_Input.Pelvis.HipBendOffset;
			p_Output.pelvis.hipHeight = p_Input.Pelvis.HipHeight;
			p_Output.pelvis.thicknessMultiplier = p_Input.Pelvis.ThicknessMultiplier;
			break;
		}
		case EManusChainType::Shoulder:
		{
			p_Output.usedSettings = ChainType::ChainType_Shoulder;
			p_Output.shoulder.forwardMultiplier = p_Input.Shoulder.ForwardMultiplier;
			p_Output.shoulder.forwardOffset = p_Input.Shoulder.ForwardOffset;
			p_Output.shoulder.shrugMultiplier = p_Input.Shoulder.ShrugMultiplier;
			p_Output.shoulder.shrugOffset = p_Input.Shoulder.ShrugOffset;
			break;
		}
		case EManusChainType::Spine:
		{
			p_Output.usedSettings = ChainType::ChainType_Spine;
			p_Output.spine.spineBendOffset = p_Input.Spine.SpineBendOffset;
			break;
		}
		case EManusChainType::Toe:
		{
			p_Output.usedSettings = ChainType::ChainType_Toe;
			p_Output.toe.footChainId = p_Input.Toe.FootChainId;
			p_Output.toe.useLeafAtEnd = p_Input.Toe.UseLeafAtEnd;
			break;
		}
		case EManusChainType::Invalid:
		{
			p_Output.usedSettings = ChainType::ChainType_Invalid;
			break;
		}
	}
}

void ManusConvert::SDKChainSettingsToBp(const ChainSettings& p_Input, FManusChainSettings& p_Output)
{

	switch (p_Input.usedSettings)
	{
		case ChainType::ChainType_Arm:
		{
			p_Output.UsedSettings = EManusChainType::Arm;
			p_Output.Arm.ArmLengthMultiplier = p_Input.arm.armLengthMultiplier;
			SDKVectorToBp(p_Input.arm.armRotationOffset, p_Output.Arm.ArmRotationOffset);

			p_Output.Arm.ElbowRotationOffset = p_Input.arm.elbowRotationOffset;
			SDKVectorToBp(p_Input.arm.positionMultiplier, p_Output.Arm.PositionMultiplier);

			SDKVectorToBp(p_Input.arm.positionOffset, p_Output.Arm.PositionOffset);
			break;
		}
		case ChainType::ChainType_FingerIndex:
		{
			p_Output.UsedSettings = EManusChainType::FingerIndex;
			p_Output.Finger.HandChainId = (uint32_t)p_Input.finger.handChainId;
			p_Output.Finger.MetacarpalBoneId = (uint32_t)p_Input.finger.metacarpalBoneId;
			p_Output.Finger.UseLeafAtEnd = p_Input.finger.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_FingerMiddle:
		{
			p_Output.UsedSettings = EManusChainType::FingerMiddle;
			p_Output.Finger.HandChainId = (uint32_t)p_Input.finger.handChainId;
			p_Output.Finger.MetacarpalBoneId = (uint32_t)p_Input.finger.metacarpalBoneId;
			p_Output.Finger.UseLeafAtEnd = p_Input.finger.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_FingerPinky:
		{
			p_Output.UsedSettings = EManusChainType::FingerPinky;
			p_Output.Finger.HandChainId = (uint32_t)p_Input.finger.handChainId;
			p_Output.Finger.MetacarpalBoneId = (uint32_t)p_Input.finger.metacarpalBoneId;
			p_Output.Finger.UseLeafAtEnd = p_Input.finger.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_FingerRing:
		{
			p_Output.UsedSettings = EManusChainType::FingerRing;
			p_Output.Finger.HandChainId = (uint32_t)p_Input.finger.handChainId;
			p_Output.Finger.MetacarpalBoneId = (uint32_t)p_Input.finger.metacarpalBoneId;
			p_Output.Finger.UseLeafAtEnd = p_Input.finger.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_FingerThumb:
		{
			p_Output.UsedSettings = EManusChainType::FingerThumb;
			p_Output.Finger.HandChainId = (uint32_t)p_Input.finger.handChainId;
			p_Output.Finger.MetacarpalBoneId = (uint32_t)p_Input.finger.metacarpalBoneId;
			p_Output.Finger.UseLeafAtEnd = p_Input.finger.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_Foot:
		{
			p_Output.UsedSettings = EManusChainType::Foot;
			for (int i = 0; i < p_Input.foot.toeChainIdsUsed; i++)
			{
				p_Output.Foot.ToeChainIds.Add( p_Input.foot.toeChainIds[i]);
			}
			break;
		}
		case ChainType::ChainType_Hand:
		{
			p_Output.UsedSettings = EManusChainType::Hand;			
			for (int i = 0; i < p_Input.hand.fingerChainIdsUsed; i++)
			{
				p_Output.Hand.FingerChainIds.Add( p_Input.hand.fingerChainIds[i]);
			}
			p_Output.Hand.HandMotion = SDKHandMotionToBp(p_Input.hand.handMotion);
			break;
		}
		case ChainType::ChainType_Head:
		{
			p_Output.UsedSettings = EManusChainType::Head;
			p_Output.Head.HeadPitchOffset = p_Input.head.headPitchOffset;
			p_Output.Head.HeadTiltOffset = p_Input.head.headTiltOffset;
			p_Output.Head.HeadYawOffset = p_Input.head.headYawOffset;
			p_Output.Head.UseLeafAtEnd = p_Input.head.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_Leg:
		{
			p_Output.UsedSettings = EManusChainType::Leg;
			p_Output.Leg.FootForwardOffset = p_Input.leg.footForwardOffset;
			p_Output.Leg.FootSideOffset = p_Input.leg.footSideOffset;
			p_Output.Leg.KneeRotationOffset = p_Input.leg.kneeRotationOffset;
			p_Output.Leg.ReverseKneeDirection = p_Input.leg.reverseKneeDirection;
			break;
		}
		case ChainType::ChainType_Neck:
		{
			p_Output.UsedSettings = EManusChainType::Neck;
			p_Output.Neck.NeckBendOffset = p_Input.neck.neckBendOffset;
			break;
		}
		case ChainType::ChainType_Pelvis:
		{
			p_Output.UsedSettings = EManusChainType::Pelvis;
			p_Output.Pelvis.HipBendOffset = p_Input.pelvis.hipBendOffset;
			p_Output.Pelvis.HipHeight = p_Input.pelvis.hipHeight;
			p_Output.Pelvis.ThicknessMultiplier = p_Input.pelvis.thicknessMultiplier;
			break;
		}
		case ChainType::ChainType_Shoulder:
		{
			p_Output.UsedSettings = EManusChainType::Shoulder;
			p_Output.Shoulder.ForwardMultiplier = p_Input.shoulder.forwardMultiplier;
			p_Output.Shoulder.ForwardOffset = p_Input.shoulder.forwardOffset;
			p_Output.Shoulder.ShrugMultiplier = p_Input.shoulder.shrugMultiplier;
			p_Output.Shoulder.ShrugOffset = p_Input.shoulder.shrugOffset;
			break;
		}
		case ChainType::ChainType_Spine:
		{
			p_Output.UsedSettings = EManusChainType::Spine;
			p_Output.Spine.SpineBendOffset = p_Input.spine.spineBendOffset;
			break;
		}
		case ChainType::ChainType_Toe:
		{
			p_Output.UsedSettings = EManusChainType::Toe;
			p_Output.Toe.FootChainId = p_Input.toe.footChainId;
			p_Output.Toe.UseLeafAtEnd = p_Input.toe.useLeafAtEnd;
			break;
		}
		case ChainType::ChainType_Invalid:
		{
			p_Output.UsedSettings = EManusChainType::Invalid;
			break;
		}
	}
}

Side ManusConvert::BpSideToSdk(const EManusSide p_Input)
{
	if (p_Input == EManusSide::Left) return Side::Side_Left;
	if (p_Input == EManusSide::Right) return Side::Side_Right;
	if (p_Input == EManusSide::Center) return Side::Side_Center;
	return Side::Side_Invalid;
}

EManusSide ManusConvert::SDKSideToBp(const Side p_Input)
{
	if (p_Input == Side::Side_Left) return EManusSide::Left;
	if (p_Input == Side::Side_Right) return EManusSide::Right;
	if (p_Input == Side::Side_Center) return EManusSide::Center;
	return EManusSide::Invalid;
}



// support function
ChainType ManusConvert::ChainTypeToSDK(const EManusChainType p_Input)
{
	switch (p_Input)
	{
		case EManusChainType::Arm: return ChainType::ChainType_Arm;
		case EManusChainType::FingerIndex: return ChainType::ChainType_FingerIndex;
		case EManusChainType::FingerMiddle: return ChainType::ChainType_FingerMiddle;
		case EManusChainType::FingerPinky: return ChainType::ChainType_FingerPinky;
		case EManusChainType::FingerRing: return ChainType::ChainType_FingerRing;
		case EManusChainType::FingerThumb: return ChainType::ChainType_FingerThumb;
		case EManusChainType::Foot: return ChainType::ChainType_Foot;
		case EManusChainType::Hand: return ChainType::ChainType_Hand;
		case EManusChainType::Head: return ChainType::ChainType_Head;
		case EManusChainType::Leg: return ChainType::ChainType_Leg;
		case EManusChainType::Neck: return ChainType::ChainType_Neck;
		case EManusChainType::Pelvis: return ChainType::ChainType_Pelvis;
		case EManusChainType::Shoulder: return ChainType::ChainType_Shoulder;
		case EManusChainType::Spine: return ChainType::ChainType_Spine;
		case EManusChainType::Toe: return ChainType::ChainType_Toe;
	}
	return ChainType::ChainType_Invalid;
}

EManusChainType ManusConvert::ChainTypeToBp(const ChainType p_Input)
{
	switch (p_Input)
	{
		case ChainType::ChainType_Arm: return EManusChainType::Arm;
		case ChainType::ChainType_FingerIndex: return EManusChainType::FingerIndex;
		case ChainType::ChainType_FingerMiddle: return EManusChainType::FingerMiddle;
		case ChainType::ChainType_FingerPinky: return EManusChainType::FingerPinky;
		case ChainType::ChainType_FingerRing: return EManusChainType::FingerRing;
		case ChainType::ChainType_FingerThumb: return EManusChainType::FingerThumb;
		case ChainType::ChainType_Foot: return EManusChainType::Foot;
		case ChainType::ChainType_Hand: return EManusChainType::Hand;
		case ChainType::ChainType_Head: return EManusChainType::Head;
		case ChainType::ChainType_Leg: return EManusChainType::Leg;
		case ChainType::ChainType_Neck: return EManusChainType::Neck;
		case ChainType::ChainType_Pelvis: return EManusChainType::Pelvis;
		case ChainType::ChainType_Shoulder: return EManusChainType::Shoulder;
		case ChainType::ChainType_Spine: return EManusChainType::Spine;
		case ChainType::ChainType_Toe: return EManusChainType::Toe;
	}
	return EManusChainType::Invalid;
}