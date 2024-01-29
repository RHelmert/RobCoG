// Don't forget to update or add initializers when we make/update types.
// this is a 99% copy of the SDK version, but due to the way that unreal handles external dll's etc. we duplicate it.
#include "ManusTypeInitializers.h"

#include <cstddef>

void ManusTypeInitializer::ManusVec3_Init(ManusVec3* p_Val)
{
	p_Val->x = 0;
	p_Val->y = 0;
	p_Val->z = 0;
}

void ManusTypeInitializer::ManusVec2_Init(ManusVec2* p_Val)
{
	p_Val->x = 0;
	p_Val->y = 0;
}

void ManusTypeInitializer::ManusQuaternion_Init(ManusQuaternion* p_Val)
{
	p_Val->w = 1;
	p_Val->x = 0;
	p_Val->y = 0;
	p_Val->z = 0;
}

void ManusTypeInitializer::ManusTransform_Init(ManusTransform* p_Val)
{
	ManusVec3_Init(&p_Val->position);
	ManusQuaternion_Init(&p_Val->rotation);
	p_Val->scale.x = 1.0f;
	p_Val->scale.y = 1.0f;
	p_Val->scale.z = 1.0f;
}


void ManusTypeInitializer::Color_Init(Color* p_Val)
{
    p_Val->r = 0;
    p_Val->g = 0;
    p_Val->b = 0;
    p_Val->a = 0;
}

void ManusTypeInitializer::ManusTimestampInfo_Init(ManusTimestampInfo* p_Val)
{
    p_Val->fraction = 0;
    p_Val->second = 0;
    p_Val->minute = 0;
    p_Val->hour = 0;
    p_Val->day = 0;
    p_Val->month = 0;
    p_Val->year = 0;
    p_Val->timecode = false;
}

void ManusTypeInitializer::ManusTimestamp_Init(ManusTimestamp* p_Val)
{
    p_Val->time = 0;
}

void ManusTypeInitializer::IMUCalibrationInfo_Init(IMUCalibrationInfo* p_Val)
{
	p_Val->mag = 0;
	p_Val->acc = 0;
	p_Val->gyr = 0;
	p_Val->sys = 0;
}

void ManusTypeInitializer::Version_Init(Version* p_Val)
{
	p_Val->major = 0;
	p_Val->minor = 0;
	p_Val->patch = 0;
	p_Val->label[0] = 0;
	p_Val->sha[0] = 0;
	p_Val->tag[0] = 0;
}

void ManusTypeInitializer::FirmwareVersion_Init(FirmwareVersion* p_Val)
{
	p_Val->version = 0;
	ManusTimestamp_Init(&p_Val->timestamp);
}

void ManusTypeInitializer::ManusVersion_Init(ManusVersion* p_Val)
{
	p_Val->versionInfo[0] = 0;
}

void ManusTypeInitializer::TrackerId_Init(TrackerId* p_Val)
{
	p_Val->id[0] = 0;
}

void ManusTypeInitializer::TrackerData_Init(TrackerData* p_Val)
{
	ManusTimestamp_Init(&p_Val->lastUpdateTime);
	TrackerId_Init(&(p_Val->trackerId));
	p_Val->userId = 0;
	p_Val->isHmd = false;
	p_Val->trackerType = TrackerType::TrackerType_Unknown;

	ManusQuaternion_Init(&(p_Val->rotation));
	ManusVec3_Init(&(p_Val->position));
	p_Val->quality = TrackingQuality::TrackingQuality_Untrackable;
}

void ManusTypeInitializer::ManusHost_Init(ManusHost* p_Val)
{
	p_Val->hostName[0] = 0;
	p_Val->ipAddress[0] = 0;
}

void ManusTypeInitializer::RawSkeletonInfo_Init(RawSkeletonInfo* p_Val)
{
    p_Val->gloveId = 0;
    p_Val->nodesCount = 0;
    ManusTimestamp_Init(&p_Val->publishTime);
}

void ManusTypeInitializer::SkeletonNode_Init(SkeletonNode* p_Val)
{
	p_Val->id = 0;
	ManusTransform_Init(&(p_Val->transform));
}

