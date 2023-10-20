// Copyright 2015-2022 Manus

#pragma once


// Set up a Doxygen group.
/** @addtogroup ManusBlueprintTypes
 *  @{
 */


#include <Runtime/Launch/Resources/Version.h>
#ifdef MANUS_PLUGIN_USE_STEAMVR
	#include <SteamVRFunctionLibrary.h>
#endif
#include "ManusBlueprintTypes.generated.h"

////////////////////////////////////////////////////////////////////////////////
// enums

/// @brief Manus motion capture types.
UENUM(BlueprintType)
enum class EManusMotionCaptureType : uint8
{
	LeftHand,
	RightHand,
	BothHands,
	FullBody UMETA(DisplayName = "Full Body with Polygon"),
	Max UMETA(Hidden)
};

/// @brief An enum used to define the glove types.
UENUM(BlueprintType)
enum class EManusGloveType : uint8
{
	None,
	PrimeOne,
	PrimeTwo,
	PrimeX,
	PrimeMeta,
	Max UMETA(Hidden)
};

/// @brief An enum used to denote a left or a right hand.
UENUM(BlueprintType)
enum class EManusHandType : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right"),
	Max		UMETA(Hidden)
};


/// @brief An enum used to denote a left, right or center.
UENUM(BlueprintType)
enum class EManusSide : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right"),
	Center	UMETA(DisplayName = "Center"),
	Invalid	UMETA(DisplayName = "Invalid")
};

/// @brief enum used to define the different types of animation used for skeleton
UENUM(BlueprintType)
enum class EManusSkeletonType : uint8
{
	Invalid				UMETA(DisplayName = "Invalid"),
	Hand				UMETA(DisplayName = "Hand"),
	Body				UMETA(DisplayName = "Body"),
	Both				UMETA(DisplayName = "Both") // probably obsolete
};

/// @brief enum used to define the different types of animation used for skeleton
UENUM(BlueprintType)
enum class EManusSkeletonTargetType : uint8
{
	Invalid				UMETA(DisplayName = "Invalid"),
	UserData			UMETA(DisplayName = "UserData"),
	UserIndexData		UMETA(DisplayName = "UserIndexData"),
	AnimationData		UMETA(DisplayName = "AnimationData"),
	GloveData			UMETA(DisplayName = "GloveData")
};


/// @brief enum used to combine flags of which nodesettings are used
/// for handling flags correctly as uproperties see https://forums.unrealengine.com/t/bitmask-enum-declaration-in-4-12-whats-missing/361671/3
/// and the var UsedSettings we adjusted in this source code further below
UENUM(BlueprintType, meta = (Bitflags))
enum class EManusNodeSettingsFlag : uint8
{
	None = 0					UMETA(DisplayName = "None"),
	IK = 1 << 0					UMETA(DisplayName = "IK"),
	Foot = 1 << 1				UMETA(DisplayName = "Foot"),
	RotationOffset = 1 << 2		UMETA(DisplayName = "RotationOffset"),
	Leaf = 1 << 3				UMETA(DisplayName = "Leaf")
};
ENUM_CLASS_FLAGS(EManusNodeSettingsFlag)

/// @brief type of node in the skeleton
UENUM(BlueprintType)
enum class EManusNodeType : uint8
{
	Invalid				UMETA(DisplayName = "Invalid"),
	Joint				UMETA(DisplayName = "Joint"),
	Mesh				UMETA(DisplayName = "Mesh")
};

/// @brief type of chain in the skeleton
UENUM(BlueprintType)
enum class EManusChainType : uint8
{
	Invalid				UMETA(DisplayName = "Invalid"),
	Arm					UMETA(DisplayName = "Arm"),
	Leg					UMETA(DisplayName = "Leg"),
	Neck				UMETA(DisplayName = "Neck"),
	Spine				UMETA(DisplayName = "Spine"),
	FingerThumb			UMETA(DisplayName = "FingerThumb"),
	FingerIndex			UMETA(DisplayName = "FingerIndex"),
	FingerMiddle		UMETA(DisplayName = "FingerMiddle"),
	FingerRing			UMETA(DisplayName = "FingerRing"),
	FingerPinky			UMETA(DisplayName = "FingerPinky"),
	Pelvis				UMETA(DisplayName = "Pelvis"),
	Head				UMETA(DisplayName = "Head"),
	Shoulder			UMETA(DisplayName = "Shoulder"),
	Hand				UMETA(DisplayName = "Hand"),
	Foot				UMETA(DisplayName = "Foot"),
	Toe					UMETA(DisplayName = "Toe")
};


