// Copyright 2015-2020 Manus

#pragma once

#include <Runtime/Launch/Resources/Version.h>
#ifdef MANUS_PLUGIN_USE_STEAMVR
	#include <SteamVRFunctionLibrary.h>
#endif
#include "ManusBlueprintTypes.generated.h"



////////////////////////////////////////////////////////////////////////////////
// defines

#define HAND_LIVE_LINK_BONE_NUM 16
#define BODY_LIVE_LINK_BONE_NUM 25



////////////////////////////////////////////////////////////////////////////////
// enums

/**
 * Manus motion capture types.
 */
UENUM(BlueprintType)
enum class EManusMotionCaptureType : uint8
{
	LeftHand,
	RightHand,
	BothHands,
	FullBody UMETA(DisplayName = "Full Body with Polygon"),
	Max UMETA(Hidden)
};

/**
 * An enum used to define the glove types.
 */
UENUM(BlueprintType)
enum class EManusGloveType : uint8
{
	None,
	PrimeOne,
	PrimeTwo,
	Max UMETA(Hidden)
};

/**
 * An enum used to denote a left or a right hand.
 */
UENUM(BlueprintType)
enum class EManusHandType : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right"),
	Max		UMETA(Hidden)
};

/**
 * An enum used as a return value for some Manus plugin functions.
 * It must match the WrapperReturnCode enum in CoreSdkWrapperTypes.h.
 */
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
	ConnectionTimeout UMETA(DisplayName = "ConnectionTimeout")
};

/**
 * The parts of the body that can be tracked.
 */
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

/**
 * A way to identify fingers by a name and number.
 */
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

/**
 * A way to identify fingers by a name and number.
 */
UENUM(BlueprintType)
enum class EManusPhalangeName : uint8
{
	Proximal		UMETA(DisplayName = "Proximal"),
	Intermediate	UMETA(DisplayName = "Intermediate"),
	Distal			UMETA(DisplayName = "Distal"),
	Max				UMETA(Hidden)
};

/**
 * A way to identify fingers flex sensors.
 */
UENUM(BlueprintType)
enum class EManusFingerFlexSensorType : uint8
{
	First	UMETA(DisplayName = "First"),
	Second  UMETA(DisplayName = "Second"),
	Max		UMETA(Hidden)
};

/**
 * The names of the bones of a body that Manus can animate.  
 */
