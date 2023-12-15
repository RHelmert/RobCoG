#include "Individuals/Type/SLParticleIndividual.h"

USLParticleIndividual::USLParticleIndividual() {

}


// Called before destroying the object.
void USLParticleIndividual::BeginDestroy()
{
	SetIsInit(false);
	Super::BeginDestroy();
}

// Set pointer to the semantic owner
bool USLParticleIndividual::Init(bool bReset)
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
bool USLParticleIndividual::Load(bool bReset, bool bTryImport)
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
FString USLParticleIndividual::CalcDefaultClassValue()
{
	return GetTypeName();
}

// Private init implementation
bool USLParticleIndividual::InitImpl()
{
	return true;
}

// Private load implementation
bool USLParticleIndividual::LoadImpl(bool bTryImport)
{
	return true;
}

// Clear all values of the individual
void USLParticleIndividual::InitReset()
{
	SetIsInit(false);
}

// Clear all data of the individual
void USLParticleIndividual::LoadReset()
{
	SetIsLoaded(false);
}