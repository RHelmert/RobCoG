#pragma once

#include <AnimGraphNode_Base.h>
#include "AnimNode_ManusLiveLinkPose.h"
#include "AnimGraphNode_ManusLiveLinkPose.generated.h"

/**
 * An animation node base that animates a mesh based on Manus Live Link data.
 */
UCLASS(Blueprintable, BlueprintType)
class MANUSEDITOR_API UAnimGraphNode_ManusLiveLinkPose : public UAnimGraphNode_Base
{
	GENERATED_UCLASS_BODY()

	virtual void PostLoad() override;
	virtual void BeginDestroy() override;

protected:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FString GetNodeCategory() const override;
	// End of UEdGraphNode interface

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
#else
	virtual void PreEditChange(UProperty* PropertyAboutToChange) override;
#endif
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

public:
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 25
	FStructProperty* GetFNodeProperty() const;
#else
	UStructProperty* GetFNodeProperty() const;
#endif

public:
	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_ManusLiveLinkPose Node;
};
