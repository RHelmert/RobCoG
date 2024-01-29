// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Set up a Doxygen group.
/** @addtogroup FAnimNode_ManusLiveLinkPose
 *  @{
 */

#include "AnimNode_LiveLinkPose.h"
#include "ManusBlueprintTypes.h"
#include "AnimNode_ManusLiveLinkPose.generated.h"

/// @brief
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

    /// @brief The %Manus skeleton to use. When available, the %Manus skeleton from the %Manus component will be used instead. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ManusLiveLink")
	class UManusSkeleton* ManusSkeleton = NULL;

    /// @brief Tracking device delta transform. 
	UPROPERTY(EditAnywhere, Category = "ManusLiveLink", meta = (NeverAsPin))
	FTransform TrackingDeviceDeltaTransform[(int)EManusHandType::Max];
};


// Close the Doxygen group.
/** @} */
