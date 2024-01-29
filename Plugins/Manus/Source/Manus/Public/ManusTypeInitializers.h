#ifndef _MANUS_TYPE_INITIALIZERS_H_
#define _MANUS_TYPE_INITIALIZERS_H_

#include "ManusSdkTypes.h"

struct MANUS_API ManusTypeInitializer
{
//public:
	///@brief All these initializers take a preallocated instance of their related structure and set default values.

	/// @brief Initializer for a ManusVec3 struct
	/// @param p_Val 
	static void ManusVec3_Init(ManusVec3* p_Val);

	/// @brief Initializer for a ManusVec2 struct
	/// @param p_Val 
	static void ManusVec2_Init(ManusVec2* p_Val);

	/// @brief Initializer for a ManusQuaternion struct
	/// @param p_Val 
	static void ManusQuaternion_Init(ManusQuaternion* p_Val);

	/// @brief Initializer for a ManusTransform struct
	/// @param p_Val 
	static void ManusTransform_Init(ManusTransform* p_Val);

    /// @brief Initializer for a Color struct
    /// @param p_Val 
    static void Color_Init(Color* p_Val);

    /// @brief Initializer for a ManusTimestampInfo struct
    /// @param p_Val 
    static void ManusTimestampInfo_Init(ManusTimestampInfo* p_Val);

    /// @brief Initializer for a ManusTimestamp struct
    /// @param p_Val 
    static void ManusTimestamp_Init(ManusTimestamp* p_Val);

	/// @brief Initializer for a IMUCalibrationInfo struct
	/// @param p_Val 
	static void IMUCalibrationInfo_Init(IMUCalibrationInfo* p_Val);

	/// @brief Initializer for a Version struct
	/// @param p_Val 
	static void Version_Init(Version* p_Val);

	/// @brief Initializer for a FirmwareVersion struct
	/// @param p_Val 
	static void FirmwareVersion_Init(FirmwareVersion* p_Val);


	/// @brief Initializer for a ManusVersion struct
	/// @param p_Val 
	static void ManusVersion_Init(ManusVersion* p_Val);

	/// @brief Initializer for a TrackerId struct
	/// @param p_Val 
	static void TrackerId_Init(TrackerId* p_Val);

	/// @brief Initializer for a TrackerData struct
	/// @param p_Val 
	static void TrackerData_Init(TrackerData* p_Val);

	/// @brief Initializer for a ManusHost struct
	/// @param p_Val 
	static void ManusHost_Init(ManusHost* p_Val);

	/// @brief Initializer for a SkeletonNode struct
	/// @param p_Val 
	static void SkeletonNode_Init(SkeletonNode* p_Val);

	/// @brief Initializer for a SkeletonInfo struct
	/// @param p_Val 
	static void SkeletonInfo_Init(SkeletonInfo* p_Val);

	/// @brief Initializer for a SkeletonStreamInfo struct
	/// @param p_Val 
	static void SkeletonStreamInfo_Init(SkeletonStreamInfo* p_Val);

	/// @brief Initializer for a ErgonomicsData struct
	/// @param p_Val 
	static void ErgonomicsData_Init(ErgonomicsData* p_Val);

	/// @brief Initializer for a ErgonomicsStream struct
	/// @param p_Val 
	static void ErgonomicsStream_Init(ErgonomicsStream* p_Val);

	/// @brief Initializer for a DongleLandscapeData struct
	/// @param p_Val 
	static void DongleLandscapeData_Init(DongleLandscapeData* p_Val);

	/// @brief Initializer for a GloveLandscapeData struct
	/// @param p_Val 
	static void GloveLandscapeData_Init(GloveLandscapeData* p_Val);

	/// @brief Initializer for a Measurement struct
	/// @param p_Val 
	static void Measurement_Init(Measurement* p_Val);

    /// @brief Initializer for a RawSkeletonInfo struct
    /// @param p_Val 
    static void RawSkeletonInfo_Init(RawSkeletonInfo* p_Val);

	/// @brief Initializer for a TrackerOffset struct
	/// @param p_Val 
	static void TrackerOffset_Init(TrackerOffset* p_Val);

	/// @brief Initializer for a ExtraTrackerOffset struct
	/// @param p_Val 
	static void ExtraTrackerOffset_Init(ExtraTrackerOffset* p_Val);

	/// @brief Initializer for a TrackerLandscapeData struct
	/// @param p_Val 
	static void TrackerLandscapeData_Init(TrackerLandscapeData* p_Val);

	/// @brief Initializer for a UserProfileLandscapeData struct
	/// @param p_Val 
	static void UserProfileLandscapeData_Init(UserProfileLandscapeData* p_Val);

	/// @brief Initializer for a UserLandscapeData struct
	/// @param p_Val 
	static void UserLandscapeData_Init(UserLandscapeData* p_Val);

