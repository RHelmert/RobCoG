#ifndef H_CORESDKWRAPPERTYPES
#define H_CORESDKWRAPPERTYPES

// Set up a Doxygen group.
/** @addtogroup CoreSdkWrapper
 *  @{
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/******************************************************************************
 * Preprocessor defines.
 *****************************************************************************/

/// @brief Used to descriptively refer to the number of fingers on a hand.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define NUM_FINGERS_ON_HAND 5
/// @brief Used to descriptively refer to the number of phalanges in a finger.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define NUM_PHALANGES_IN_FINGER 3
/// @brief Used to descriptively refer to the number of flex sensor segments.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define NUM_FLEX_SEGMENTS_PER_FINGER 2
/// @brief Used to descriptively refer to the number of Polygon skeleton bones.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define NUM_BONES_IN_POLYGON_SKELETON (eBONETYPE_RIGHT_HAND + 1)
/// @brief Used to descriptively refer to the maximum IMUs count on a glove.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define MAX_NUM_IMUS_ON_GLOVE (NUM_FINGERS_ON_HAND + 1)
/// @brief Used to descriptively refer to the MCP joint in an array.
///
/// Used with arrays containing flex sensor data to refer to the location of
/// the metacarpophalangeal joint's data in a descriptive way. This is the
/// joint at the base of the finger.
#define FLEX_SENSOR_MCP 0
/// @brief Used to descriptively refer to the PIP joint in an array.
///
/// Used with arrays containing flex sensor data to refer to the location of
/// the proximal interphalangeal joint's data in a descriptive way. This is the
/// middle joint of a finger.
#define FLEX_SENSOR_PIP 1
/// @brief Used to descriptively refer to the maximum number of Polygon users.
///
/// Used with arrays and loops to make them more descriptive than simply using
/// the number, and to make changing the number easier and safer.
#define MAX_NUM_USERS 32
// Host name length is based on a post made here:
// https://community.cisco.com/t5/other-network-architecture/maximum-length-hostname/td-p/529327
// Which in turn was based on: https://www.ietf.org/rfc/rfc1035.txt
/// @brief Used to descriptively refer to the maximum host name length.
///
/// Used with arrays to make them more descriptive than simply using the
/// number, and to make changing the number easier and safer.
#define MAX_NUM_CHARS_IN_HOST_NAME 256
/// @brief Used to descriptively refer to the maximum IP address length.
///
/// Used with arrays to make them more descriptive than simply using the
/// number, and to make changing the number easier and safer.
/// It is based on the length of an IPv6 address. Example:
/// "2001:0db8:0000:0000:0000:8a2e:0370:7334".
#define MAX_NUM_CHARS_IN_IP_ADDRESS 40
/// @brief Used to descriptively refer to the maximum tracker name length.
///
/// Used with arrays to make them more descriptive than simply using the
/// number, and to make changing the number easier and safer.
#define MAX_NUM_CHARS_IN_TRACKER_ID 32
/// @brief Used to descriptively refer to the maximum target name length.
///
/// Used with arrays to make them more descriptive than simply using the
/// number, and to make changing the number easier and safer.
#define MAX_NUM_CHARS_IN_TARGET_ID 32
/// @brief Used to descriptively refer to the maximum version string length.
///
/// Used with arrays to make them more descriptive than simply using the
/// number, and to make changing the number easier and safer.
#define MAX_NUM_CHARS_IN_VERSION 16

/// @brief The value given to glove and dongle IDs when they are uninitialised.
#define UNINITIALISED_ID 0

/******************************************************************************
 * Enums.
 *****************************************************************************/

/// @brief The return values that can be given by SDK wrapper functions.
typedef enum
{
	/// No issues occurred.
	eCORESDKWRAPPER_SUCCESS,

	/// Something went wrong, but no specific reason can be given.
	eCORESDKWRAPPER_ERROR,

	/// One of the arguments given had an invalid value.
	eCORESDKWRAPPER_INVALID_ARGUMENT,

	/// The size of an argument given (e.g. an array) does not match the size
	/// of the data that it is intended to hold.
	eCORESDKWRAPPER_ARGUMENT_SIZE_MISMATCH,

	/// A string of an unsupported size was encountered.
	eCORESDKWRAPPER_UNSUPPORTED_STRING_SIZE_ENCOUNTERED,

	/// The Core SDK is not available.
	eCORESDKWRAPPER_SDK_NOT_AVAILABLE,

	/// The network host finder is not available.
	eCORESDKWRAPPER_HOST_FINDER_NOT_AVAILABLE,

	/// The data requested is not available.
	eCORESDKWRAPPER_DATA_NOT_AVAILABLE,

	/// Failed to allocate memory for something.
	eCORESDKWRAPPER_MEMORY_ERROR,

	/// Something went wrong in the SDK internally.
	eCORESDKWRAPPER_INTERNAL_ERROR,

	/// The function was not intended to be called at this time.
	eCORESDKWRAPPER_FUNCTION_CALLED_AT_WRONG_TIME,

	/// No connection to Core was made.
	eCORESDKWRAPPER_NOT_CONNECTED,

	/// The connection with Core timed out.
	eCORESDKWRAPPER_CONNECTION_TIMEOUT
} WrapperReturnCode;

