#pragma once
#include "ROSBridgeSrv.h"
#include "world_control_msgs/msgs/CuttablePartDescription.h"


class UROSBRIDGE_API FROSGetCuttableInfoSrv : public FROSBridgeSrv
{
        protected:
            FString Type;

        public:
            FROSGetCuttableInfoSrv(FString InType)
            {
                Type = InType;
            }

            class Request : public SrvRequest
            {
            private:
                FString Id;

            public:
                Request() {}

                Request(FString InId)
                {
                    Id = InId;
                }

                FString GetId()
                {
                    return Id;
                }

                virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
                {
                    Id = JsonObject->GetStringField("id");
                }

                static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
                {
                    Request Req;
                    Req.FromJson(JsonObject);
                    return Req;
                }

                FString ToString() const override
                {
                    return "FROSActivatehSrv:Request {id = " + Id +"}";
                }

                virtual TSharedPtr<FJsonObject> ToJsonObject() const override
                {
                    TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
                    Object->SetStringField(TEXT("id"), Id);
                    return Object;
                }

            };

            class Response : public SrvResponse
            {
            private:
                FString Name;
                TArray<world_control_msgs::CuttablePartDescription> CuttableParts;
                bool Success;


            public:
                Response() {}

                Response(FString InName, TArray<world_control_msgs::CuttablePartDescription>InCuttableParts, bool InSuccess)
                {
                    Name = InName;
                    CuttableParts=InCuttableParts;
                    Success = InSuccess;
                }
                Response(bool InSuccess)
                {
                    Success = InSuccess;
                }

                FString GetName()
                {
                    return Name;
                }

                TArray< world_control_msgs::CuttablePartDescription> GetCuttableParts()
                {
                    return CuttableParts;
                }


                bool GetSuccess()
                {
                    return Success;
                }

                virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
                {
                    Name = JsonObject->GetStringField("name");

                    CuttableParts.Empty();
                    TArray<TSharedPtr<FJsonValue>> CuttablePartsPtrArray= JsonObject->GetArrayField(TEXT("CuttableParts"));
                    for(auto& ptr: CuttablePartsPtrArray)
                    {
                        world_control_msgs::CuttablePartDescription CuttablePart;
                        CuttablePart.FromJson(ptr->AsObject());
                        CuttableParts.Add(CuttablePart);
                    }

                    Success = JsonObject->GetBoolField("success");
                }

                static Response GetFromJson(TSharedPtr<FJsonObject> JsonObject)
                {
                    Response Res;
                    Res.FromJson(JsonObject);
                    return Res;
                }

                FString ToString() const override
                {

                    return "FROSGetCuttableInfoSrv::Response { name = " + Name +
                            ", CuttableParts size = " + FString::FromInt(CuttableParts.Num()) +
                            ", success = " + (Success ? FString("True") : FString("False")) + "}";
                }

                virtual TSharedPtr<FJsonObject> ToJsonObject() const override
                {
                    TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
                    Object->SetStringField(TEXT("name"), Name);

                    TArray<TSharedPtr<FJsonValue>> CuttablePartsPtrArray;
                    for(auto &Entry : CuttableParts)
                    {
                        TSharedPtr<FJsonValue> Ptr = MakeShareable(new FJsonValueObject(Entry.ToJsonObject()));
                        CuttablePartsPtrArray.Add(Ptr);
                    }
                    Object->SetArrayField(TEXT("CuttableParts"),CuttablePartsPtrArray);
                    Object->SetBoolField(TEXT("success"), Success);
                    return Object;
                }

            };

        };
