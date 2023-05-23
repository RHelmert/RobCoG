#include "AnimGraphNode_ManusLiveLinkPose.h"
#include "AnimationGraphSchema.h"
#include "Manus.h"
#include "ManusLiveLinkSource.h"


UAnimGraphNode_ManusLiveLinkPose::UAnimGraphNode_ManusLiveLinkPose(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Hide "Live Link Subject Name" pin
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FStructProperty* NodeProperty = GetFNodeProperty();
#else
	UStructProperty* NodeProperty = GetFNodeProperty();
#endif
	if (NodeProperty)
	{
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
		FProperty* Property = NodeProperty->Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, LiveLinkSubjectName));
#else
#if ENGINE_MINOR_VERSION >= 23
		UProperty* Property = NodeProperty->Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, LiveLinkSubjectName));
#else
		UProperty* Property = NodeProperty->Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, SubjectName));
#endif
#endif
		if (Property)
		{
			Property->RemoveMetaData(GetDefault<UAnimationGraphSchema>()->NAME_PinShownByDefault);
			Property->SetMetaData(GetDefault<UAnimationGraphSchema>()->NAME_PinHiddenByDefault, TEXT("true"));
		}
	}
}

void UAnimGraphNode_ManusLiveLinkPose::PostLoad()
{
	Super::PostLoad();

	FManusModule::Get().AddObjectUsingManusLiveLinkUser(Node.ManusDashboardUserIndex, Node.ManusSkeleton, this);
}

void UAnimGraphNode_ManusLiveLinkPose::BeginDestroy()
{
	Super::BeginDestroy();

	FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(Node.ManusDashboardUserIndex, Node.ManusSkeleton, this);
}

FText UAnimGraphNode_ManusLiveLinkPose::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(FString::Printf(TEXT("Manus Live Link Pose")));
}

FLinearColor UAnimGraphNode_ManusLiveLinkPose::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.75f);
}

FText UAnimGraphNode_ManusLiveLinkPose::GetTooltipText() const
{
	return FText::FromString(TEXT("Applies data recieved from Manus Live Link to a skeletal mesh."));
}

FString UAnimGraphNode_ManusLiveLinkPose::GetNodeCategory() const
{
	return FString("Manus");
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
FStructProperty* UAnimGraphNode_ManusLiveLinkPose::GetFNodeProperty() const
#else
UStructProperty* UAnimGraphNode_ManusLiveLinkPose::GetFNodeProperty() const
#endif
{
	UScriptStruct* BaseFStruct = FAnimNode_Base::StaticStruct();

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	for (TFieldIterator<FProperty> PropIt(GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
#else
	for (TFieldIterator<UProperty> PropIt(GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
#endif
	{
#if ENGINE_MAJOR_VERSION == 5 ||  ENGINE_MINOR_VERSION >= 25
		if (FStructProperty* StructProp = CastField<FStructProperty>(*PropIt))
#else
		if (UStructProperty* StructProp = Cast<UStructProperty>(*PropIt))
#endif
		{
			if (StructProp->Struct->IsChildOf(BaseFStruct))
			{
				return StructProp;
			}
		}
	}

	return NULL;
}

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
void UAnimGraphNode_ManusLiveLinkPose::PreEditChange(FProperty* PropertyAboutToChange)
#else
void UAnimGraphNode_ManusLiveLinkPose::PreEditChange(UProperty* PropertyAboutToChange)
#endif
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange)
	{
		const FName PropertyName = PropertyAboutToChange->GetFName();

		// When the Manus Live Link User index is about to change
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusDashboardUserIndex) || PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusSkeleton))
		{
			// We are switching user, update which user's data we're using
			FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(Node.ManusDashboardUserIndex, Node.ManusSkeleton, this);
		}
	}
}

void UAnimGraphNode_ManusLiveLinkPose::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// When the Manus Live Link User changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusDashboardUserIndex) || PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusSkeleton))
	{
		// We switched user, update which user's data we're using
		FManusModule::Get().AddObjectUsingManusLiveLinkUser(Node.ManusDashboardUserIndex, Node.ManusSkeleton, this);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
