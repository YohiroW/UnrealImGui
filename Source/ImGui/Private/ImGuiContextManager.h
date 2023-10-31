// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiContextProxy.h"
#include "VersionCompatibility.h"


class FImGuiModuleSettings;
struct FImGuiDPIScaleInfo;

// TODO: It might be useful to broadcast FContextProxyCreatedDelegate to users, to support similar cases to our ImGui
// demo, but we would need to remove from that interface internal classes.

// Delegate called when new context proxy is created.
// @param ContextIndex - Index for that world
// @param ContextProxy - Created context proxy
DECLARE_MULTICAST_DELEGATE_TwoParams(FContextProxyCreatedDelegate, int32, FImGuiContextProxy&);

// Manages ImGui context proxies.
class FImGuiContextManager
{
public:

	FImGuiContextManager(FImGuiModuleSettings& InSettings);

	FImGuiContextManager(const FImGuiContextManager&) = delete;
	FImGuiContextManager& operator=(const FImGuiContextManager&) = delete;

	FImGuiContextManager(FImGuiContextManager&&) = delete;
	FImGuiContextManager& operator=(FImGuiContextManager&&) = delete;

	~FImGuiContextManager();

	ImFontAtlas& GetFontAtlas() { return FontAtlas; }
	const ImFontAtlas& GetFontAtlas() const { return FontAtlas; }

#if WITH_EDITOR
	// Get or create editor ImGui context proxy.
	FORCEINLINE FImGuiContextProxy& GetEditorContextProxy() { return *GetEditorContextData().ContextProxy; }

	// Get or create editor window ImGui context proxy.
	FORCEINLINE FImGuiContextProxy& GetEditorWindowContextProxy(int32 Index) { return *GetEditorWindowContextData(Index).ContextProxy; }
#endif

#if !WITH_EDITOR
	// Get or create standalone game ImGui context proxy.
	FORCEINLINE FImGuiContextProxy& GetWorldContextProxy() { return *GetStandaloneWorldContextData().ContextProxy; }
#endif

	// Get or create ImGui context proxy for given world.
	FORCEINLINE FImGuiContextProxy& GetWorldContextProxy(const UWorld& World) { return *GetWorldContextData(World).ContextProxy; }

	// Get or create ImGui context proxy for given world. Additionally get context index for that proxy.
	FORCEINLINE FImGuiContextProxy& GetWorldContextProxy(const UWorld& World, int32& OutContextIndex) { return *GetWorldContextData(World, &OutContextIndex).ContextProxy; }

	// Get context proxy by index, or null if context with that index doesn't exist.
	FORCEINLINE FImGuiContextProxy* GetContextProxy(const int32& ContextIndex)
	{
		FContextData* Data = Contexts.Find(ContextIndex);
		return Data ? Data->ContextProxy.Get() : nullptr;
	}

	// Used to remove context data
	FORCEINLINE void RemoveContext(const int32& ContextIndex) { Contexts.Remove(ContextIndex); }

	// Delegate called when a new context proxy is created.
	FContextProxyCreatedDelegate OnContextProxyCreated;

	// Delegate called after font atlas is built.
	FSimpleMulticastDelegate OnFontAtlasBuilt;

	void Tick(float DeltaSeconds);

	void RebuildFontAtlas();

private:

	struct FContextData
	{
		FContextData(const FString& ContextName, int32 ContextIndex, ImFontAtlas& FontAtlas, float DPIScale, int32 InPIEInstance = -1)
			: PIEInstance(InPIEInstance)
			, ContextProxy(new FImGuiContextProxy(ContextName, ContextIndex, &FontAtlas, DPIScale))
		{
		}

		FORCEINLINE bool CanTick() const { return PIEInstance < 0 || GEngine->GetWorldContextFromPIEInstance(PIEInstance); }

		int32 PIEInstance = -1;
		TUniquePtr<FImGuiContextProxy> ContextProxy;
	};

#if ENGINE_COMPATIBILITY_LEGACY_WORLD_ACTOR_TICK
	void OnWorldTickStart(ELevelTick TickType, float DeltaSeconds);
#endif
	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);

#if ENGINE_COMPATIBILITY_WITH_WORLD_POST_ACTOR_TICK
	void OnWorldPostActorTick(UWorld* World, ELevelTick TickType, float DeltaSeconds);
#endif

#if WITH_EDITOR
	FContextData& GetEditorContextData();
	FContextData& GetEditorWindowContextData(int32 Index);
#endif

#if !WITH_EDITOR
	FContextData& GetStandaloneWorldContextData();
#endif

	FContextData& GetWorldContextData(const UWorld& World, int32* OutContextIndex = nullptr);

	void SetDPIScale(const FImGuiDPIScaleInfo& ScaleInfo);
	void BuildFontAtlas(const TMap<FName, TSharedPtr<ImFontConfig>>& CustomFontConfigs = {});
	
	// Used as a fix for missed character like Chinese/Japanese
	void AddExtraFontAtlas();

	TMap<int32, FContextData> Contexts;

	ImFontAtlas FontAtlas;
	TArray<TUniquePtr<ImFontAtlas>> FontResourcesToRelease;

	FImGuiModuleSettings& Settings;

	float DPIScale = -1.f;
	int32 FontResourcesReleaseCountdown = 0;
};