void ManusTypeInitializer::SkeletonInfo_Init(SkeletonInfo* p_Val)
{
	p_Val->id = 0;
	p_Val->nodesCount = 0;
}

void ManusTypeInitializer::SkeletonStreamInfo_Init(SkeletonStreamInfo* p_Val)
{
	ManusTimestamp_Init(&p_Val->publishTime);
	p_Val->skeletonsCount = 0;
}

void ManusTypeInitializer::ErgonomicsData_Init(ErgonomicsData* p_Val)
{
	p_Val->id = 0;
	p_Val->isUserID = false;
	for (size_t i = 0; i < ErgonomicsDataType_MAX_SIZE; i++)
	{
		p_Val->data[i] = 0.0f;
	}
}

void ManusTypeInitializer::ErgonomicsStream_Init(ErgonomicsStream* p_Val)
{
	ManusTimestamp_Init(&p_Val->publishTime);
	for (size_t i = 0; i < MAX_NUMBER_OF_ERGONOMICS_DATA; i++)
	{
		ErgonomicsData_Init(&(p_Val->data[i]));
	}
	p_Val->dataCount = 0;
}


void ManusTypeInitializer::DongleLandscapeData_Init(DongleLandscapeData* p_Val)
{
	p_Val->id = 0;
	p_Val->classType = DeviceClassType::DeviceClassType_Unknown;
	p_Val->familyType = DeviceFamilyType::DeviceFamilyType_Unknown;
	p_Val->isHaptics = false;

	Version_Init(&(p_Val->hardwareVersion));
	Version_Init(&(p_Val->firmwareVersion));
	ManusTimestamp_Init(&p_Val->firmwareTimestamp);

	p_Val->chargingState = 0;

	p_Val->channel = 0;

	p_Val->updateStatus = UpdateStatusEnum::UpdateStatusEnum_Unknown;

	p_Val->licenseType[0]=0;

	ManusTimestamp_Init(&p_Val->lastSeen);

	p_Val->leftGloveID = 0;
	p_Val->rightGloveID = 0;

	p_Val->licenseLevel = LicenseType::LicenseType_Undefined;
}

void ManusTypeInitializer::GloveLandscapeData_Init(GloveLandscapeData* p_Val)
{
	p_Val->id = 0;
	p_Val->classType = DeviceClassType::DeviceClassType_Unknown;
	p_Val->familyType = DeviceFamilyType::DeviceFamilyType_Unknown;
	p_Val->side = Side::Side_Invalid;
	p_Val->isHaptics = false;

	p_Val->pairedState = DevicePairedState::DevicePairedState_Unknown;
	p_Val->dongleID = 0;

	Version_Init(&(p_Val->hardwareVersion));
	Version_Init(&(p_Val->firmwareVersion));
	ManusTimestamp_Init(&p_Val->firmwareTimestamp);

	p_Val->updateStatus = UpdateStatusEnum::UpdateStatusEnum_Unknown;

	p_Val->batteryPercentage = 0;
	p_Val->transmissionStrength = 0;

	for (size_t i = 0; i < MAX_NUM_IMUS_ON_GLOVE; i++)
	{
		IMUCalibrationInfo_Init(&(p_Val->iMUCalibrationInfo[i]));
	}

	ManusTimestamp_Init(&p_Val->lastSeen);
}

void ManusTypeInitializer::Measurement_Init(Measurement* p_Val)
{
	p_Val->entryType = MeasurementType::MeasurementType_Unknown;
	p_Val->value = 0.0f;

	p_Val->unit = MeasurementUnit::MeasurementUnit_Meters;
	p_Val->category = MeasurementCategory::MeasurementCategory_Misc;
	p_Val->displayName[0] = 0;
}

void ManusTypeInitializer::TrackerOffset_Init(TrackerOffset* p_Val)
{
	p_Val->entryType = TrackerOffsetType::TrackerOffsetType_Unknown;
	ManusVec3_Init(&(p_Val->translation));
	ManusQuaternion_Init(&(p_Val->rotation));
}