/// @brief The type used for the return values of SDK wrapper functions.
///
/// This typedef is used as a return value instead of enum values to ensure a
/// known data size. When using enums the size is up to the compiler.
typedef uint32_t WrapperReturnCode_t;

/// @brief Used to tell what hand is being referred to.
typedef enum
{
	/// An invalid value used for uninitialised variables.
	eHANDTYPE_INVALID,

	/// Selects the left hand.
	eHANDTYPE_LEFT_HAND,

	/// Selects the right hand.
	eHANDTYPE_RIGHT_HAND
} HandType;

/// @brief Used to tell what client is using the wrapper.
///
/// This is used to select a mesh config preset when initialising the wrapper,
/// for example. Mesh configs are used to transform rotations and translations
/// so that they are correct for the selected application.
typedef enum
{
	/// An invalid value used for uninitialised variables.
	eCLIENTTYPE_INVALID,

	/// Make data work for MotionBuilder.
	eCLIENTTYPE_MOTIONBUILDER,

	/// Make data work for Unreal.
	eCLIENTTYPE_UNREAL,

	/// Make data work for VRED.
	eCLIENTTYPE_VRED
} ClientType;

/// @brief Types of gloves that data can be received from.
typedef enum
{
	/// An invalid value used for uninitialised variables.
	eGLOVETYPE_INVALID,

	/// Glove data coming from the legacy Apollo application, instead of being
	/// handled by Manus Core directly.
	eGLOVETYPE_APOLLO_LEGACY,

	/// A Prime One glove.
	eGLOVETYPE_PRIME_ONE,

	/// A Prime II glove.
	eGLOVETYPE_PRIME_TWO
} GloveType;

/// @brief Used when retargeting to select what the hand is relative to.
///
/// Used to select what the relative hand rotation and translation are relative
/// to.
typedef enum 
{
	/// Hand rotation and translation is relative to the skeleton's root.
	eHANDLOCALTO_ROOT,

	/// Hand rotation and translation is relative to the skeleton's hips.
	eHANDLOCALTO_HIPS,

	/// Hand rotation and translation is relative to the skeleton's spine.
	eHANDLOCALTO_SPINE,

	/// Hand rotation and translation is relative to the skeleton's arms.
	eHANDLOCALTO_ARMS
} HandLocalTo;

/// @brief All the bones in a skeleton that Polygon handles.
///
/// The enum values are also the position of the bone's data in arrays of bone
/// data received from Polygon.
typedef enum
{
	eBONETYPE_ROOT,
	eBONETYPE_HEAD,
	eBONETYPE_NECK,
	eBONETYPE_HIPS,
	eBONETYPE_SPINE,
	eBONETYPE_CHEST,
	eBONETYPE_UPPER_CHEST,
	eBONETYPE_LEFT_UPPER_LEG,
	eBONETYPE_RIGHT_UPPER_LEG,
	eBONETYPE_LEFT_LOWER_LEG,
	eBONETYPE_RIGHT_LOWER_LEG,
	eBONETYPE_LEFT_FOOT,
	eBONETYPE_RIGHT_FOOT,
	eBONETYPE_LEFT_TOES,
	eBONETYPE_RIGHT_TOES,
	eBONETYPE_LEFT_TOES_END,
	eBONETYPE_RIGHT_TOES_END,
	eBONETYPE_LEFT_SHOULDER,
	eBONETYPE_RIGHT_SHOULDER,
	eBONETYPE_LEFT_UPPER_ARM,
	eBONETYPE_RIGHT_UPPER_ARM,
	eBONETYPE_LEFT_LOWER_ARM,
	eBONETYPE_RIGHT_LOWER_ARM,
	eBONETYPE_LEFT_HAND,
	eBONETYPE_RIGHT_HAND
} BoneType;

