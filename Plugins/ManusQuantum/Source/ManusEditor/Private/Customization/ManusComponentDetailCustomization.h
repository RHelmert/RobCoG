// Copyright 2015-2020 Manus

#pragma once

#include "IDetailCustomization.h"

#include "CoreMinimal.h"

/**
* Customizes a UManusComponent
*/
class FManusComponentDetailCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance() 
	{
		return MakeShared<FManusComponentDetailCustomization>();
	}

	// IDetailCustomization interface
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
	// End IDetailCustomization interface

private:
	void ForceRefresh();

private:
	IDetailLayoutBuilder* DetailBuilder = nullptr;
};
