// Copyright 2015-2020 Manus

#pragma once


/**
 * The gesture detection live data of a Manus Live Link User.
 */
struct FManusLiveLinkUserGestureDetectionLiveData
{
	/** Timer timing for how long each gesture was detected */
	TArray<float> ManusLiveLinkUserDetectedHandGestureTimers;
};


/**
 * Manus Live Link Users are the associations of a Manus Dashboard User and a Manus Skeleton, and their data.
 */
struct MANUS_API FManusLiveLinkUser
{
	/** The index of the User as displayed in the Manus Dashboard (first User is index 0).	*/
	int ManusDashboardUserIndex = 0;

	/** The Manus skeleton to use. */
	class UManusSkeleton* ManusSkeleton = NULL;

	/** The objects currently using this Manus Live Link User data. */
	TArray<TWeakObjectPtr<UObject>> ObjectsUsingUser;
	/** Whether we should update the Live Link data or not. */
	bool bShouldUpdateLiveLinkData = false;

	/** Manus data last update time. */
	int64 ManusDataLastUpdateTime = 0;

	/** Hand gesture detection data. */
	FManusLiveLinkUserGestureDetectionLiveData HandGestureDetectionData[(int)EManusHandType::Max];

	/** Polygon initialization retry frames countdown. */
	int PolygonInitializationRetryCountdown = 0;

	/** Polygon initialization retry number. */
	int PolygonInitializationRetryNumber = 0;

#if WITH_EDITORONLY_DATA
	/** Whether a hand preview is on-going. */
	bool bIsHandPreviewOnGoing = false;

	/** On-going hand preview settings. */
	FManusHandPreviewSettings OnGoingHandPreviewSettings;

	/** On-going hand preview timer. */
	float OnGoingHandPreviewTimer = 0.0f;

	/** On-going hand preview gesture index. */
	int OnGoingHandPreviewGestureIndex;

	/** On-going hand preview loop progress. */
	float OnGoingHandPreviewLoopProgress;

	/** On-going hand preview bounds. */
	FFingerRotationsExtents OnGoingHandPreviewBounds[(int)EManusFingerName::Max];
#endif // WITH_EDITORONLY_DATA

public:
	int64 GetLeftGloveId() const;
	int64 GetRightGloveId() const;
};
