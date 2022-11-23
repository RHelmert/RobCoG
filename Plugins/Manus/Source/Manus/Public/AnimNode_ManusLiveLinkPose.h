// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AnimNode_LiveLinkPose.h"
#include "ManusBlueprintTypes.h"
#include "AnimNode_ManusLiveLinkPose.generated.h"


USTRUCT(BlueprintInternalUseOnly)
struct MANUS_API FAnimNode_ManusLiveLinkPose : public FAnimNode_LiveLinkPose
{
	GENERATED_BODY()

public:
	FAnimNode_ManusLiveLinkPose();

	//~ FAnimNode_Base interface
	virtual bool HasPreUpdate() const { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	//~ End of FAnimNode_Base interface


public:
	/** The index of the User as displayed in the Manus Dashboard (first User is index 0). When available, the Manus Dashboard User index from the Manus component will be used instead.	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ManusLiveLink", meta = (UIMin = 0))
	int ManusDashboardUserIndex;

	/** The Manus skeleton to use. When available, the Manus skeleton from the Manus component will be used instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ManusLiveLink")
	class UManusSkeleton* ManusSkeleton;

	/** Motion capture type. When available, the motion capture type from the Manus component will be used instead. */
	UPROPERTY(EditAnywhere, Category = "ManusLiveLink", meta = (NeverAsPin))
	EManusMotionCaptureType MotionCaptureType;

	/** Tracking device delta transform. */
	UPROPERTY(EditAnywhere, Category = "ManusLiveLink", meta = (NeverAsPin))
	FTransform TrackingDeviceDeltaTransform[(int)EManusHandType::Max];

	/** List of bones with their remapped names. */
	UPROPERTY(Transient)
	FBoneReference BoneMap[(int)EManusBoneName::Max];

	/** Map bone names to their enum counterparts. */
	UPROPERTY(Transient)
	TMap<FName, EManusBoneName> BoneNameToEnum;
};