/// @brief The different targets a Polygon skeleton can have.
typedef enum
{
	/// @brief An invalid value used for uninitialised variables.
	ePOLYGONTARGETCASE_NONE,

	/// @brief The Polygon skeleton will be generated from tracking data.
	ePOLYGONTARGETCASE_BODYESTIMATION,

	/// @brief Used to retarget on an existing skeleton.
	///
	/// The skeleton targets a user with trackers.
	ePOLYGONTARGETCASE_TARGETSKELETON
} PolygonTargetCase;

/// @brief The different types of trackers that can be used.
typedef enum
{
	eTRACKERTYPE_UNKNOWN,
	eTRACKERTYPE_HEAD,
	eTRACKERTYPE_WAIST,
	eTRACKERTYPE_LEFT_HAND,
	eTRACKERTYPE_RIGHT_HAND,
	eTRACKERTYPE_LEFT_FOOT,
	eTRACKERTYPE_RIGHT_FOOT,
	eTRACKERTYPE_LEFT_UPPER_ARM,
	eTRACKERTYPE_RIGHT_UPPER_ARM,
	eTRACKERTYPE_LEFT_UPPER_LEG,
	eTRACKERTYPE_RIGHT_UPPER_LEG,
	eTRACKERTYPE_CONTROLLER,
	eTRACKERTYPE_CAMERA
} TrackerType;

/// @brief Used instead of TrackerType enum values to ensure a known data size.
///
/// This typedef is used as instead of enum values to ensure a known data size.
/// When using enums the size is up to the compiler.
typedef uint32_t TrackerType_t;

/******************************************************************************
 * Structs.
 *****************************************************************************/

/// @brief A 3D vector, used for translations.
typedef struct ManusVec3
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
} ManusVec3;

/// @brief A quaternion, used for rotations.
typedef struct ManusQuaternion
{
	float w = 1.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
} ManusQuaternion;

/// @brief Stores a single version string.
typedef struct ManusVersion 
{
	char m_String[MAX_NUM_CHARS_IN_VERSION];
} ManusVersion;

/// @brief All the glove data that is sent at a lower rate than other data.
typedef struct LessFrequentlySentData
{
	/// @brief If the data in this struct can be used yet.
	///
	/// This data is received at a lower rate than other data.
	/// This value tracks if data has been received at least once.
	/// The struct's values are not valid if this is false.
	bool m_DataIsAvailable = false;

	/// @brief The current charge of the battery.
	uint32_t m_BatteryPercentage = 0;

	/// @brief The strength of the glove's signal. The lower, the better.
	int32_t m_TransmissionStrengthInDb = 0;
	GloveType m_GloveType = GloveType::eGLOVETYPE_INVALID;
} LessFrequentlySentData;

/// @brief Raw, unprocessed glove data.
typedef struct ManusRawData
{
	/// @brief Flex sensor values, from thumb to pinky.
	///
	/// Values from thumb to pinky, from MCP joint to (P)IP joint.
	float m_FlexSensor[NUM_FINGERS_ON_HAND][NUM_FLEX_SEGMENTS_PER_FINGER];

	/// @brief IMU rotations.
	///
	/// The first value is the IMU on the back of the hand.
	/// The rest of the values go from thumb to pinky, with only the thumb used
	/// for Prime One gloves.
	ManusQuaternion m_Imus[MAX_NUM_IMUS_ON_GLOVE];
} ManusRawData;

/// @brief Processed transform of the wrist computed from tracker data.
typedef struct ManusWristTransform
{
	/// @brief If the data in this struct can be used yet.
	bool m_DataIsAvailable;

	ManusVec3 m_Position;
	ManusQuaternion m_Orientation;
} ManusWristTransform;