void ManusTypeInitializer::ExtraTrackerOffset_Init(ExtraTrackerOffset* p_Val)
{
	p_Val->entryType = ExtraTrackerOffsetType::ExtraTrackerOffsetType_Unknown;
	p_Val->value = 0.0f;
}

void ManusTypeInitializer::TrackerLandscapeData_Init(TrackerLandscapeData* p_Val)
{
	p_Val->id[0] = 0;
	p_Val->type = TrackerType::TrackerType_Unknown;
	p_Val->systemType = TrackerSystemType::TrackerSystemType_Unknown;
	p_Val->user = 0;
	p_Val->isHMD = false;
}

void ManusTypeInitializer::UserProfileLandscapeData_Init(UserProfileLandscapeData* p_Val)
{
	p_Val->profileType = ProfileType::ProfileType_Hands;
	for (size_t i = 0; i < MeasurementType::MeasurementType_MAX_SIZE; i++)
	{
		Measurement_Init(&(p_Val->measurements[i]));
	}
	for (size_t i = 0; i < TrackerOffsetType::TrackerOffsetType_MAX_SIZE; i++)
	{
		TrackerOffset_Init(&(p_Val->trackerOffsets[i]));
	}
	for (size_t i = 0; i < ExtraTrackerOffsetType::ExtraTrackerOffsetType_MAX_SIZE; i++)
	{
		ExtraTrackerOffset_Init(&(p_Val->extraTrackerOffsets[i]));
	}
}

void ManusTypeInitializer::UserLandscapeData_Init(UserLandscapeData* p_Val)
{
	p_Val->id = 0;

    p_Val->name[0] = 0;
    Color_Init(&(p_Val->color));

    p_Val->dongleID = 0;
	p_Val->leftGloveID = 0;
	p_Val->rightGloveID = 0;
	UserProfileLandscapeData_Init(&(p_Val->profile));
	p_Val->userIndex = 0;
}

void ManusTypeInitializer::SkeletonLandscapeData_Init(SkeletonLandscapeData* p_Val)
{
	p_Val->id = 0;
	p_Val->session[0] = 0;
    p_Val->userId = 0;
	p_Val->type = SkeletonType::SkeletonType_Invalid;
	p_Val->rootBoneName[0] = 0;
	p_Val->scaled = false;
}

void ManusTypeInitializer::DeviceLandscape_Init(DeviceLandscape* p_Val)
{
	for (size_t i = 0; i < MAX_NUMBER_OF_DONGLES; i++)
	{
		DongleLandscapeData_Init(&(p_Val->dongles[i]));
	}
	p_Val->dongleCount = 0;
	for (size_t i = 0; i < MAX_NUMBER_OF_GLOVES; i++)
	{
		GloveLandscapeData_Init(&(p_Val->gloves[i]));
	}
	p_Val->gloveCount = 0;
}

void ManusTypeInitializer::UserLandscape_Init(UserLandscape* p_Val)
{
	for (size_t i = 0; i < MAX_USERS; i++)
	{
		UserLandscapeData_Init(&(p_Val->users[i]));
	}
	p_Val->userCount = 0;
}

void ManusTypeInitializer::SkeletonLandscape_Init(SkeletonLandscape* p_Val)
{
	for (size_t i = 0; i < MAX_NUMBER_OF_SKELETONS; i++)
	{
		SkeletonLandscapeData_Init(&(p_Val->skeletons[i]));
	}
	p_Val->skeletonCount = 0;
}

void ManusTypeInitializer::TrackerLandscape_Init(TrackerLandscape* p_Val)
{
	for (size_t i = 0; i < MAX_NUMBER_OF_TRACKERS; i++)
	{
		TrackerLandscapeData_Init(&(p_Val->trackers[i]));
	}
	p_Val->trackerCount = 0;
}

void ManusTypeInitializer::LicenseInfo_Init(LicenseInfo* p_Val)
{
    p_Val->sdkData = false;
    p_Val->recordingAndPlayback = false;
    p_Val->timeCode = false;
    p_Val->unlimitedGloves = false;
    p_Val->ergonomicData = false;

    p_Val->xsensSession = false;
    p_Val->optitrackSession = false;
    p_Val->unitySession = false;
    p_Val->unrealSession = false;
    p_Val->mobuSession = false;
    p_Val->qualisysSession = false;
    p_Val->vredSession = false;
}

