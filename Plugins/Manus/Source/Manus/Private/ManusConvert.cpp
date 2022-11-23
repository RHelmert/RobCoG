// Copyright 2015-2020 Manus

#include "ManusConvert.h"
#include "Manus.h"


const FVector ManusConvert::MANUS_UP_VECTOR = FVector(0.0f, 1.0f, 0.0f);


// Convert an SDK hand type to a blueprint-compatible hand type.
EManusRet ManusConvert::SdkHandTypeToBp(HandType Input, EManusHandType &Output)
{
	switch (Input)
	{
	case HandType::eHANDTYPE_LEFT_HAND:
		Output = EManusHandType::Left;

		return EManusRet::Success;

	case HandType::eHANDTYPE_RIGHT_HAND:
		Output = EManusHandType::Right;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK HandType value to a blueprint-compatible HandType value. The value was %d."), static_cast<int>(Input));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert a blueprint-compatible hand type to an SDK hand type.
EManusRet ManusConvert::BpHandTypeToSdk(EManusHandType Input, HandType &Output)
{
	switch (Input)
	{
	case EManusHandType::Left:
		Output = HandType::eHANDTYPE_LEFT_HAND;

		return EManusRet::Success;

	case EManusHandType::Right:
		Output = HandType::eHANDTYPE_RIGHT_HAND;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised blueprint-compatible HandType value to an SDK HandType value. The value was %d."), static_cast<int>(Input));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert a hand type to an IsLeft bool that can be used to check if an hand is the left hand.
EManusRet ManusConvert::HandTypeToIsLeft(EManusHandType HandType, bool &Output)
{
	switch (HandType)
	{
	case EManusHandType::Left:
		Output = true;

		return EManusRet::Success;

	case EManusHandType::Right:
		Output = false;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised HandType value to an IsLeft bool. The value was %d."), static_cast<int>(HandType));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert a hand type to an integer value (0 or 1) for use with arrays.
EManusRet ManusConvert::HandTypeToArrayIndex(EManusHandType HandType, unsigned int &Output)
{
	switch (HandType)
	{
	case EManusHandType::Left:
		Output = 0;

		return EManusRet::Success;

	case EManusHandType::Right:
		Output = 1;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised blueprint-compatible HandType value to an array index. The value was %d."), static_cast<int>(HandType));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert a hand type to a string. Useful for log messages.
EManusRet ManusConvert::HandTypeToString(EManusHandType HandType, FString &Output)
{
	switch (HandType)
	{
	case EManusHandType::Left:
		Output = FString("Left");

		return EManusRet::Success;

	case EManusHandType::Right:
		Output = FString("Right");

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised blueprint-compatible HandType value to a string. The value was %d."), static_cast<int>(HandType));
		Output = FString("unrecognised");

		return EManusRet::InvalidArgument;
	}

	Output = FString("error");

	return EManusRet::InvalidArgument;
}

// Convert an SDK glove type to a blueprint-compatible glove type.
EManusRet ManusConvert::SdkGloveTypeToBp(GloveType Input, EManusGloveType& Output)
{
	switch (Input)
	{
	case GloveType::eGLOVETYPE_PRIME_ONE:
		Output = EManusGloveType::PrimeOne;

		return EManusRet::Success;

	case GloveType::eGLOVETYPE_PRIME_TWO:
		Output = EManusGloveType::PrimeTwo;

		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK GloveType value to a blueprint-compatible GloveType value. The value was %d."), static_cast<int>(Input));

		return EManusRet::InvalidArgument;
	}

	return EManusRet::Error;
}

// Convert an SDK tracker type to a blueprint-compatible hand type.
EManusRet ManusConvert::SdkTrackerTypeToBpHandType(uint32_t Input, EManusHandType& Output)
{
	// See the file Tracker.pb.h for the "TrackerType" enum values used as SDK tracker types.
	switch (Input)
	{
	case 3:
		Output = EManusHandType::Left;
		return EManusRet::Success;

	case 4:
		Output = EManusHandType::Right;
		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised SDK tracker type value to a BP ManusHandType value. The value was %d."), (int)Input);
		return EManusRet::InvalidArgument;
	}
	return EManusRet::Error;
}

// Convert a blueprint-compatible hand type to an SDK tracker type.
EManusRet ManusConvert::BpHandTypeToSdkTrackerType(EManusHandType Input, uint32_t& Output)
{
	// See the file Tracker.pb.h for the "TrackerType" enum values used as SDK tracker types.
	switch (Input)
	{
	case EManusHandType::Left:
		Output = 3;
		return EManusRet::Success;

	case EManusHandType::Right:
		Output = 4;
		return EManusRet::Success;

	default:
		UE_LOG(LogManus, Error, TEXT("Attempted to convert an unrecognised BP ManusHandType value to an SDK tracker type value. The value was %d."), (int)Input);
		return EManusRet::InvalidArgument;
	}
	return EManusRet::Error;
}

// Convert a transform from Unity to Unreal coordinates.
FTransform ManusConvert::ConvertUnityToUnrealTransform(FTransform Transform)
{
	FTransform ConvertedTransform;

	// Translation
	FVector ConvertedTranslation;
	FVector Translation = Transform.GetLocation();
	ConvertedTranslation.X = Translation.Z * +100.0f;	// Convert Core -> UE4 coordinate system: UE4 +Y is Core +X
	ConvertedTranslation.Y = Translation.X * +100.0f;	// Convert Core -> UE4 coordinate system: UE4 +Z is Core +Y
	ConvertedTranslation.Z = Translation.Y * +100.0f;	// Convert Core -> UE4 coordinate system: UE4 +X is Core +Z
	ConvertedTransform.SetLocation(ConvertedTranslation);

	// Rotation
	ConvertedTransform.SetRotation(ConvertUnityToUnrealQuat(Transform.GetRotation()));

	// Scale
	FVector ConvertedScale;
	FVector Scale = Transform.GetScale3D();
	ConvertedScale.X = Scale.Z;
	ConvertedScale.Y = Scale.X;
	ConvertedScale.Z = Scale.Y;
	ConvertedTransform.SetScale3D(ConvertedScale);
	
	return ConvertedTransform;
}

// Convert a transform from Unreal to Unity coordinates.
FTransform ManusConvert::ConvertUnrealToUnityTransform(FTransform Transform)
{
	FTransform ConvertedTransform;

	// Translation
	FVector ConvertedTranslation;
	FVector Translation = Transform.GetLocation();
	ConvertedTranslation.X = Translation.Y * +0.01f;	// Convert Core -> UE4 coordinate system: UE4 +Y is Core +X
	ConvertedTranslation.Y = Translation.Z * +0.01f;	// Convert Core -> UE4 coordinate system: UE4 +Z is Core +Y
	ConvertedTranslation.Z = Translation.X * +0.01f;	// Convert Core -> UE4 coordinate system: UE4 +X is Core +Z
	ConvertedTransform.SetLocation(ConvertedTranslation);
	
	// Rotation
	ConvertedTransform.SetRotation(FQuat(
		Transform.GetRotation().Y,
		Transform.GetRotation().Z,
		Transform.GetRotation().X,
		Transform.GetRotation().W
	));

	// Scale
	FVector ConvertedScale;
	FVector Scale = Transform.GetScale3D();
	ConvertedScale.X = Scale.Y;
	ConvertedScale.Y = Scale.Z;
	ConvertedScale.Z = Scale.X;
	ConvertedTransform.SetScale3D(ConvertedScale);
	
	return ConvertedTransform;
}


FQuat ManusConvert::ConvertUnityToUnrealQuat(FQuat Quaternion)
{
	 return FQuat(Quaternion.Z, Quaternion.X, Quaternion.Y, Quaternion.W);
}