/// @brief Processed glove data, based on the raw data.
typedef struct ManusProcessedData
{
	/// @brief Orientation of the wrist computed from the wrist IMU.
	ManusQuaternion m_WristImu;

	/// @brief Transform of the wrist computed from tracker data.
	///
	/// Processed for each User.
	ManusWristTransform m_WristTransforms[MAX_NUM_USERS];

	/// @brief Orientation of skeletal joints, from thumb to pinky.
	///
	/// Orientation for thumb[0], index[1], middle[2], ring[3], pinky[4] finger
	/// skeletal joints:
	/// CMC, MCP, IP on thumb; MCP, PIP, DIP on other fingers.
	ManusQuaternion m_Joints[NUM_FINGERS_ON_HAND][NUM_PHALANGES_IN_FINGER];

	/// @brief Finger bending values, from thumb to pinky.
	///
	/// For the first joint this is a blend between flex and IMU value, for
	/// gloves with IMUs on the fingers.
	/// Values go from thumb to pinky, from the CMC (thumb) or MCP (other
	/// fingers) joint to (D)IP joint.
	/// For the thumb, 0 means it is parallel to the hand, and 1 means it is
	/// pointing away from it to the side.
	/// For other fingers, with the palm of the hand facing down and looking at
	/// the hand from above, a stretch of -1 means the finger is stretched
	/// upward, and 1 means it is stretched down.
	float m_Stretch[NUM_FINGERS_ON_HAND][NUM_PHALANGES_IN_FINGER];
	/// @brief Finger stretch values in degrees, from thumb to pinky.
	float m_StretchDegrees[NUM_FINGERS_ON_HAND][NUM_PHALANGES_IN_FINGER];

	/// @brief Finger spreading values, from thumb to pinky.
	///
	/// The values are between 0 and 1 for the thumb, and normally between -1
	/// and 1 for other fingers.
	/// The thumb will never go outside this range, but other fingers can.
	/// For the thumb, 0 means it is parallel to the hand, and 1 means it is
	/// spread down, away from the palm.
	/// For other fingers, with the palm of the hand facing down and looking at
	/// the hand from above, a spread of -1 means a finger is spread to the
	/// left, and 1 means it is spread to the right.
	float m_Spread[NUM_FINGERS_ON_HAND];
	/// @brief Finger spreading values in degrees, from thumb to pinky.
	float m_SpreadDegrees[NUM_FINGERS_ON_HAND];
} ManusProcessedData;

/// @brief All data for a single glove.
typedef struct GloveData
{
	int64_t m_LastUpdateTime = 0;

	uint32_t m_GloveId = UNINITIALISED_ID;
	uint32_t m_DongleId = UNINITIALISED_ID;
	HandType m_HandType = HandType::eHANDTYPE_INVALID;

	LessFrequentlySentData m_LowFrequencyData;
	ManusRawData m_Raw;
	ManusProcessedData m_Processed;
} GloveData;

/// @brief Polygon retargeting settings.
typedef struct RetargetingSettings 
{
	// Hip
	/// @brief Compensate for skeleton size in movement speed.
	float m_MatchSameSpeed = 0.0f;
	/// @brief Change the height of the skeleton's hips.
	///
	/// A value below 1.0 will lower the hips. The knees will be bent to handle
	/// this if necessary.
	/// A value above 1.0 will raise the hips. The legs will be straightened
	/// to handle this if necessary. This can straighten the legs of characters
	/// that have bent knees by default.
	float m_HipHeightMultiplier = 1.0f;
	
	// Legs
	float m_LegWidth = 0.0f;
	float m_KneeRotation = 0.0f;

	// Arms
	float m_ShoulderForwardOffset = 0.0f;
	float m_ShoulderHeightOffset = 0.0f;

	float m_ShoulderForwardRotationMultiplier = 1.0f;
	float m_ShoulderHeightRotationMultiplier = 1.0f;

	float m_ElbowRotation = 0.0f;
	HandLocalTo m_HandLocalPosition = HandLocalTo::eHANDLOCALTO_ARMS;
	HandLocalTo m_HandLocalRotation = HandLocalTo::eHANDLOCALTO_ROOT;

	float m_ArmSpacing = 0.0f;
	float m_ArmLengthMultiplier = 1.0f;
	float m_ArmIK = 0.0f;
	bool m_HandRotationLocal = false;

	// HandIK
	float m_HandForwardOffset = 0.0f;
	float m_HandWidthOffset = 0.0f;
	float m_HandHeightOffset = 0.0f;

	float m_HandForwardMultiplier = 1.0f;
	float m_HandWidthMultiplier = 1.0f;
	float m_HandHeightMultiplier = 1.0f;

	// Spine
	float m_DefaultHipBend = 0.0f;
	float m_DefaultSpineBend = 0.0f;
	float m_DefaultNeckBend = 0.0f;

	float m_SpineBendMultiplier = 1.0f;
	float m_SpineAngleMultiplier = 1.0f;
	float m_SpineTwistMultiplier = 1.0f;
} RetargetingSettings;

/// @brief A wrapper for Polygon retargeting settings.
typedef struct RetargetingSettingsArgs
{
	uint32_t m_ID = 0;
	RetargetingSettings m_Settings;
} RetargetingSettingsArgs;