UENUM(BlueprintType)
enum class EManusBoneName : uint8
{
	/** The name of the root bone (bone 0). */
	Root,
	/** The name of the hips bone (bone 1). */
	Hips,
	/** The name of the spine bone (bone 2). */
	Spine,
	/** The name of the chest bone (bone 3). */
	Chest,
	/** The name of the upper chest bone (bone 4). */
	UpperChest,
	/** The name of the neck  bone (bone 5). */
	Neck,
	/** The name of the head  bone (bone 6). */
	Head,
	/** The name of the left upper leg  bone (bone 7). */
	LeftUpperLeg,
	/** The name of the left lower leg  bone (bone 8). */
	LeftLowerLeg,
	/** The name of the left foot  bone (bone 9). */
	LeftFoot,
	/** The name of the left toes  bone (bone 10). */
	LeftToes,
	/** The name of the left toes end  bone (bone 11). */
	LeftToesEnd,
	/** The name of the right upper leg  bone (bone 12). */
	RightUpperLeg,
	/** The name of the right lower leg  bone (bone 13). */
	RightLowerLeg,
	/** The name of the right foot  bone (bone 14). */
	RightFoot,
	/** The name of the right toes  bone (bone 15). */
	RightToes,
	/** The name of the right toes end  bone (bone 16). */
	RightToesEnd,
	/** The name of the left shoulder  bone (bone 17). */
	LeftShoulder,
	/** The name of the left upper arm  bone (bone 18). */
	LeftUpperArm,
	/** The name of the left lower arm  bone (bone 19). */
	LeftLowerArm,
	/** The name of the left hand wrist  bone (bone 20). */
	LeftHand,
	/** The name of the right shoulder  bone (bone 21). */
	RightShoulder,
	/** The name of the right upper arm  bone (bone 22). */
	RightUpperArm,
	/** The name of the right lower arm  bone (bone 23). */
	RightLowerArm,
	/** The name of the right hand wrist  bone (bone 24). */
	RightHand,
	/** The name of the left hand tracker  bone (bone 25). */
	LeftHandTracker UMETA(Hidden),
	/** The name of the left hand first thumb  bone (bone 26). This is the proximal joint. */
	LeftHandThumb1,
	/** The name of the left hand second thumb  bone (bone 27). This is the medial joint. */
	LeftHandThumb2,
	/** The name of the left hand last thumb  bone (bone 28). This is the distal joint. */
	LeftHandThumb3,
	/** The name of the left hand first index finger  bone (bone 29). This is the proximal joint. */
	LeftHandIndex1,	
	/** The name of the left hand second index finger  bone (bone 30). This is the medial joint. */
	LeftHandIndex2,	
	/** The name of the left hand last index finger  bone (bone 31). This is the distal joint. */
	LeftHandIndex3,
	/** The name of the left hand first middle finger  bone (bone 32). This is the proximal joint. */
	LeftHandMiddle1,
	/** The name of the left hand second middle finger  bone (bone 33). This is the medial joint. */
	LeftHandMiddle2,
	/** The name of the left hand last middle finger  bone (bone 34). This is the distal joint. */
	LeftHandMiddle3,
	/** The name of the left hand first ring finger  bone (bone 35). This is the proximal joint. */
	LeftHandRing1,
	/** The name of the left hand second ring finger  bone (bone 36). This is the medial joint. */
	LeftHandRing2,
	/** The name of the left hand last ring finger  bone (bone 37). This is the distal joint. */
	LeftHandRing3,
	/** The name of the left hand first pinky  bone (bone 38). This is the proximal joint. */
	LeftHandPinky1,
	/** The name of the left hand second pinky  bone (bone 39). This is the medial joint. */
	LeftHandPinky2,
	/** The name of the left hand last pinky  bone (bone 40). This is the distal joint. */
	LeftHandPinky3,
	/** // The name of the right hand tracker  bone (bone 41). */
	RightHandTracker UMETA(Hidden),	
	/** The name of the right hand first thumb  bone (bone 42). This is the proximal joint. */
	RightHandThumb1,
	/** The name of the right hand second thumb  bone (bone 43). This is the medial joint. */
	RightHandThumb2,
	/** The name of the right hand last thumb  bone (bone 44). This is the distal joint. */
	RightHandThumb3,
	/** The name of the right hand first index finger  bone (bone 45). This is the proximal joint. */
	RightHandIndex1,
	/** The name of the right hand second index finger  bone (bone 46). This is the medial joint. */
	RightHandIndex2,
	/** The name of the right hand last index finger  bone (bone 47). This is the distal joint. */
	RightHandIndex3,
	/** The name of the right hand first middle finger  bone (bone 48). This is the proximal joint. */
	RightHandMiddle1,
	/** The name of the right hand second middle finger  bone (bone 49). This is the medial joint. */
	RightHandMiddle2,
	/** The name of the right hand last middle finger  bone (bone 50). This is the distal joint. */
	RightHandMiddle3,
	/** The name of the right hand first ring finger  bone (bone 51). This is the proximal joint. */
	RightHandRing1,
	/** The name of the right hand second ring finger  bone (bone 52). This is the medial joint. */
	RightHandRing2,
	/** The name of the right hand last ring finger  bone (bone 53). This is the distal joint. */
	RightHandRing3,
	/** The name of the right hand first pinky  bone (bone 54). This is the proximal joint. */
	RightHandPinky1,
	/** The name of the right hand second pinky  bone (bone 55). This is the medial joint. */
	RightHandPinky2,
	/** The name of the right hand last pinky  bone (bone 56). This is the distal joint. */
	RightHandPinky3,
	Max UMETA(Hidden)
};

