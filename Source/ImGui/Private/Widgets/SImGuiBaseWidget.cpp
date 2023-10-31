#include "Widgets/SImGuiBaseWidget.h"
#include "ImGuiContextManager.h"
#include "ImGuiContextProxy.h"
#include "ImGuiInputHandler.h"
#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInteroperability.h"
#include "ImGuiModuleManager.h"
#include "ImGuiModuleSettings.h"
#include "ImGuiModuleDebug.h"
#include "Utilities/WorldContextIndex.h"

#include "TextureManager.h"
#include "Utilities/Arrays.h"
#include "VersionCompatibility.h"

#include <Engine/Console.h>
#include <Framework/Application/SlateApplication.h>
#include <SlateOptMacros.h>
#include <utility>

#if IMGUI_WIDGET_DEBUG
	DEFINE_LOG_CATEGORY_STATIC(LogImGuiWidget, Warning, All);
	#define IMGUI_WIDGET_LOG(Verbosity, Format, ...) UE_LOG(LogImGuiWidget, Verbosity, Format, __VA_ARGS__)

#else
	#define IMGUI_WIDGET_LOG(...)

#endif // IMGUI_WIDGET_DEBUG

SImGuiBaseWidget::SImGuiBaseWidget()
{
	ImGuiWidgetType = EImGuiWidgetType::Unknown;
}

SImGuiBaseWidget::~SImGuiBaseWidget()
{
	ReleaseInputHandler();
}

void SImGuiBaseWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

bool SImGuiBaseWidget::SupportsKeyboardFocus() const
{
	return bInputEnabled;
}

FReply SImGuiBaseWidget::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent)
{
	return InputHandler->OnKeyChar(CharacterEvent);
}

FReply SImGuiBaseWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	return InputHandler->OnKeyDown(KeyEvent);
}

FReply SImGuiBaseWidget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	return InputHandler->OnKeyUp(KeyEvent);
}

FReply SImGuiBaseWidget::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent)
{
	return InputHandler->OnAnalogValueChanged(AnalogInputEvent);
}

FReply SImGuiBaseWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDown(MouseEvent);
}

FReply SImGuiBaseWidget::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseButtonDoubleClick(MouseEvent);
}

FReply SImGuiBaseWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = InputHandler->OnMouseButtonUp(MouseEvent);
	Reply.ReleaseMouseLock();

	return Reply;
}

FReply SImGuiBaseWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseWheel(MouseEvent);
}

FReply SImGuiBaseWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseMove(TransformScreenPointToImGui(MyGeometry, MouseEvent.GetScreenSpacePosition()), MouseEvent);
}

void SImGuiBaseWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{	
	Super::OnMouseEnter(MyGeometry, MouseEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %d - Mouse Enter."), ContextIndex);

	InputHandler->OnMouseInputEnabled();
}

void SImGuiBaseWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %d - Mouse Leave."), ContextIndex);

	InputHandler->OnMouseInputDisabled();
}

FReply SImGuiBaseWidget::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent)
{
	Super::OnFocusReceived(MyGeometry, FocusEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %d - Focus Received."), ContextIndex);

	InputHandler->OnKeyboardInputEnabled();
	InputHandler->OnGamepadInputEnabled();

	FSlateApplication::Get().ResetToDefaultPointerInputSettings();
	return FReply::Handled();
}

void SImGuiBaseWidget::OnFocusLost(const FFocusEvent& FocusEvent)
{
	Super::OnFocusLost(FocusEvent);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %d - Focus Lost."), ContextIndex);

	InputHandler->OnKeyboardInputDisabled();
	InputHandler->OnGamepadInputDisabled();
}

FReply SImGuiBaseWidget::OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchStarted(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiBaseWidget::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	return InputHandler->OnTouchMoved(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

FReply SImGuiBaseWidget::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
	UpdateVisibility();
	return InputHandler->OnTouchEnded(TransformScreenPointToImGui(MyGeometry, TouchEvent.GetScreenSpacePosition()), TouchEvent);
}

void SImGuiBaseWidget::ReleaseInputHandler()
{
	if (InputHandler.IsValid())
	{
		FImGuiInputHandlerFactory::ReleaseHandler(InputHandler.Get());
		InputHandler.Reset();
	}
}

void SImGuiBaseWidget::UpdateVisibility()
{
	// Make sure that we do not occlude other widgets, if input is disabled or if mouse is set to work in a transparent
	// mode (hit-test invisible).
	SetVisibility(bInputEnabled && !bTransparentMouseInput ? EVisibility::Visible : EVisibility::HitTestInvisible);

	IMGUI_WIDGET_LOG(VeryVerbose, TEXT("ImGui Widget %d - Visibility updated to '%s'."),
					 ContextIndex, *GetVisibility().ToString());

}

void SImGuiBaseWidget::SetHideMouseCursor(bool bHide)
{
	if (bHideMouseCursor != bHide)
	{
		bHideMouseCursor = bHide;
		UpdateMouseCursor();
	}
}

void SImGuiBaseWidget::UpdateMouseCursor()
{
	if (!bHideMouseCursor)
	{
		const FImGuiContextProxy* ContextProxy = ModuleManager->GetContextManager().GetContextProxy(ContextIndex);
		SetCursor(ContextProxy ? ContextProxy->GetMouseCursor() : EMouseCursor::Default);
	}
	else
	{
		SetCursor(EMouseCursor::None);
	}
}

FVector2D SImGuiBaseWidget::TransformScreenPointToImGui(const FGeometry& MyGeometry, const FVector2D& Point) const
{
	const FSlateRenderTransform ImGuiToScreen = ImGuiTransform.Concatenate(MyGeometry.GetAccumulatedRenderTransform());
	return ImGuiToScreen.Inverse().TransformPoint(Point);
}