/// @brief type of hand motion used
UENUM(BlueprintType)
enum class EManusHandMotion : uint8
{
	None								UMETA(DisplayName = "None"),
	IMU									UMETA(DisplayName = "IMU"),
	Tracker								UMETA(DisplayName = "Tracker"),
	Tracker_RotationOnly				UMETA(DisplayName = "Tracker_RotationOnly"),
    Auto                                UMETA(DisplayName = "Auto"),
};

/// @brief An enum used as a return value for some Manus plugin functions.
/// It must match the WrapperReturnCode enum in ManusSdkTypes.h.
UENUM(BlueprintType)
enum class EManusRet : uint8
{
	Success UMETA(DisplayName = "Success"),
	Error UMETA(DisplayName = "Error"),
	InvalidArgument UMETA(DisplayName = "InvalidArgument"),
	ArgumentSizeMismatch UMETA(DisplayName = "ArgumentSizeMismatch"),
	UnsupportedStringSizeEncountered UMETA(DisplayName = "UnsupportedStringSizeEncountered"),
	SDKNotAvailable UMETA(DisplayName = "SDKNotAvailable"),
	HostFinderNotAvailable UMETA(DisplayName = "HostFinderNotAvailable"),
	DataNotAvailable UMETA(DisplayName = "DataNotAvailable"),
	MemoryError UMETA(DisplayName = "MemoryError"),
	InternalError UMETA(DisplayName = "InternalError"),
	FunctionCalledAtWrongTime UMETA(DisplayName = "FunctionCalledAtWrongTime"),
	NotConnected UMETA(DisplayName = "NotConnected"),
	ConnectionTimeout UMETA(DisplayName = "ConnectionTimeout"),
	NoHostFound UMETA(DisplayName = "NoHostFound")
};

/// @brief The parts of the body that can be tracked.
UENUM(BlueprintType)
enum class EManusTrackingBodyPart : uint8
{
	None UMETA(DisplayName = "None"),
	LeftHand UMETA(DisplayName = "LeftHand"),
	RightHand UMETA(DisplayName = "RightHand"),
	LeftFoot UMETA(DisplayName = "LeftFoot"),
	RightFoot UMETA(DisplayName = "RightFoot"),
	Head UMETA(DisplayName = "Head"),
	Waist UMETA(DisplayName = "Waist")
};

/// @brief A way to identify fingers by a name and number.
UENUM(BlueprintType)
enum class EManusFingerName : uint8
{
	Thumb  UMETA(DisplayName = "Thumb"),
	Index  UMETA(DisplayName = "Index"),
	Middle UMETA(DisplayName = "Middle"),
	Ring   UMETA(DisplayName = "Ring"),
	Pinky  UMETA(DisplayName = "Pinky"),
	Max UMETA(Hidden)
};

/// @brief The types of tracking devices that can be used.
UENUM(BlueprintType)
enum class EManusTrackingDeviceType : uint8
{
	Invalid,
	Hmd,                // Headset.
	Controller,
	GenericTracker
};

/// @brief All available data on a single skeleton bone.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusBone
{
	GENERATED_BODY()

    /// @brief The validity of the bone. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool Validity = false;

    /// @brief The transform of the bone. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FTransform Transform;

    /// @brief id of the bone.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 BoneId;
};

/// @brief All available data on a single skeleton.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusMetaSkeleton
{
	GENERATED_BODY()

    /// @brief The time it was last updated.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 LastUpdateTime;

    /// @brief A number that identifies the skeleton.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 SkeletonId;

    /// @brief Data on the bones.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FManusBone> Bones;
};

/// @brief All available data on a tracker.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusTracker
{
	GENERATED_BODY()
		
    /// @brief The time it was last updated.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 LastUpdateTime; // note . due to blueprints  uint64 is not supported. so some unsigned to signed shenanigans can pop up. be aware!

    /// @brief The ID of the Tracker.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString TrackerId;

    /// @brief The transform of the tracker.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FTransform Transform;

    /// @brief The User Index this Tracker is assigned to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int UserId;

    /// @brief The hand type this tracker is used for. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusHandType HandType;
};