/**
 * The types of tracking devices that can be used.
 */
UENUM(BlueprintType)
enum class EManusTrackingDeviceType : uint8
{
	Invalid,
	/** Headset. */
	Hmd,
	/** Controller. */
	Controller,
	/** Vive Tracker. */
	GenericTracker
};

UENUM(BlueprintType)
enum class EManusAxisOption : uint8
{
	X,
	Y,
	Z,
	X_Neg,
	Y_Neg,
	Z_Neg,
	Max UMETA(Hidden)
};

/**
 * The skeleton preview modes.
 */
UENUM(BlueprintType)
enum class EManusSkeletonPreviewMode : uint8
{
	/** No preview. */
	NoPreview,
	/** Preview animates the skeleton from the lower to the upper bounds, and reversely. */
	Animated,
	/** Preview sets the skeleton to its lower bounds. */
	LowerBounds,
	/** Preview sets the skeleton to its upper bounds. */
	UpperBounds,
	/** Preview sets the skeleton to the bounds center. */
	BoundsCenter
};

/**
 * The hand preview types.
 */
UENUM(BlueprintType)
enum class EManusHandPreviewType : uint8
{
	Stretch,
	Spread
};

/**
 * Hand Tracking method.
 */
UENUM(BlueprintType)
enum class EManusHandTrackingMethod : uint8
{
	/** Uses tracking data from Manus Core, without any processing. */
	ManusCore,
	/** Uses tracking data from Unreal directly (less latency but only works with Vive Trackers). */
	Unreal
};

/**
 * Retargeting target option
 */
UENUM(BlueprintType)
enum class EManusRetargetingTarget : uint8
{
	BodyEstimation,
	TargetSkeleton,
};

/**
 * Retargeting hand local to option
 */
UENUM(BlueprintType)
enum class EManusRetargetingHandLocalTo : uint8
{
	Root,
	Hips,
	Spine,
	Arms
};



////////////////////////////////////////////////////////////////////////////////
// structs