void ManusTypeInitializer::SettingsLandscape_Init(SettingsLandscape* p_Val)
{
	Version_Init(&(p_Val->manusCoreVersion));
	LicenseInfo_Init(&(p_Val->license));
	p_Val->playbackMode = false;
	p_Val->ignoreSessionTimeOuts = false;

	FirmwareVersion_Init(&(p_Val->firmwareOne));
	FirmwareVersion_Init(&(p_Val->firmwareTwo));
}

void ManusTypeInitializer::TimecodeInterface_Init(TimecodeInterface* p_Val)
{
	p_Val->name[0] = 0;
	p_Val->api[0] = 0;
	p_Val->index = -1;
}

void ManusTypeInitializer::TimeLandscape_Init(TimeLandscape* p_Val)
{
	for (size_t i = 0; i < MAX_NUMBER_OF_AUDIO_INTERFACES; i++)
	{
		TimecodeInterface_Init(&p_Val->interfaces[i]);
	}
	p_Val->interfaceCount = 0;
	TimecodeInterface_Init(&p_Val->currentInterface);
	p_Val->fps = TimecodeFPS::TimecodeFPS_Undefined;
	p_Val->fakeTimecode = false;
	p_Val->useSyncPulse = false;
	p_Val->deviceKeepAlive = false;
	p_Val->syncStatus = false;
	p_Val->timecodeStatus = false;
}

void ManusTypeInitializer::GestureLandscapeData_Init(GestureLandscapeData* p_Val)
{
    p_Val->id = 0;
    p_Val->name[0] = 0;
}

void ManusTypeInitializer::Landscape_Init(Landscape* p_Val)
{
	DeviceLandscape_Init(&(p_Val->gloveDevices));
	UserLandscape_Init(&(p_Val->users));
	SkeletonLandscape_Init(&(p_Val->skeletons));
	TrackerLandscape_Init(&(p_Val->trackers));
	SettingsLandscape_Init(&(p_Val->settings));
    TimeLandscape_Init(&(p_Val->time));
    p_Val->gestureCount = 0;
}


void ManusTypeInitializer::SkeletonSetupArraySizes_Init(SkeletonSetupArraySizes* p_Val)
{
	p_Val->nodesCount = 0;
	p_Val->chainsCount = 0;
    p_Val->collidersCount = 0;
    p_Val->meshCount = 0;
}

void ManusTypeInitializer::NodeSettingsIK_Init(NodeSettingsIK* p_Val)
{
	p_Val->ikAim = 0.0f;
}

void ManusTypeInitializer::NodeSettingsFoot_Init(NodeSettingsFoot* p_Val)
{
	p_Val->heightFromGround = 0.0f;
}

void ManusTypeInitializer::NodeSettingsRotationOffset_Init(NodeSettingsRotationOffset* p_Val)
{
	ManusQuaternion_Init(&(p_Val->value));
}

void ManusTypeInitializer::NodeSettingsLeaf_Init(NodeSettingsLeaf* p_Val)
{
	ManusVec3_Init(&(p_Val->direction));
	p_Val->length = 0.0f;
}

void ManusTypeInitializer::NodeSettings_Init(NodeSettings* p_Val)
{
	p_Val->usedSettings = NodeSettingsFlag::NodeSettingsFlag_None;
	NodeSettingsIK_Init(&(p_Val->ik));
	NodeSettingsFoot_Init(&(p_Val->foot));
	NodeSettingsRotationOffset_Init(&(p_Val->rotationOffset));
	NodeSettingsLeaf_Init(&(p_Val->leaf));
}

void ManusTypeInitializer::NodeSetup_Init(NodeSetup* p_Val)
{
	p_Val->id = 0;
	p_Val->name[0] = 0;
	p_Val->type = NodeType::NodeType_Invalid;
	ManusTransform_Init(&(p_Val->transform));
	p_Val->parentID = 0;
	NodeSettings_Init(&(p_Val->settings));
}


