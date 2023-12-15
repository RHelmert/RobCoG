#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "SLCustomStaticMeshComponentIndividual.generated.h"


/**
 *
 */
UCLASS(ClassGroup = SL)
class USEMLOG_API USLCustomStaticMeshComponentIndividual : public USLBaseIndividual
{
    GENERATED_BODY()

public:
    // Ctor
    USLCustomStaticMeshComponentIndividual();

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    // Init asset references (bForced forces re-initialization)
    virtual bool Init(bool bReset);

    // Load semantic data (bForced forces re-loading)
    virtual bool Load(bool bReset, bool bTryImport);

    // Get the type name as string
    virtual FString GetTypeName() const override { return FString("CustomStaticMeshComponentIndividual"); };

protected:
    // Get class name, virtual since each invidiual type will have different name
    virtual FString CalcDefaultClassValue() override;

private:
    // Set dependencies
    bool InitImpl();

    // Set data
    bool LoadImpl(bool bTryImport);

    // Clear all values of the individual
    void InitReset();

    // Clear all data of the individual
    void LoadReset();
};