/**
 * Raw flex-sensor values for a finger.
 * Flex sensor values range from -1 to 1, where 0 is an extended finger, 
 * 1 is a bent finger, and -1 is a finger bent backward.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusFingerRaw
{
	GENERATED_BODY()

	/**
	* MetaCarpo-Phalangeal joint (base joint of a finger).
	* Flex sensor values range from 0 to 1, where 0 is an extended finger,  
	* 1 is a bent finger, and -1 is a finger bent backward.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float McpFlexSensor = 0.0f;

	/**
	* (Proximal) InterPhalangeal joint (middle joint of a finger).
	* Flex sensor values range from 0 to 1, where 0 is an extended finger,  
	* 1 is a bent finger, and -1 is a finger bent backward.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float PipFlexSensor = 0.0f;
};

/**
 * Raw data of a glove.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusGloveRaw
{
	GENERATED_BODY()

	/**
	* Finger data, from thumb to pinky.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FManusFingerRaw> Fingers;

	/**
	* IMU rotations.
	* The first value is the IMU on the back of the hand.
	* The rest of the values go from thumb to pinky, with only the thumb used for Prime One gloves.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FRotator> Imus;
};

/**
 * Processed data for a finger joint.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusFingerJoint
{
	GENERATED_BODY()


	/**
	* Finger stretching value.
	* For the first joint this is a blend between flex and IMU value, for gloves with IMUs on the fingers.
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is pointing away from it to the side.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a stretch of -1 means the finger is stretched upward, and 1 means it is stretched down.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float Stretch = 0.0f;

	/**
	 * The orientation of the phalange (can be applied directly as a bone rotation).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Processed data for a finger.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusFingerProcessed
{
	GENERATED_BODY()

	/**
	* Data for each phalange of this finger.
	* Contains the CMC, MCP, and IP joints for the thumb.
	* Contains the MCP, PIP, and DIP joints for all other fingers.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FManusFingerJoint> Joints;

	/**
	* Finger spreading value, between 0 and 1 for the thumb, and normally between -1 and 1 for other fingers.
	* The thumb will never go outside this range, but other fingers can.
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is spread down, away from the palm.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a spread of -1 means a finger is spread to the left, and 1 means it is spread to the right.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float Spread = 0.0f;
};

/**
 * Information on the hardware behind the glove data.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusGloveInfo
{
	GENERATED_BODY()

	/**
	* A number that identifies the glove this glove info came from.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 GloveId;
	
	/**
	 * A number that identifies the
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 DongleId;
	
	/**
	 * The laterality of the glove.
	 * This value is updated every time glove data is received.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusHandType HandType = EManusHandType::Left;

	/**
	 * The type of the glove.
	 * This value is updated every time glove data is received.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusGloveType GloveType = EManusGloveType::None;

	/**
	 * The battery charge of the device, in percent.
	 * This value is only updated periodically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int32 BatteryPercentage = 0;

	/**
	 * The transmission strength used, in dB.
	 * This value is only updated periodically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int32 TransmissionStrengthInDb = 0;

	/**
	 * Glove type, battery percentage and transmission strength information is received at a lower rate than other data.
	 * This value tracks if data has been received at least once.
	 * These values are not valid if this is false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool ReceivedLowFrequencyData = false;
};

/**
 * All available data on a single glove.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusGlove
{
	GENERATED_BODY()

	/** The time it was last updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 LastUpdateTime;

	/** Processed data on the fingers, from thumb to pinky. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FManusFingerProcessed> Fingers;

	/** The rotation of the wrist coming from the IMU on the back of the hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FQuat WristImuOrientation;

	/** The transform of the wrist computed from a tracker and each User calibration profile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FTransform> WristTrackerTransforms;

	/** Information on the hardware behind this glove's data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusGloveInfo GloveInfo;

	/** Unprocessed data from the glove's sensors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusGloveRaw RawData;
};

/**
 * All available data on a single Polygon skeleton bone.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonBone
{
	GENERATED_BODY()

	/** The validity of the bone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	bool Validity = false;

	/** The transform of the bone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FTransform Transform;
};

/**
 * All available data on a single Polygon skeleton.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonSkeleton
{
	GENERATED_BODY()

	/** The time it was last updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 LastUpdateTime;

	/** A number that identifies the Polygon skeleton. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 PolygonSkeletonId;

	/** Data on the bones. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	TArray<FManusPolygonBone> Bones;
};

/**
 * Rotations extents of one hand. They define the min and max angles each joint of each finger can bend in one direction (in degrees).
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FFingerRotationsExtents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Manus", meta = (UIMin = -180.0, UIMax = 180.0))
	FFloatRange FingerSpreadExtent;

	UPROPERTY(EditAnywhere, Category = "Manus", meta = (UIMin = -180.0, UIMax = 180.0))
	FFloatRange JointsStretchExtents[(int)EManusPhalangeName::Max];
};

/**
 * Defines the pose a finger must match to be recognized as a gesture.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FFingerGestureDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Manus", meta = (UIMin = -1.0, UIMax = 1.0))
	FFloatRange FlexSensorRanges[(int)EManusFingerFlexSensorType::Max];

	FFingerGestureDescriptor()
	{
		for (int i = 0; i < (int)EManusFingerFlexSensorType::Max; i++)
		{
			FlexSensorRanges[i] = FFloatRange(FFloatRangeBound::Inclusive(-1.0f), FFloatRangeBound::Inclusive(1.0f));
		}
	}
};

/**
 * Defines the poses all the fingers of a hand must match to be recognized as a gesture.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FHandGestureDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Manus")
	FName GestureName;

	UPROPERTY(EditAnywhere, Category = "Manus")
	FFingerGestureDescriptor FingerGestureDescriptors[EManusFingerName::Max];
};

/**
 * User-friendly finger data.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusFingerData
{
	GENERATED_BODY()

	/**
	* Finger stretching value for the first joint of the finger (CMC or CarpoMetaCarpal for the thumb, but MCP or MetaCarpoPhalangeal for the other fingers).
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is pointing away from it to the side.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a stretch of -1 means the finger is stretched upward, and 1 means it is stretched down.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float FirstJointStretch = 0.0f;

	/**
	* Finger stretching value for the second joint of the finger (MCP or MetaCarpoPhalangeal for the thumb, but PIP or Proximal InterPhalangeal for the other fingers).
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is pointing away from it to the side.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a stretch of -1 means the finger is stretched upward, and 1 means it is stretched down.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float SecondJointStretch = 0.0f;

	/**
	* Finger stretching value for the third joint of the finger (IP or InterPhalangeal for the thumb, but DIP or Distal InterPhalangeal for the other fingers).
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is pointing away from it to the side.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a stretch of -1 means the finger is stretched upward, and 1 means it is stretched down.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float ThirdJointStretch = 0.0f;

	/**
	* Finger spreading value, between 0 and 1 for the thumb, and normally between -1 and 1 for other fingers.
	* The thumb will never go outside this range, but other fingers can.
	* For the thumb, 0 means it is parallel to the hand, and 1 means it is spread down, away from the palm.
	* For other fingers, with the palm of the hand facing down and looking at the hand from above,
	* a spread of -1 means a finger is spread to the left, and 1 means it is spread to the right.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	float Spread = 0.0f;
};

/**
 * Manus hand animation setup
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusHandAnimationSetup
{
	GENERATED_BODY()

	/** The axis around which the finger joints need to be rotated with the stretch value. */
	UPROPERTY(EditAnywhere, Category = "Manus")
	EManusAxisOption FingersStretchRotationAxis;

	/** The axis around which the finger needs to be rotated with the spread value. */
	UPROPERTY(EditAnywhere, Category = "Manus")
	EManusAxisOption FingersSpreadRotationAxis;

	/** The axis around which the thumb joints need to be rotated with the stretch value. */
	UPROPERTY(EditAnywhere, Category = "Manus")
	EManusAxisOption ThumbStretchRotationAxis;

	/** The axis around which the thumb needs to be rotated with the spread value. */
	UPROPERTY(EditAnywhere, Category = "Manus")
	EManusAxisOption ThumbSpreadRotationAxis;

	/** Fingers rotations extents. They define the min and max angles each joint of each finger can bend in one direction (in degrees). */
	UPROPERTY(EditAnywhere, Category = "Manus")
	FFingerRotationsExtents FingersRotationsExtents[(int)EManusFingerName::Max];
};