void ManusTypeInitializer::ChainSettingsPelvis_Init(ChainSettingsPelvis* p_Val)
{
	p_Val->hipHeight = 0.0f;
	p_Val->hipBendOffset = 0.0f;
	p_Val->thicknessMultiplier = 1.0f;
}

void ManusTypeInitializer::ChainSettingsLeg_Init(ChainSettingsLeg* p_Val)
{
	p_Val->reverseKneeDirection = false;
	p_Val->kneeRotationOffset = 0.0f;
	p_Val->footForwardOffset = 0.0f;
	p_Val->footSideOffset = 0.0f;
}

void ManusTypeInitializer::ChainSettingsSpine_Init(ChainSettingsSpine* p_Val)
{
	p_Val->spineBendOffset = 0.0f;
}

void ManusTypeInitializer::ChainSettingsNeck_Init(ChainSettingsNeck* p_Val)
{
	p_Val->neckBendOffset = 0.0f;
}

void ManusTypeInitializer::ChainSettingsHead_Init(ChainSettingsHead* p_Val)
{
	p_Val->headPitchOffset = 0.0f;
	p_Val->headYawOffset = 0.0f;
	p_Val->headTiltOffset = 0.0f;
	p_Val->useLeafAtEnd = false;
}

void ManusTypeInitializer::ChainSettingsArm_Init(ChainSettingsArm* p_Val)
{
	p_Val->armLengthMultiplier = 0.0f;
	p_Val->elbowRotationOffset = 0.0f;

	ManusVec3_Init(&(p_Val->armRotationOffset));

	ManusVec3_Init(&(p_Val->positionMultiplier));
	ManusVec3_Init(&(p_Val->positionOffset));
}

void ManusTypeInitializer::ChainSettingsShoulder_Init(ChainSettingsShoulder* p_Val)
{
	p_Val->forwardOffset = 0.0f;
	p_Val->shrugOffset = 0.0f;

	p_Val->forwardMultiplier = 0.0f;
	p_Val->shrugMultiplier = 0.0f;
}

void ManusTypeInitializer::ChainSettingsFinger_Init(ChainSettingsFinger* p_Val)
{
	p_Val->useLeafAtEnd = false;
	p_Val->metacarpalBoneId = 0;
	p_Val->handChainId = 0;
}

void ManusTypeInitializer::ChainSettingsHand_Init(ChainSettingsHand* p_Val)
{
	for (size_t i = 0; i < MAX_NUM_FINGER_IDS; i++)
	{
		p_Val->fingerChainIds[i] = 0;
	}
	p_Val->fingerChainIdsUsed = 0;
	p_Val->handMotion = HandMotion::HandMotion_None;
}

void ManusTypeInitializer::ChainSettingsFoot_Init(ChainSettingsFoot* p_Val)
{
	for (size_t i = 0; i < MAX_NUM_TOE_IDS; i++)
	{
		p_Val->toeChainIds[i] = 0;
	}
	p_Val->toeChainIdsUsed = 0;
}

void ManusTypeInitializer::ChainSettingsToe_Init(ChainSettingsToe* p_Val)
{
	p_Val->footChainId = 0;
	p_Val->useLeafAtEnd = false;
}

void ManusTypeInitializer::ChainSettings_Init(ChainSettings* p_Val)
{
	p_Val->usedSettings = ChainType::ChainType_Invalid;
	ChainSettingsPelvis_Init(&(p_Val->pelvis));
	ChainSettingsLeg_Init(&(p_Val->leg));
	ChainSettingsSpine_Init(&(p_Val->spine));
	ChainSettingsNeck_Init(&(p_Val->neck));
	ChainSettingsHead_Init(&(p_Val->head));
	ChainSettingsArm_Init(&(p_Val->arm));
	ChainSettingsShoulder_Init(&(p_Val->shoulder));
	ChainSettingsFinger_Init(&(p_Val->finger));
	ChainSettingsHand_Init(&(p_Val->hand));
	ChainSettingsFoot_Init(&(p_Val->foot));
	ChainSettingsToe_Init(&(p_Val->toe));
}