	/// @brief Initializer for a SkeletonLandscapeData struct
	/// @param p_Val 
	static void SkeletonLandscapeData_Init(SkeletonLandscapeData* p_Val);

	/// @brief Initializer for a DeviceLandscape struct
	/// @param p_Val 
	static void DeviceLandscape_Init(DeviceLandscape* p_Val);

	/// @brief Initializer for a UserLandscape struct
	/// @param p_Val 
	static void UserLandscape_Init(UserLandscape* p_Val);

	/// @brief Initializer for a SkeletonLandscape struct
	/// @param p_Val 
	static void SkeletonLandscape_Init(SkeletonLandscape* p_Val);

	/// @brief Initializer for a TrackerLandscape struct
	/// @param p_Val 
	static void TrackerLandscape_Init(TrackerLandscape* p_Val);

    /// @brief Initializer for a GestureLandscapeData struct
    /// @param p_Val 
    static void GestureLandscapeData_Init(GestureLandscapeData* p_Val);

	/// @brief Initializer for a LicenseInfo struct
	/// @param p_Val 
	static void LicenseInfo_Init(LicenseInfo* p_Val);

	/// @brief Initializer for a SettingsLandscape struct
	/// @param p_Val 
	static void SettingsLandscape_Init(SettingsLandscape* p_Val);

	/// @brief Initializer for a TimecodeInterface struct
	/// @param p_Val 
	static void TimecodeInterface_Init(TimecodeInterface* p_Val);

	/// @brief Initializer for a TimeLandscape struct
	/// @param p_Val 
	static void TimeLandscape_Init(TimeLandscape* p_Val);

	/// @brief Initializer for a Landscape struct
	/// @param p_Val 
	static void Landscape_Init(Landscape* p_Val);

	/// @brief Initializer for a SkeletonSetupArraySizes struct
	/// @param p_Val 
	static void SkeletonSetupArraySizes_Init(SkeletonSetupArraySizes* p_Val);

	/// @brief Initializer for a NodeSettingsIK struct
	/// @param p_Val 
	static void NodeSettingsIK_Init(NodeSettingsIK* p_Val);

	/// @brief Initializer for a NodeSettingsFoot struct
	/// @param p_Val 
	static void NodeSettingsFoot_Init(NodeSettingsFoot* p_Val);

	/// @brief Initializer for a NodeSettingsRotationOffset struct
	/// @param p_Val 
	static void NodeSettingsRotationOffset_Init(NodeSettingsRotationOffset* p_Val);

	/// @brief Initializer for a NodeSettingsLeaf struct
	/// @param p_Val 
	static void NodeSettingsLeaf_Init(NodeSettingsLeaf* p_Val);

	/// @brief Initializer for a NodeSettings struct
	/// @param p_Val 
	static void NodeSettings_Init(NodeSettings* p_Val);

	/// @brief Initializer for a NodeSetup struct
	/// @param p_Val 
	static void NodeSetup_Init(NodeSetup* p_Val);

	/// @brief Initializer for a ChainSettingsPelvis struct
	/// @param p_Val 
	static void ChainSettingsPelvis_Init(ChainSettingsPelvis* p_Val);

	/// @brief Initializer for a ChainSettingsLeg struct
	/// @param p_Val 
	static void ChainSettingsLeg_Init(ChainSettingsLeg* p_Val);

	/// @brief Initializer for a ChainSettingsSpine struct
	/// @param p_Val 
	static void ChainSettingsSpine_Init(ChainSettingsSpine* p_Val);

	/// @brief Initializer for a ChainSettingsNeck struct
	/// @param p_Val 
	static void ChainSettingsNeck_Init(ChainSettingsNeck* p_Val);

	/// @brief Initializer for a ChainSettingsHead struct
	/// @param p_Val 
	static void ChainSettingsHead_Init(ChainSettingsHead* p_Val);

	/// @brief Initializer for a ChainSettingsArm struct
	/// @param p_Val 
	static void ChainSettingsArm_Init(ChainSettingsArm* p_Val);

	/// @brief Initializer for a ChainSettingsShoulder struct
	/// @param p_Val 
	static void ChainSettingsShoulder_Init(ChainSettingsShoulder* p_Val);

	/// @brief Initializer for a ChainSettingsFinger struct
	/// @param p_Val 
	static void ChainSettingsFinger_Init(ChainSettingsFinger* p_Val);

	/// @brief Initializer for a ChainSettingsHand struct
	/// @param p_Val 
	static void ChainSettingsHand_Init(ChainSettingsHand* p_Val);

	/// @brief Initializer for a ChainSettingsFoot struct
	/// @param p_Val 
	static void ChainSettingsFoot_Init(ChainSettingsFoot* p_Val);

