#include "Individuals/Type/SLCustomStaticMeshComponentIndividual.h"

USLCustomStaticMeshComponentIndividual::USLCustomStaticMeshComponentIndividual() {

}


// Called before destroying the object.
void USLCustomStaticMeshComponentIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLCustomStaticMeshComponentIndividual::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(Super::Init(bReset) && InitImpl());
	return IsInit();
}

// Load semantic data
bool USLCustomStaticMeshComponentIndividual::Load(bool bReset, bool bTryImport)
{
	if (bReset)
	{
		LoadReset();
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Cannot load individual %s, init fails.."),
				*FString(__FUNCTION__), __LINE__, *GetFullName());
			return false;
		}
	}

	SetIsLoaded(Super::Load(bReset, bTryImport) && LoadImpl(bTryImport));
	return IsLoaded();
}

// Get class name, virtual since each invidiual type will have different name
FString USLCustomStaticMeshComponentIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLCustomStaticMeshComponentIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLCustomStaticMeshComponentIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLCustomStaticMeshComponentIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLCustomStaticMeshComponentIndividual::LoadReset()
{
	SetIsLoaded(false);
}