void ManusTypeInitializer::ChainSetup_Init(ChainSetup* p_Val)
{
	p_Val->id = 0;
	p_Val->type = ChainType::ChainType_Invalid;
	p_Val->dataType = ChainType::ChainType_Invalid;
	p_Val->dataIndex = 0;
	p_Val->nodeIdCount = 0;
	for (size_t i = 0; i < MAX_CHAIN_LENGTH; i++)
	{
		p_Val->nodeIds[i] = 0;
	}
	ChainSettings_Init(&(p_Val->settings));
	p_Val->side = Side::Side_Invalid;
}

void ManusTypeInitializer::SphereColliderSetup_Init(SphereColliderSetup* p_Val)
{
    p_Val->radius = 0;
}

void ManusTypeInitializer::CapsuleColliderSetup_Init(CapsuleColliderSetup* p_Val)
{
    p_Val->radius = 0;
    p_Val->length = 0;
}

void ManusTypeInitializer::BoxColliderSetup_Init(BoxColliderSetup* p_Val)
{
    ManusVec3_Init(&(p_Val->size));
}

void ManusTypeInitializer::ColliderSetup_Init(ColliderSetup* p_Val)
{
    p_Val->nodeID = 0;
    ManusVec3_Init(&(p_Val->localPosition));
    ManusVec3_Init(&(p_Val->localRotation));
    p_Val->type = ColliderType::ColliderType_Invalid;
    SphereColliderSetup_Init(&(p_Val->sphere));
    CapsuleColliderSetup_Init(&(p_Val->capsule));
    BoxColliderSetup_Init(&(p_Val->box));
}

void ManusTypeInitializer::Weight_Init(Weight* p_Val)
{
    p_Val->nodeID = 0;
    p_Val->weightValue = 0;
}

void ManusTypeInitializer::Vertex_Init(Vertex* p_Val)
{
    ManusVec3_Init(&(p_Val->position));
    p_Val->weightsCount = 0;
    for (size_t i = 0; i < MAX_BONE_WEIGHTS_PER_VERTEX; i++)
    {
        Weight_Init(&(p_Val->weights[i]));
    }
}

void ManusTypeInitializer::Triangle_Init(Triangle* p_Val)
{
    p_Val->vertexIndex1 = 0;
    p_Val->vertexIndex2 = 0;
    p_Val->vertexIndex3 = 0;
}

void ManusTypeInitializer::SkeletonTargetUserData_Init(SkeletonTargetUserData* p_Val)
{
	p_Val->userID = 0;
}

void ManusTypeInitializer::SkeletonTargetUserIndexData_Init(SkeletonTargetUserIndexData* p_Val)
{
	p_Val->userIndex = 0;
}

void ManusTypeInitializer::SkeletonTargetAnimationData_Init(SkeletonTargetAnimationData* p_Val)
{
	for (size_t i = 0; i < MAX_NUM_CHARS_IN_TARGET_ID; i++)
	{
		p_Val->id[i] = 0;
	}
}

void ManusTypeInitializer::SkeletonTargetGloveData_Init(SkeletonTargetGloveData* p_Val)
{
	p_Val->gloveID = 0;
}

void ManusTypeInitializer::SkeletonSettings_Init(SkeletonSettings* p_Val)
{
    p_Val->scaleToTarget = false;
    p_Val->useEndPointApproximations = false;
    p_Val->collisionType = CollisionType::CollisionType_None;

    p_Val->targetType = SkeletonTargetType::SkeletonTargetType_Invalid;
    SkeletonTargetUserData_Init(&(p_Val->skeletonTargetUserData));
    SkeletonTargetUserIndexData_Init(&(p_Val->skeletonTargetUserIndexData));
    SkeletonTargetAnimationData_Init(&(p_Val->skeletonTargetAnimationData));
    SkeletonTargetGloveData_Init(&(p_Val->skeletonGloveData));
}


