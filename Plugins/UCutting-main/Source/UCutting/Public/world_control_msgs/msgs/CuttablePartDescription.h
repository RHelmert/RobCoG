// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen

#pragma once
#include "ROSBridgeMsg.h"
//#include "PhysicsConstraintDetails.h"
#include "geometry_msgs/Pose.h"

namespace world_control_msgs
{
    class CuttablePartDescription : public FROSBridgeMsg
	{
        FString Name;
		geometry_msgs::Pose Pose;
        float Height;
        float Width;
        float Depth;


	public:
        CuttablePartDescription() {}

        CuttablePartDescription(FString InName, geometry_msgs::Pose InPose, float InHeight, float InWidth, float InDepth)
		{
            CuttablePartDescription();
            Name = InName;
			Pose = InPose;
            Height=InHeight;
            Width=InWidth;
            Depth=InDepth;
		}

        FString GetName()
		{
            return Name;
		}

		geometry_msgs::Pose GetPose()
		{
			return Pose;
		}
        float GetHeight()
        {
            return Height;
        }
        float GetWidth()
        {
            return Width;
        }
        float GetDepth()
        {
            return Depth;
        }

		virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
		{
            Name = JsonObject->GetStringField("name");
			Pose.FromJson(JsonObject->GetObjectField("pose"));
            Height=JsonObject->GetNumberField("height");
            Width=JsonObject->GetNumberField("width");
            Depth=JsonObject->GetNumberField("depth");
		}

        static CuttablePartDescription GetFromJson(TSharedPtr<FJsonObject> JsonObject)
		{
            CuttablePartDescription Result;
			Result.FromJson(JsonObject);
			return Result;
		}

		virtual FString ToString() const override
		{
            return "ConstraintDescription {name = " + Name +
                ", pose = " + Pose.ToString() +
                ", height = " + *FString::SanitizeFloat(Height) +
                ", width = " + *FString::SanitizeFloat(Width) +
                ", depth = " + *FString::SanitizeFloat(Depth) +"}";
		}

		virtual TSharedPtr<FJsonObject> ToJsonObject() const override
		{
			TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
            Object->SetStringField(TEXT("name"), Name);
			Object->SetObjectField(TEXT("pose"), Pose.ToJsonObject());
            Object->SetNumberField(TEXT("height"),Height);
            Object->SetNumberField(TEXT("width"),Width);
            Object->SetNumberField(TEXT("depth"),Depth);
			return Object;
		}

		virtual FString ToYamlString() const override
		{
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(ToJsonObject().ToSharedRef(), Writer);
			return OutputString;
		}
	};
}
