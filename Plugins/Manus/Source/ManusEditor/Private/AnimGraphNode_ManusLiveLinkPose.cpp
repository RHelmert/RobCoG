#include "AnimGraphNode_ManusLiveLinkPose.h"
#include "AnimationGraphSchema.h"
#include "Manus.h"
#include "ManusLiveLinkSource.h"
#include "ManusSkeleton.h"

UAnimGraphNode_ManusLiveLinkPose::UAnimGraphNode_ManusLiveLinkPose(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	// Hide "Live Link Subject Name" pin
	FStructProperty* NodeProperty = GetFNodeProperty();
	if (NodeProperty)
	{
		FProperty* Property = NodeProperty->Struct->FindPropertyByName(GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, LiveLinkSubjectName));
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
    if (Node.ManusSkeleton != NULL)
    {
        FManusModule::Get().AddObjectUsingManusLiveLinkUser(Node.ManusSkeleton->TargetUserIndexData.userIndex, Node.ManusSkeleton, this);
    }
}

void UAnimGraphNode_ManusLiveLinkPose::BeginDestroy()
{
	Super::BeginDestroy();
    if (Node.ManusSkeleton != NULL)
    {
        FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(Node.ManusSkeleton->TargetUserIndexData.userIndex, Node.ManusSkeleton, this);
    }
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

FStructProperty* UAnimGraphNode_ManusLiveLinkPose::GetFNodeProperty() const
{
	UScriptStruct* BaseFStruct = FAnimNode_Base::StaticStruct();

	for (TFieldIterator<FProperty> PropIt(GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(*PropIt))
		{
			if (StructProp->Struct->IsChildOf(BaseFStruct))
			{
				return StructProp;
			}
		}
	}

	return NULL;
}

void UAnimGraphNode_ManusLiveLinkPose::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	if (PropertyAboutToChange)
	{
		const FName PropertyName = PropertyAboutToChange->GetFName();

		// When the Manus Live Link User index is about to change
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusSkeleton))
		{
			// We are switching user, update which user's data we're using
            if (Node.ManusSkeleton != NULL)
            {
                FManusModule::Get().RemoveObjectUsingManusLiveLinkUser(Node.ManusSkeleton->TargetUserIndexData.userIndex, Node.ManusSkeleton, this);
            }
		}
	}
}

void UAnimGraphNode_ManusLiveLinkPose::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// When the Manus Live Link User changed
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_ManusLiveLinkPose, ManusSkeleton))
    {
        // We switched user, update which user's data we're using
        if (Node.ManusSkeleton != NULL)
        {
            FManusModule::Get().AddObjectUsingManusLiveLinkUser(Node.ManusSkeleton->TargetUserIndexData.userIndex, Node.ManusSkeleton, this);
        }
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
