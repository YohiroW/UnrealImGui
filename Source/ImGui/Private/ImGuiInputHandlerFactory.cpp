// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInputHandler.h"
#include "Utilities/WorldContextIndex.h"
#include "ImGuiModuleDebug.h"

#include <Engine/GameViewportClient.h>
#include <InputCoreTypes.h>


UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandler(const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, UGameViewportClient* GameViewport, int32 ContextIndex)
{
	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid())
	{
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();

		if (!HandlerClass)
		{
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass)
	{
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(GameViewport, HandlerClass);
	if (Handler)
	{
		Handler->Initialize(ModuleManager, GameViewport, ContextIndex);
		Handler->AddToRoot();
	}
	else
	{
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
}

UImGuiInputHandler* FImGuiInputHandlerFactory::NewEditorWindowHandler(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int32 ContextIndex)
{
#if WITH_EDITOR
	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid())
	{
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();

		if (!HandlerClass)
		{
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass)
	{
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(OuterPkg, HandlerClass);
	if (Handler)
	{
		Handler->Initialize(ModuleManager, nullptr, Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET + ContextIndex);
		Handler->AddToRoot();
	}
	else
	{
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
#else
	return nullptr;
#endif
}

void FImGuiInputHandlerFactory::ReleaseHandler(UImGuiInputHandler* Handler)
{
	if (Handler)
	{
		Handler->RemoveFromRoot();
	}
}
