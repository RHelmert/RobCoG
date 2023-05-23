// Copyright 2015-2020 Manus

#pragma once

#include "ManusBlueprintTypes.h"
#include "CoreSdkWrapper.h"

struct MANUS_API ManusConvert
{
	static EManusRet SdkHandTypeToBp(HandType Input, EManusHandType &Output);
	static EManusRet BpHandTypeToSdk(EManusHandType Input, HandType &Output);

	static EManusRet HandTypeToIsLeft(EManusHandType HandType, bool &Output);
	static EManusRet HandTypeToArrayIndex(EManusHandType HandType, unsigned int &Output);

	static EManusRet HandTypeToString(EManusHandType HandType, FString &Output);

	static EManusRet SdkGloveTypeToBp(GloveType Input, EManusGloveType& Output);

	static EManusRet SdkTrackerTypeToBpHandType(uint32_t Input, EManusHandType& Output);
	static EManusRet BpHandTypeToSdkTrackerType(EManusHandType Input, uint32_t& Output);

	static FTransform ConvertUnityToUnrealTransform(FTransform Transform);
	static FTransform ConvertUnrealToUnityTransform(FTransform Transform);
	static FQuat ConvertUnityToUnrealQuat(FQuat Quaternion);


	static const FVector MANUS_UP_VECTOR;
};