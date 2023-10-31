#include "Widgets/SImGuiEditorWidget.h"

#include "ImGuiModule.h"
#include "ImGuiContextManager.h"
#include "ImGuiContextProxy.h"
#include "ImGuiInputHandler.h"
#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInteroperability.h"
#include "ImGuiModuleManager.h"
#include "TextureManager.h"
#include "Utilities/Arrays.h"
#include "VersionCompatibility.h"

#include <Framework/Application/SlateApplication.h>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SImGuiEditorWidget::Construct(const FArguments& InArgs)
{
	ModuleManager = GetImGuiModuleManager();
	ImGuiWidgetType = EImGuiWidgetType::Editor;

	// Register to context manager
	ContextIndex = InArgs._ContextIndex;
	OnImGui = InArgs._OnImGui;

	RegisterDelegates();

	ModuleManager->GetProperties().SetInputEnabledInEditor(true);
	ModuleManager->LoadTextures();

	const auto& Settings = ModuleManager->GetSettings();
	CreateInputHandler(Settings.GetImGuiInputHandlerClass());
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SImGuiEditorWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UpdateInputState();
}

SImGuiEditorWidget::~SImGuiEditorWidget()
{
	ModuleManager->GetProperties().SetInputEnabledInEditor(false);
	ReleaseInputHandler();

	UnregisterDelegates();
}

int32 SImGuiEditorWidget::GetContextIndex() const
{
	return Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET+ ContextIndex;
}

void SImGuiEditorWidget::CreateInputHandler(const FSoftClassPath& HandlerClassReference)
{
	ReleaseInputHandler();

	if (!InputHandler.IsValid())
	{
		InputHandler = FImGuiInputHandlerFactory::NewEditorWindowHandler(GetInputHandlerOuterPkg(), HandlerClassReference, ModuleManager, ContextIndex);
	}
}

UPackage* SImGuiEditorWidget::GetInputHandlerOuterPkg()
{
	static UPackage* InputHandlerOuterPkg = nullptr;
	if (InputHandlerOuterPkg == nullptr)
	{
		// Used a dummy asset package as handler class reference to create FImGuiInputHandler
		InputHandlerOuterPkg = CreatePackage(TEXT("/Temp/InputHandlerOuter"));
	}

	return InputHandlerOuterPkg;
}

void SImGuiEditorWidget::RegisterDelegates()
{
	// Register setting delegates here

	if (ModuleManager)
	{
		// In manager we use actual context index (plus with offset)
		if (!ModuleManager->GetContextManager().GetContextProxy(GetContextIndex()))
		{
			OnImGuiHandle = FImGuiModule::Get().AddEditorWindowImGuiDelegate(OnImGui, ContextIndex);
		}
	}

	ModuleManager->OnPostImGuiUpdate().AddRaw(this, &SImGuiEditorWidget::OnPostImGuiUpdate);
}

void SImGuiEditorWidget::UnregisterDelegates()
{
	// Unregister setting delegates here
	ModuleManager->OnPostImGuiUpdate().RemoveAll(this);

	// Just cache current context for editor window might reopen.
	//if (OnImGuiHandle.IsValid())
	//{
	//	FImGuiModule::Get().RemoveImGuiDelegate(OnImGuiHandle);
	//	OnImGuiHandle.Reset();
	//}
}

void SImGuiEditorWidget::UpdateInputState()
{
	auto& Properties = ModuleManager->GetProperties();

	bInputEnabled = Properties.IsInputEnabledInEditor();
}

FReply SImGuiEditorWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FSlateRenderTransform ImGuiToScreen = MyGeometry.GetAccumulatedRenderTransform();
	FVector2D OnScreenPos = static_cast<FVector2D>(ImGuiToScreen.Inverse().TransformPoint(MouseEvent.GetScreenSpacePosition()));
	return InputHandler->OnMouseMove(OnScreenPos, MouseEvent);
}

void SImGuiEditorWidget::OnPostImGuiUpdate()
{

}

int32 SImGuiEditorWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const
{
#if WITH_EDITOR
	FImGuiContextProxy& ContextProxy = ModuleManager->GetContextManager().GetEditorWindowContextProxy(ContextIndex);
	{
		ContextProxy.Tick(FSlateApplication::Get().GetDeltaTime());

		float Width = FMath::Abs(MyClippingRect.Right - MyClippingRect.Left);
		float Height = FMath::Abs(MyClippingRect.Bottom - MyClippingRect.Top);
		FVector2D Dimension = FVector2D(Width, Height);
		ContextProxy.SetNextWindowSize(Dimension);
		ContextProxy.SetDisplaySize(Dimension);

		const FSlateRenderTransform& WidgetToScreen = AllottedGeometry.GetAccumulatedRenderTransform();
		const FSlateRenderTransform ImGuiToScreen = ImGuiWidget::RoundTranslation(ImGuiRenderTransform.Concatenate(WidgetToScreen));

		for (const auto& DrawList : ContextProxy.GetDrawData())
		{
			DrawList.CopyVertexData(VertexBuffer, ImGuiToScreen);
			int IndexBufferOffset = 0;
			for (int CommandNb = 0; CommandNb < DrawList.NumCommands(); CommandNb++)
			{
				const auto& DrawCommand = DrawList.GetCommand(CommandNb, ImGuiToScreen);

				DrawList.CopyIndexData(IndexBuffer, IndexBufferOffset, DrawCommand.NumElements);

				IndexBufferOffset += DrawCommand.NumElements;

				const FSlateResourceHandle& handle = ModuleManager->GetTextureManager().GetTextureHandle(DrawCommand.TextureId);

				const FSlateRect ClippingRect = DrawCommand.ClippingRect.IntersectionWith(MyClippingRect);

				OutDrawElements.PushClip(FSlateClippingZone{ ClippingRect });

				FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, handle, VertexBuffer, IndexBuffer, nullptr, 0, 0);

				OutDrawElements.PopClip();
			}
		}
	}
#endif
	return Super::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, WidgetStyle, bParentEnabled);
}
