// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.


#pragma once
#include "GameFramework/ForceFeedbackAttenuation.h"
#include "Components/SceneComponent.h"
#include "Tickable.h"
#include "GCObject.h"
#include "ForceFeedbackComponent.generated.h"

struct FDisplayDebugManager;
struct FForceFeedbackValues;
class UForceFeedbackEffect;

class FForceFeedbackManager : public FTickableGameObject, FGCObject
{
private:

	FForceFeedbackManager(UWorld* InWorld)
		: World(InWorld)
	{
	}

public:

	static FForceFeedbackManager* Get(UWorld* World, bool bCreateIfMissing = false);

	void AddActiveComponent(UForceFeedbackComponent* ForceFeedbackComponent);
	void RemoveActiveComponent(UForceFeedbackComponent* ForceFeedbackComponent);

	void Update(FVector Location, FForceFeedbackValues& Values) const;

	void DrawDebug(const FVector Location, FDisplayDebugManager& DisplayDebugManager) const;

private:
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual void Tick( float DeltaTime ) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	UWorld* World;
	TArray<UForceFeedbackComponent*> ActiveForceFeedbackComponents;

	static void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	static TArray<FForceFeedbackManager*> PerWorldForceFeedbackManagers;
	static FDelegateHandle OnWorldCleanupHandle;
};

/** called when we finish playing forcefeedback effect, either because it played to completion or because a Stop() call turned it off early */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnForceFeedbackFinished, UForceFeedbackComponent*, ForceFeedbackComponent);

/**
 * ForceFeedbackComponent allows placing a rumble effect in to the world and having it apply to player characters who come near it
 */
UCLASS(ClassGroup=(Utility), hidecategories=(Object, ActorComponent, Physics, Rendering, Mobility, LOD), ShowCategories=Trigger, meta=(BlueprintSpawnableComponent))
class ENGINE_API UForceFeedbackComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UForceFeedbackComponent(const FObjectInitializer& ObjectInitializer);

	/** The feedback effect to be played */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ForceFeedback)
	UForceFeedbackEffect* ForceFeedbackEffect;

	/** Auto destroy this component on completion */
	UPROPERTY()
	uint8 bAutoDestroy:1;

	/** Stop effect when owner is destroyed */
	UPROPERTY()
	uint8 bStopWhenOwnerDestroyed:1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ForceFeedback)
	uint8 bLooping:1;

	/** Should the Attenuation Settings asset be used (false) or should the properties set directly on the component be used for attenuation properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attenuation)
	uint8 bOverrideAttenuation:1;

	/** The intensity multiplier to apply to effects generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ForceFeedback)
	float IntensityMultiplier;

	/** If bOverrideSettings is false, the asset to use to determine attenuation properties for effects generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attenuation, meta=(EditCondition="!bOverrideAttenuation"))
	class UForceFeedbackAttenuation* AttenuationSettings;

	/** If bOverrideSettings is true, the attenuation properties to use for effects generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Attenuation, meta=(EditCondition="bOverrideAttenuation"))
	struct FForceFeedbackAttenuationSettings AttenuationOverrides;

	/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
	UPROPERTY(BlueprintAssignable)
	FOnForceFeedbackFinished OnForceFeedbackFinished;

	/** Set what force feedback effect is played by this component */
	UFUNCTION(BlueprintCallable, Category="ForceFeedback")
	void SetForceFeedbackEffect( UForceFeedbackEffect* NewForceFeedbackEffect);

	/** Start a feedback effect playing */
	UFUNCTION(BlueprintCallable, Category="ForceFeedback")
	virtual void Play(float StartTime = 0.f);

	/** Stop playing the feedback effect */
	UFUNCTION(BlueprintCallable, Category="ForceFeedback")
	virtual void Stop();

	/** Set a new intensity multiplier */
	UFUNCTION(BlueprintCallable, Category="ForceFeedback")
	void SetIntensityMultiplier(float NewIntensityMultiplier);

	/** Modify the attenuation settings of the component */
	UFUNCTION(BlueprintCallable, Category="ForceFeedback")
	void AdjustAttenuation(const FForceFeedbackAttenuationSettings& InAttenuationSettings);

	//~ Begin UObject Interface.
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface.

	//~ Begin USceneComponent Interface
	virtual void Activate(bool bReset=false) override;
	virtual void Deactivate() override;
	//~ End USceneComponent Interface

	//~ Begin ActorComponent Interface.
#if WITH_EDITORONLY_DATA
	virtual void OnRegister() override;
#endif
	virtual void OnUnregister() override;
	virtual const UObject* AdditionalStatObject() const override;
	virtual bool IsReadyForOwnerToAutoDestroy() const override;
	//~ End ActorComponent Interface.

	/** Returns a pointer to the attenuation settings to be used (if any) for this audio component dependent on the ForceFeedbackEffectAttenuation asset or overrides set. */
	const FForceFeedbackAttenuationSettings* GetAttenuationSettingsToApply() const;

	UFUNCTION(BlueprintCallable, Category = "ForceFeedback", meta = (DisplayName = "Get Attenuation Settings To Apply"))
	bool BP_GetAttenuationSettingsToApply(FForceFeedbackAttenuationSettings& OutAttenuationSettings) const;

	/** Collects the various attenuation shapes that may be applied to the effect played by the component for visualization in the editor. */
	void CollectAttenuationShapesForVisualization(TMultiMap<EAttenuationShape::Type, FBaseAttenuationSettings::AttenuationShapeDetails>& ShapeDetailsMap) const;

private:
	
	float PlayTime;

#if WITH_EDITORONLY_DATA
	/** Utility function that updates which texture is displayed on the sprite dependent on the properties of the Audio Component. */
	void UpdateSpriteTexture();
#endif

	bool Advance(float DeltaTime);
	void Update(FVector Location, FForceFeedbackValues& Values) const;
	void StopInternal(bool bRemoveFromManager = true);

	friend FForceFeedbackManager;
};



