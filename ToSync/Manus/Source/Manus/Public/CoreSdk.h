// Copyright 2015-2020 Manus

#pragma once

#include "ManusBlueprintTypes.h"

/**
 * Manages all function pointers for the (wrapped) CoreSDK.
 * Used by the blueprint library.
 * Use the blueprint library instead of this whenever possible.
 */
class MANUS_API CoreSdk
{
public:
	static EManusRet Initialize();
	static EManusRet ShutDown();
	static EManusRet ConnectLocally();

	static EManusRet CheckConnection();
	static EManusRet CheckCompatibility();

	static EManusRet VibrateWristOfGlove(int64 GloveId, float UnitStrength, int32 DurationInMilliseconds);
	static EManusRet VibrateFingers(int64 DongleId, EManusHandType HandTypeOfGlove, TArray<float> Powers);

	static EManusRet GetGloveIdOfUser_UsingUserIndex(int32 UserIndex, EManusHandType HandTypeOfGlove, int64& GloveId);
	static EManusRet GetNumberOfAvailableGloves(int32 &NumberOfAvailableGloves);
	static EManusRet GetIdsOfAvailableGloves(TArray<int64> &IdsOfAvailableGloves);
	static EManusRet GetHandTypeOfGlove(int64 GloveId, EManusHandType &HandTypeOfGlove);
	static EManusRet GetIdOfFirstAvailableGloveOfType(EManusHandType HandTypeOfGlove, int64 &GloveId);
	
	static EManusRet GetDataForGlove_UsingGloveId(int64 GloveId, FManusGlove &DataForGlove);
	static EManusRet GetDataForGlove_UsingHandType(EManusHandType HandTypeOfGlove, FManusGlove &DataForGlove);

	static EManusRet GetNumberOfHapticDongles(int32& NumberOfHapticDongles);
	static EManusRet GetHapticDongleIds(TArray<int64>& HapticDongleIds);

	static EManusRet AddOrUpdatePolygonSkeleton(int64 CalibrationProfileId, int64 SkeletonId, class UManusSkeleton* ManusSkeleton);
	static EManusRet RemovePolygonSkeleton(int64 PolygonSkeletonId);
	static EManusRet SetPolygonSkeletonTarget(FManusPolygonSkeletonTarget SkeletonTarget);
	static EManusRet SetRetargetingSettings(int64 PolygonSkeletonId, FManusPolygonParameters PolygonSkeletonRetargetingSettings);
	static EManusRet IsPolygonSkeletonIdValid(int64 PolygonSkeletonId, bool &IsValid);
	static EManusRet GetNumberOfAvailablePolygonSkeletons(int32& NumberOfAvailablePolygonSkeletons);
	static EManusRet GetIdsOfAvailablePolygonSkeletons(TArray<int64> &IdsOfAvailablePolygonSkeletons);
	static EManusRet GetIdOfFirstAvailablePolygonSkeleton(int64 &PolygonSkeletonId);
	static EManusRet GetDataForPolygonSkeleton(int64 PolygonSkeletonId, FManusPolygonSkeleton& DataForPolygonSkeleton);

	static EManusRet GetDataForTracker(int32 UserIndex, EManusHandType HandTypeOfTracker, FManusTracker& DataForTracker);

private:
	struct CoreFunctionPointers;
	static CoreFunctionPointers* FunctionPointers;

	static void* DllHandle;
};