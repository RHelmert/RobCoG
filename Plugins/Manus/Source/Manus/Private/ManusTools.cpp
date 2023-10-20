// Copyright 2015-2022 Manus

#include "ManusTools.h"
#include "Manus.h"
#include "ManusSettings.h"
#include "ManusConvert.h"
#include "CoreSdk.h"
#include "ManusTypeInitializers.h"

#include "AnimationRuntime.h"
#include "Animation/Skeleton.h"

NodeSetup ManusTools::CreateNodeSetup(uint32_t p_Id, uint32_t p_ParentId, FTransform p_Transform, FName p_Name)
{
	NodeSetup t_Node;
	ManusTypeInitializer::NodeSetup_Init(&t_Node);

	t_Node.id = p_Id;	
    
    ConvertFStringToUTF8Array(t_Node.name, p_Name.ToString(),MAX_NUM_CHARS_IN_NODE_NAME);

	t_Node.type = NodeType::NodeType_Joint;
	t_Node.parentID = p_ParentId;
	t_Node.settings.usedSettings = NodeSettingsFlag::NodeSettingsFlag_None; //(NodeSettingsFlag)(t_Node.settings.usedSettings | NodeSettingsFlag::NodeSettings_IK); // todo make this a parameter
	//t_Node.settings.ik.ikAim = 1;

	t_Node.transform.position.x = p_Transform.GetTranslation().X; 
	t_Node.transform.position.y = p_Transform.GetTranslation().Y;
	t_Node.transform.position.z = p_Transform.GetTranslation().Z;

	t_Node.transform.rotation.x = p_Transform.GetRotation().X;
	t_Node.transform.rotation.y = p_Transform.GetRotation().Y;
	t_Node.transform.rotation.z = p_Transform.GetRotation().Z;
	t_Node.transform.rotation.w = p_Transform.GetRotation().W;

	t_Node.transform.scale.x = p_Transform.GetScale3D().X;
	t_Node.transform.scale.y = p_Transform.GetScale3D().Y;
	t_Node.transform.scale.z = p_Transform.GetScale3D().Z;
	return t_Node;
}

// convert FString to UTF8 array (stored in char array)
void ManusTools::ConvertFStringToUTF8Array(char* p_OutputArray, FString p_InputString, int p_MaxSize)
{
    for (int i = 0; i < p_MaxSize; i++)
    {
        p_OutputArray[i] = 0;
    }

    FTCHARToUTF8 t_ToUtf8Converter(p_InputString.GetCharArray().GetData());
    auto t_Utf8StringSize = t_ToUtf8Converter.Length();
    auto t_Utf8String = t_ToUtf8Converter.Get();

    for (auto i = 0; i < t_Utf8StringSize; i++)
    {
        p_OutputArray[i] = t_Utf8String[i]; // this is now an utf8 encoded string in a char array.
    }
}