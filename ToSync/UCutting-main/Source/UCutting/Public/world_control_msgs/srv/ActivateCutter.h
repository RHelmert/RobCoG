#pragma once
#include "ROSBridgeSrv.h"



class UROSBRIDGE_API FROSActivateCutterSrv : public FROSBridgeSrv
{
        protected:
            FString Type;

        public:
            FROSActivateCutterSrv(FString InType)
            {
                Type = InType;
            }

            class Request : public SrvRequest
            {
            private:
                FString Id;
//                TArray<world_control_msgs::Tag> Tags; // Maybe we want to switch over to many cutters but for now One Message one knife
                bool bCanCut;

            public:
                Request() {}

                Request(FString InId,/* TArray<world_control_msgs::Tag> InTags,*/ bool InbCanCut)
                {
                    Id = InId;
                    bCanCut=InbCanCut;
                }

                FString GetId()
                {
                    return Id;
                }

//                TArray<world_control_msgs::Tag> GetTags()
//                {
//                    return Tags;
//                }

                bool GetbCanCut()
                {
                    return bCanCut;
                }

                virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
                {
                    Id = JsonObject->GetStringField("id");
//                    Tags.Empty();
//                    TArray<TSharedPtr<FJsonValue>> TagsPtrArray = JsonObject->GetArrayField(TEXT("tags"));
//                    for (auto &ptr : TagsPtrArray)
//                    {
//                        world_control_msgs::Tag Entry;
//                        Entry.FromJson(ptr->AsObject());
//                        Tags.Add(Entry);
//                    }
                    bCanCut = JsonObject->GetBoolField("bCanCut");
                }

                static Request GetFromJson(TSharedPtr<FJsonObject> JsonObject)
                {
                    Request Req;
                    Req.FromJson(JsonObject);
                    return Req;
                }

                FString ToString() const override
                {
                    return "FROSActivatehSrv:Request {id = " + Id +
//                        ", tags size = " + FString::FromInt(Tags.Num()) +
                        ", bCanCut = " + (bCanCut ? FString("True") : FString("False")) +"}";
                }

                virtual TSharedPtr<FJsonObject> ToJsonObject() const override
                {
                    TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
                    Object->SetStringField(TEXT("id"), Id);
//                    TArray<TSharedPtr<FJsonValue>> TagsPtrArray;
//                    for (auto &Entry : Tags)
//                    {
//                        TSharedPtr<FJsonValue> Ptr = MakeShareable(new FJsonValueObject(Entry.ToJsonObject()));
//                        TagsPtrArray.Add(Ptr);
//                    }
                    Object->SetBoolField(TEXT("bCanCut"),bCanCut);
                    return Object;
                }

            };

            class Response : public SrvResponse
            {
            private:
                bool Success;


            public:
                Response() {}

                Response(bool InSuccess)
                {
                    Success = InSuccess;
                }


                bool GetSuccess()
                {
                    return Success;
                }

                virtual void FromJson(TSharedPtr<FJsonObject> JsonObject) override
                {
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
                    return "FROSSpawnModelSrv:Response {success = " + (Success ? FString("True") : FString("False")) + "}";
                }

                virtual TSharedPtr<FJsonObject> ToJsonObject() const override
                {
                    TSharedPtr<FJsonObject> Object = MakeShareable<FJsonObject>(new FJsonObject());
                    Object->SetBoolField(TEXT("success"), Success);
                    return Object;
                }

            };

        };
