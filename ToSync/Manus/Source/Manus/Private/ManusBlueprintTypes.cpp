// Copyright 2015-2020 Manus

#include "ManusBlueprintTypes.h"
#include "Manus.h"
#include "ManusBlueprintLibrary.h"


////////////////////////////////////////////////////////////////////////////////////////////
/// FManusLiveLinkUser

int64 FManusLiveLinkUser::GetLeftGloveId() const
{
	int64 GloveId = 0;
	UManusBlueprintLibrary::GetManusDashboardUserGloveId(ManusDashboardUserIndex, EManusHandType::Left, GloveId);
	return GloveId;
}

int64 FManusLiveLinkUser::GetRightGloveId() const
{
	int64 GloveId = 0;
	UManusBlueprintLibrary::GetManusDashboardUserGloveId(ManusDashboardUserIndex, EManusHandType::Right, GloveId);
	return GloveId;
}


////////////////////////////////////////////////////////////////////////////////////////////
/// FManusHandPreviewSettings

float FManusHandPreviewSettings::GetManusHandPreviewValue(float LowerValue, float UpperValue, float LoopProgress)
{
	float PreviewValue = 0.0f;
	switch (PreviewMode)
	{
	case EManusSkeletonPreviewMode::Animated:
		PreviewValue = FMath::Lerp(LowerValue, UpperValue, LoopProgress);
		break;
	case EManusSkeletonPreviewMode::LowerBounds:
		PreviewValue = FMath::Lerp(LowerValue, UpperValue, 0.0f);
		break;
	case EManusSkeletonPreviewMode::UpperBounds:
		PreviewValue = FMath::Lerp(LowerValue, UpperValue, 1.0f);
		break;
	case EManusSkeletonPreviewMode::BoundsCenter:
		PreviewValue = FMath::Lerp(LowerValue, UpperValue, 0.5f);
		break;
	}
	return PreviewValue;
}