/// @brief node settings for Inverse Kinematics
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSettingsIK
{
	GENERATED_BODY()
    
    /// @brief influences the direction of the joint angle. higher numbers adjust to forward, negative numbers to backwards.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float IkAim;
};

/// @brief node settings for the foot.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSettingsFoot
{
	GENERATED_BODY()
    
    /// @brief offset height from ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HeightFromGround;
};

/// @brief defines the rotation offset
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSettingsRotationOffset
{
	GENERATED_BODY()

    /// @brief rotation offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FQuat Value;
};

/// @brief Defines the node leaf settings 
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSettingsLeaf
{
	GENERATED_BODY()

    /// @brief the direction vector is defined with respect to the previous node in the chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FVector Direction;
    /// @brief length of the leaf.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float Length;
};

/// @brief The basic nodesettings for a node. multiple settings can be used at the same time.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSettings
{
	GENERATED_BODY()

	// for handling flags correctly as uproperties see https://forums.unrealengine.com/t/bitmask-enum-declaration-in-4-12-whats-missing/361671/3
	// and the enum EManusNodeSettingsFlag we adjusted here and above in this source code
    /// @brief bit flag of the used node settings. can contain multiple flags.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus", meta = (Bitmask, BitmaskEnum = "EManusNodeSettingsFlag"))
	int UsedSettings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusNodeSettingsIK Ik;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusNodeSettingsFoot Foot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusNodeSettingsRotationOffset RotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusNodeSettingsLeaf Leaf;
};

/// @brief The basic node from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusNodeSetup
{
	GENERATED_BODY()

    /// @brief Node ID
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 Id = 0;
    /// @brief Node Name (usually the bone)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Manus")
	FString Name;
    /// @brief type of node in the skeleton
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusNodeType Type = EManusNodeType::Invalid;
    /// @brief Base transform of the skeleton
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FTransform Transform;
    /// @brief Parent bone ID
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 ParentID = 0;
    /// @brief Node Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusNodeSettings Settings;
};


/// @brief The pelvis chain settings
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsPelvis
{
	GENERATED_BODY()

    /// @brief hip height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HipHeight = 0.0f;

    /// @brief hip bend offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HipBendOffset = 0.0f;

    /// @brief thickness multiplier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ThicknessMultiplier = 1.0f;
};