/// @brief Polygon data for a single bone.
typedef struct PolygonBone
{
	/// @brief Can be used to ignore some bones when sending Polygon data.
	///
	/// Use this to stop a bone from getting sent to Core when sending Polygon
	/// data.
	/// This is used for bone remapping in Unreal.
	bool m_ContainsValidBoneData;

	ManusQuaternion m_Rotation;
	ManusVec3 m_Position;
	float m_Scale;
} PolygonBone;

/// @brief Stores the name of a skeleton target.
typedef struct TargetSkeletonTarget
{
	char m_String[MAX_NUM_CHARS_IN_TARGET_ID];

} TargetSkeletonTarget;

/// @brief Stores the target to be used for body estimation.
typedef struct BodyEstimationTarget
{
	int32_t m_UserIndex;
} BodyEstimationTarget;

/// @brief Stores the target used for Polygon.
typedef struct PolygonTarget
{
	/// @brief The kind of targeting to use.
	PolygonTargetCase m_TargetCase;

	/// @brief The name, if any, of what is being targeted.
	///
	/// This only applies when targeting a skeleton.
	/// It can be the name of an animation that is being targeted, for
	/// example.
	TargetSkeletonTarget m_TargetSkeletonTarget;

	/// @brief The target for body estimation targeting.
	///
	/// This only applies when using body estimation targeting.
	BodyEstimationTarget m_BodyEstimationTarget;

} PolygonTarget;

/// @brief Contains everything needed to set a Polygon target.
typedef struct PolygonTargetArgs 
{
	uint32_t m_SkeletonID;
	PolygonTarget m_Target;
} PolygonTargetArgs;

/// @brief All the skeleton data that can be sent to or received from Polygon.
typedef struct PolygonSkeletonData
{
	int64_t m_LastUpdateTime = 0;

	/// @brief The Polygon user number this data belongs to.
	///
	/// The user index refers to the list of users shown in the dashboard under
	/// the Polygon tab. The first user shown is user index 0, the second is
	/// number 1, and so on. This is converted to the hexadecimal user ID
	/// internally.
	/// Note that the list of users can be edited from the dashboard, so it can
	/// change at any time.
	uint32_t m_UserIndex = 0;

	uint32_t m_SkeletonId = 0;

	PolygonBone m_Bones[NUM_BONES_IN_POLYGON_SKELETON];

	/// @brief Scale the skeleton to calibrated proportions for VR.
	///
	/// When scale to user is turned on Polygon will work for VR.
	/// The skeleton will be scaled to the calibrated proportions.
	bool m_ScaleToUser = false;

	/// @brief The height of the skeleton.
	float m_Height = 1.8f;

	/// @brief Polygon retargeting target.
	PolygonTarget m_Target;

} PolygonSkeletonData;

/// @brief Data required for the removal of a skeleton.
typedef struct RemoveSkeletonArgs
{
	uint32_t m_SkeletonID;
} RemoveSkeletonArgs;

/******************************************************************************
 * Tracking
 *****************************************************************************/

/// @brief Stores the name of a tracker.
typedef struct TrackerId
{
	char m_String[MAX_NUM_CHARS_IN_TRACKER_ID];
} TrackerId;

/// @brief All the tracker data that can be sent or received.
typedef struct TrackerData
{
	int64_t m_LastUpdateTime = 0;

	TrackerId m_TrackerId;

	/// @brief The Polygon user number this data belongs to.
	///
	/// The user index refers to the list of users shown in the dashboard under
	/// the Polygon tab. The first user shown is user index 0, the second is
	/// number 1, and so on. This is converted to the hexadecimal user ID
	/// internally.
	/// Note that the list of users can be edited from the dashboard, so it can
	/// change at any time.
	int32_t m_UserIndex = 0;

	bool m_IsHmd = false;
	TrackerType_t m_Type = TrackerType::eTRACKERTYPE_UNKNOWN;

	ManusQuaternion m_Rotation;
	ManusVec3 m_Position;
} TrackerData;

/// @brief Contains information for connecting to a host running Manus Core.
///
/// Note that if one of these values is blank, the other will be used when
/// connecting.
typedef struct ManusHost
{
	char m_HostName[MAX_NUM_CHARS_IN_HOST_NAME];
	char m_IpAddress[MAX_NUM_CHARS_IN_IP_ADDRESS];
} ManusHost;

// Callbacks
typedef void(*ConnectedToCoreCallback_t)(void);
typedef void(*DisconnectedFromCoreCallback_t)(void);

#ifdef __cplusplus
} // extern "C"
#endif

// Close the Doxygen group.
/** @} */

#endif // #ifndef H_CORESDKWRAPPERTYPES