	/// @brief Initializer for a ChainSettingsToe struct
	/// @param p_Val 
	static void ChainSettingsToe_Init(ChainSettingsToe* p_Val);

	/// @brief Initializer for a ChainSettings struct
	/// @param p_Val 
	static void ChainSettings_Init(ChainSettings* p_Val);

	/// @brief Initializer for a ChainSetup struct
	/// @param p_Val 
	static void ChainSetup_Init(ChainSetup* p_Val);

	/// @brief Initializer for a SkeletonTargetUserData struct
	/// @param p_Val 
	static void SkeletonTargetUserData_Init(SkeletonTargetUserData* p_Val);

	static void SkeletonTargetUserIndexData_Init(SkeletonTargetUserIndexData* p_Val);

	/// @brief Initializer for a SkeletonTargetAnimationData struct
	/// @param p_Val 
	static void SkeletonTargetAnimationData_Init(SkeletonTargetAnimationData* p_Val);

	/// @brief Initializer for a SkeletonTargetGloveData struct
	/// @param p_Val 
	static void SkeletonTargetGloveData_Init(SkeletonTargetGloveData* p_Val);

	/// @brief Initializer for a SkeletonSettings struct
	/// @param p_Val 
	static void SkeletonSettings_Init(SkeletonSettings* p_Val);

	/// @brief Initializer for a SkeletonSetupInfo struct
	/// @param p_Val 
	static void SkeletonSetupInfo_Init(SkeletonSetupInfo* p_Val);

	/// @brief Initializer for a TemporarySkeletonInfo struct
	/// @param p_Val 
	static void TemporarySkeletonInfo_Init(TemporarySkeletonInfo* p_Val);

    /// @brief Initializer for a SphereColliderSetup struct
    /// @param p_Val 
    static void SphereColliderSetup_Init(SphereColliderSetup* p_Val);

    /// @brief Initializer for a CapsuleColliderSetup struct
    /// @param p_Val 
    static void CapsuleColliderSetup_Init(CapsuleColliderSetup* p_Val);

    /// @brief Initializer for a BoxColliderSetup struct
    /// @param p_Val 
    static void BoxColliderSetup_Init(BoxColliderSetup* p_Val);

    /// @brief Initializer for a ColliderSetup struct
    /// @param p_Val 
    static void ColliderSetup_Init(ColliderSetup* p_Val);

    /// @brief Initializer for a Vertex Weight struct
    /// @param p_Val 
    static void Weight_Init(Weight* p_Val);

    /// @brief Initializer for a Vertex struct
    /// @param p_Val 
    static void Vertex_Init(Vertex* p_Val);

    /// @brief Initializer for a Triangle struct
    /// @param p_Val 
    static void Triangle_Init(Triangle* p_Val);

	/// @brief Initializer for a TemporarySkeletonsForSession struct
	/// @param p_Val 
	static void TemporarySkeletonsInfoForSession_Init(TemporarySkeletonsInfoForSession* p_Val);

	/// @brief Initializer for a TemporarySkeletonCountForSession struct
	/// @param p_Val 
	static void TemporarySkeletonCountForSession_Init(TemporarySkeletonCountForSession* p_Val);

	/// @brief Initializer for a TemporarySkeletonCountForSessions struct
	/// @param p_Val 
	static void TemporarySkeletonCountForAllSessions_Init(TemporarySkeletonCountForAllSessions* p_Val);

	/// @brief Initializer for a TemporarySkeletonSessionsData struct
	/// @param p_Val 
	static void TemporarySkeletonSessionsData_Init(TemporarySkeletonSessionsData* p_Val);

	/// @brief Initializer for a SystemMessage struct
	/// @param p_Val 
	static void SystemMessage_Init(SystemMessage* p_Val);

	/// @brief Initializer for a CoordinateSystemVUH struct
	/// @param p_Val 
	static void CoordinateSystemVUH_Init(CoordinateSystemVUH* p_Val);

	/// @brief Initializer for a CoordinateSystemDirection struct
	/// @param p_Val 
	static void CoordinateSystemDirection_Init(CoordinateSystemDirection* p_Val);

    /// @brief Initializer for a NodeInfo struct
    /// @param p_Val 
    static void NodeInfo_Init(NodeInfo* p_Val);

    /// @brief Initializer for a GestureStreamInfo struct
    /// @param p_Val 
    static void GestureStreamInfo_Init(GestureStreamInfo* p_Val);

    /// @brief Initializer for a GestureProbabilities struct
    /// @param p_Val 
    static void GestureProbabilities_Init(GestureProbabilities* p_Val);

    /// @brief Initializer for a GestureProbability struct
    /// @param p_Val 
    static void GestureProbability_Init(GestureProbability* p_Val);
};

#endif // #ifndef MANUS_TYPE_INITIALIZERS_H