/**
 * Manus hand preview settings
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusHandPreviewSettings
{
	GENERATED_BODY()

	/* Defines the preview mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusSkeletonPreviewMode PreviewMode = EManusSkeletonPreviewMode::Animated;

	/* Defines the preview type (stretch or spread). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusHandPreviewType PreviewType = EManusHandPreviewType::Stretch;

	/* Defines how many seconds should the preview last. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus", meta = (ForceUnits = s, UIMin = "0"))
	float PreviewDuration = 5.0f;

	/* Defines how many loops should be executed during the animated preview. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus", meta = (UIMin = "1"))
	int PreviewAnimationLoops = 5;

public:
	float GetManusHandPreviewValue(float LowerValue, float UpperValue, float LoopProgress);
};

/**
 * All available data on a tracker.
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusTracker
{
	GENERATED_BODY()
		
	/** The time it was last updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int64 LastUpdateTime;

	/** The ID of the Tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString TrackerId;

	/** The transform of the tracker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FTransform Transform;

	/** The User Index this Tracker is assigned to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int UserIndex;

	/** The hand type this tracker is used for. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusHandType HandType;
};

/**
 * Retargeting settings for polygon skeleton
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonParameters
{
	GENERATED_BODY()

	// Hips
	/** How much should the character match the speed of the target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hips", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MatchSameSpeed = 0;
	/** Multiplier for the height of the hips */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hips", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float HipHeightMultiplier = 1;

	// Legs
	/** Modifier for the width of the legs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "-3.0", ClampMax = "3.0", UIMin = "-3.0", UIMax = "3.0"))
	float LegWidth = 0;
	/** Offset for the rotation of the leg aim */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legs", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float KneeRotation = 0;

	// Arms
	/** Offset for the shoulder forward rotation in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float ShoulderForwardOffset = 0;
	/** Offset for the shoulder height rotation in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float ShoulderHeightOffset = 0;

	/** Multiplier for the shoulder forward rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float ShoulderForwardMultiplier = 1;
	/** Multiplier for the shoulder height rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float ShoulderHeightMultiplier = 1;

	/** Rotation aim offset for the arm rotation in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float ElbowRotation = 0;
	/** Width offset for the arms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-2.0", ClampMax = "2.0", UIMin = "-2.0", UIMax = "2.0"))
	float ArmSpacing = 0;

	/** Arm length multiplier the bend arms more of less*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float ArmLengthMultiplier = 1;
	/** The relative position of the hand */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms")
	EManusRetargetingHandLocalTo HandLocalPosition = EManusRetargetingHandLocalTo::Arms;
	/** The relative rotation of the hand */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms")
	EManusRetargetingHandLocalTo HandLocalRotation = EManusRetargetingHandLocalTo::Root;

	/** Use arm IK (0 only rotations, 1 only IK)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ArmIK = 0;

	/** Hand local to arms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms")
	bool HandRotationLocal = false;

	/** Forward offset of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float HandForwardOffset = 0;
	/** Width offset of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float HandWidthOffset = 0;
	/** Height offset of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float HandHeightOffset = 0;

	/** Forward position multiplier of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float HandForwardMultiplier = 1;
	/** Width position multiplier of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float HandWidthMultiplier = 1;
	/** Height position multiplier of the hand IK target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float HandHeightMultiplier = 1;

	// Spine
	/** Default bend rotation (in degrees) of the hip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float DefaultHipBend = 0;
	/** Default bend rotation (in degrees) of the spine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float DefaultSpineBend = 0;
	/** Default bend rotation (in degrees) of the neck */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	float DefaultNeckBend = 0;

	/** Bend rotation multiplier of the spine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float SpineBendMultiplier = 1;
	/** Side rotation multiplier of the spine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float SpineAngleMultiplier = 1;
	/** Twist rotation multiplier of the spine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spine", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "2.0"))
	float SpineTwistMultiplier = 1;
};

/**
 * Body estimation target for a polygon skeleton
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonBodyEstimationTarget 
{
	GENERATED_BODY()

	/** The User Index this skeleton should follow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	int32 UserIndex;
};

/**
 * Target skeleton target for a polygon skeleton
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonSkeletonTarget
{
	GENERATED_BODY()

	/** The User Index this skeleton should follow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FString SkeletonName;
};

/**
 * Target for a polygon skeleton
 */
USTRUCT(Blueprintable, BlueprintType)
struct MANUS_API FManusPolygonSkeletonTargetSetting
{
	GENERATED_BODY()
		
	/** The User Index this skeleton should follow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	EManusRetargetingTarget TargetCase;

	/** Body estimation the skeleton should follow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusPolygonBodyEstimationTarget BodyEstimationTarget;

	/** Target skeleton the skeleton should copy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manus")
	FManusPolygonSkeletonTarget SkeletonTarget;
};