/// @brief The leet chain settings
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsLeg
{
	GENERATED_BODY()

    /// @brief reverse the knee direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool ReverseKneeDirection = false;

    /// @brief offset for knee rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float KneeRotationOffset = 0.0f;

    /// @brief offset foot forward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float FootForwardOffset = 0.0f;

    /// @brief offset foot to the side
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float FootSideOffset = 0.0f;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsSpine
{
	GENERATED_BODY()

    /// @brief offset for spine bend
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float SpineBendOffset = 0.0f;
} ;

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsNeck
{
	GENERATED_BODY()

    /// @brief offset for neck bend
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float NeckBendOffset = 0.0f;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsHead
{
	GENERATED_BODY()

    /// @brief head pitch offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HeadPitchOffset = 0.0f;

    /// @brief head yaw offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HeadYawOffset = 0.0f;

    /// @brief head tilt offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float HeadTiltOffset = 0.0f;

    /// @brief use the leaf at the end of the chain.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool UseLeafAtEnd = false;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsArm
{
	GENERATED_BODY()

    /// @brief arm length scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ArmLengthMultiplier = 0.0f;

    /// @brief elbow rotation offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ElbowRotationOffset = 0.0f;

    /// @brief arm rotation offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FVector ArmRotationOffset;

    /// @brief position scaler
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FVector PositionMultiplier;

    /// @brief position offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FVector PositionOffset;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsShoulder
{
	GENERATED_BODY()

    /// @brief should forward offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ForwardOffset = 0.0f;

    /// @brief shoulder shrug offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ShrugOffset = 0.0f;

    /// @brief shoulder forward scaler
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ForwardMultiplier = 0.0f;

    /// @brief shoulder shrug scaler
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ShrugMultiplier = 0.0f;
};


/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsFinger
{
	GENERATED_BODY()

    /// @brief use finger leaf at end
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool UseLeafAtEnd = false;

    /// @brief bone id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int MetacarpalBoneId = 0;

    /// @brief parent hand id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int HandChainId = 0;
};


/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsHand
{
	GENERATED_BODY()

    /// @brief finger chain id's connected to this finger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<int> FingerChainIds;

    /// @brief type of handmotion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusHandMotion HandMotion = EManusHandMotion::None;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingsFoot
{
	GENERATED_BODY()

    /// @brief toe Chain ID's connected tot his foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<int> ToeChainIds;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettingToe
{
	GENERATED_BODY()

    /// @brief parent foot id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int FootChainId = 0;

    /// @brief use the leaf at the end of the toe
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool UseLeafAtEnd = false;

};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSettings
{
	GENERATED_BODY()

    /// @brief which setting is used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusChainType UsedSettings;

    /// @brief pelvis settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsPelvis Pelvis;

    /// @brief leg settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsLeg Leg;

    /// @brief spine settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsSpine Spine;

    /// @brief neck settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsNeck Neck;

    /// @brief head settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsHead Head;

    /// @brief arm settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsArm Arm;

    /// @brief shoulder settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsShoulder Shoulder;

    /// @brief finger settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsFinger Finger;

    /// @brief hand settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsHand Hand;

    /// @brief foot settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingsFoot Foot;

    /// @brief toe settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettingToe Toe;
};

/// @brief The basic chain from which a skeleton is built
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusChainSetup
{
	GENERATED_BODY()

	/// @brief Chain ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 Id = 0;

    /// @brief chain type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusChainType Type = EManusChainType::Invalid;

    /// @brief chain data type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusChainType DataType = EManusChainType::Invalid;

    /// @brief data index
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int DataIndex = 0;

    /// @brief connected node id's
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<int> NodeIds;

    /// @brief settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusChainSettings Settings;

    /// @brief side of chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusSide Side = EManusSide::Invalid;
};


/// @brief Stores the information regarding the user data used to animate the skeleton.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusSkeletonTargetUserData
{
	GENERATED_BODY()

    /// @brief
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 userID; //default = 0;
};

/// @brief Stores the information regarding the user index data used to animate the skeleton.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusSkeletonTargetUserIndexData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 userIndex; //default = 0;
};

/// @brief Stores the information regarding the animation data used to animate the skeleton.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusSkeletonTargetAnimationData
{
	GENERATED_BODY()

    /// @brief
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString id;
};

/// @brief Stores the information regarding the glove data used to animate the skeleton.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusSkeletonTargetGloveData
{
	GENERATED_BODY()

    /// @brief
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 gloveID; //default = 0;
};


/// @brief All the skeleton setup that can be sent or received.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusSkeletonSetup
{
	GENERATED_BODY()

	/// @brief skeleton ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 Id = 0;

    /// @brief skeleton type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusSkeletonType Type = EManusSkeletonType::Invalid;

    /// @brief scale it to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool ScaleToTarget = false;

    /// @brief pinch correction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
    bool UseEndPointApproximations = true;


    /// @brief type of animation data is being targeted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusSkeletonTargetType TargetType;

    /// @brief user id for the skeleton to use for animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 SkeletonTargetUserId;

    /// @brief type of animation for skeleton
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString SkeletonTargetAnimationId;

    /// @brief glove id being used as input data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 SkeletonGloveId;

    /// @brief name of skeleton
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString Name;
};

/// @brief All the skeleton setup that can be sent or received.
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusErgonomicsData
{
	GENERATED_BODY()

	/// @brief skeleton ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 Id = 0;

    /// @brief skeleton ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool IsUserID = false;

    /// @brief skeleton ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<float> Data;

};

USTRUCT(Blueprintable, BlueprintType)
/// @brief Contains information about a gesture
struct MANUS_API FGestureLandscapeData
{
    GENERATED_BODY()

    /// @brief gesture ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
    int64 Id = 0;

    /// @brief gesture name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
    FString Name;
};

// Close the Doxygen group.
/** @} */