void ManusTypeInitializer::SkeletonSetupInfo_Init(SkeletonSetupInfo* p_Val)
{
	p_Val->id = 0;
	p_Val->type = SkeletonType::SkeletonType_Invalid;
	SkeletonSettings_Init(&(p_Val->settings));
	p_Val->name[0]=0;
}


void ManusTypeInitializer::TemporarySkeletonInfo_Init(TemporarySkeletonInfo* p_Val)
{
	p_Val->name[0] = 0;
	p_Val->index = UINT32_MAX;
}

void ManusTypeInitializer::TemporarySkeletonsInfoForSession_Init(TemporarySkeletonsInfoForSession* p_Val)
{
	p_Val->sessionId = 0;
	p_Val->sessionName[0] = 0;
	p_Val->skeletonCount = 0;
	for (size_t i = 0; i < MAX_NUMBER_OF_SKELETONS_PER_SESSION; i++)
	{
		TemporarySkeletonInfo_Init(&(p_Val->skeletonInfo[i]));
	}
}

void ManusTypeInitializer::TemporarySkeletonCountForSession_Init(TemporarySkeletonCountForSession* p_Val)
{
	p_Val->sessionId = 0;
	p_Val->sessionName[ 0 ] = 0;
	p_Val->skeletonCount = 0;
}

void ManusTypeInitializer::TemporarySkeletonCountForAllSessions_Init(TemporarySkeletonCountForAllSessions* p_Val)
{
	p_Val->sessionsCount = 0;
	for (size_t i = 0; i < MAX_NUMBER_OF_SESSIONS; i++)
	{
		TemporarySkeletonCountForSession_Init(&(p_Val->temporarySkeletonCountForSessions[i]));
	}
}

void ManusTypeInitializer::TemporarySkeletonSessionsData_Init(TemporarySkeletonSessionsData* p_Val)
{
	p_Val->sessionsCount = 0;
	for (size_t i = 0; i < MAX_NUMBER_OF_SESSIONS; i++)
	{
		TemporarySkeletonsInfoForSession_Init(&(p_Val->temporarySkeletonsSessions[i]));
	}
}

void ManusTypeInitializer::SystemMessage_Init(SystemMessage* p_Val)
{
	p_Val->type = SystemMessageType::SystemMessageType_Unknown;
	p_Val->infoString[0]=0;
	p_Val->infoUInt = 0;
}

void ManusTypeInitializer::CoordinateSystemVUH_Init(CoordinateSystemVUH* p_Val)
{
	p_Val->view = AxisView::AxisView_Invalid;
	p_Val->up = AxisPolarity::AxisPolarity_Invalid;
	p_Val->handedness = Side::Side_Invalid;
	p_Val->unitScale = 1.0f;
}

void ManusTypeInitializer::CoordinateSystemDirection_Init(CoordinateSystemDirection* p_Val)
{
	p_Val->x = AxisDirection::AxisDirection_Invalid;
	p_Val->y = AxisDirection::AxisDirection_Invalid;
	p_Val->z = AxisDirection::AxisDirection_Invalid;
	p_Val->unitScale = 1.0f;
}

void ManusTypeInitializer::NodeInfo_Init(NodeInfo* p_Val)
{
    p_Val->nodeId = 0;
    p_Val->parentId = 0;
}

void ManusTypeInitializer::GestureStreamInfo_Init(GestureStreamInfo* p_Val)
{
    ManusTimestamp_Init(&p_Val->publishTime);
    p_Val->gestureProbabilitiesCount = 0;
}

void ManusTypeInitializer::GestureProbabilities_Init(GestureProbabilities* p_Val)
{
    p_Val->id = 0;
    p_Val->isUserID = false;
    p_Val->totalGestureCount = 0;
    for (uint32_t i = 0; i < MAX_GESTURE_DATA_CHUNK_SIZE; i++)
    {
        GestureProbability_Init(&p_Val->gestureData[i]);
    }
    p_Val->gestureCount = 0;
}

void ManusTypeInitializer::GestureProbability_Init(GestureProbability* p_Val)
{
    p_Val->id = 0;
    p_Val->percent = 0.0f;
}