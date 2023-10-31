// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "Widgets/SImGuiBaseWidget.h"
#include "ImGuiModuleSettings.h"
#include "ImGuiModuleDebug.h"

class SImGuiCanvasControl;

class UGameViewportClient;
class ULocalPlayer;

// Slate widget for rendering ImGui output and storing Slate inputs.
class SImGuiWidget : public SImGuiBaseWidget
{
	typedef SImGuiBaseWidget Super;

public:
	SLATE_BEGIN_ARGS(SImGuiWidget)
	{}
	SLATE_ARGUMENT(FImGuiModuleManager*, ModuleManager)
	SLATE_ARGUMENT(UGameViewportClient*, GameViewport)
	SLATE_ARGUMENT(int32, ContextIndex)
	SLATE_END_ARGS()

	~SImGuiWidget();

	void Construct(const FArguments& InArgs);

	// Get index of the context that this widget is targeting.
	virtual int32 GetContextIndex() const override { return ContextIndex; }

	//----------------------------------------------------------------------------------------------------
	// SWidget overrides
	//----------------------------------------------------------------------------------------------------
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual bool SupportsKeyboardFocus() const override { return bInputEnabled && !IsConsoleOpened(); }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent) override;

protected:
	virtual void CreateInputHandler(const FSoftClassPath& HandlerClassReference) override;

	void RegisterImGuiSettingsDelegates();
	void UnregisterImGuiSettingsDelegates();

	bool IsConsoleOpened() const;

	ULocalPlayer* GetLocalPlayer() const;
	void TakeFocus();
	void ReturnFocus();

	// Update input state.
	void UpdateInputState();
	void UpdateTransparentMouseInput(const FGeometry& AllottedGeometry);
	void HandleWindowFocusLost();

	void SetDPIScale(const FImGuiDPIScaleInfo& ScaleInfo);

	void SetCanvasSizeInfo(const FImGuiCanvasSizeInfo& CanvasSizeInfo);
	void UpdateCanvasSize();

	void UpdateCanvasControlMode(const FInputEvent& InputEvent);

	void OnPostImGuiUpdate();

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

#if IMGUI_WIDGET_DEBUG
	void OnDebugDraw();
#endif // IMGUI_WIDGET_DEBUG

private:
	TWeakObjectPtr<UGameViewportClient> GameViewport;

	FVector2D MinCanvasSize = FVector2D::ZeroVector;
	FVector2D CanvasSize = FVector2D::ZeroVector;

	float DPIScale = 1.f;

	bool bForegroundWindow = false;
	bool bTransparentMouseInput = false;
	bool bAdaptiveCanvasSize = false;
	bool bUpdateCanvasSize = false;
	bool bCanvasControlEnabled = false;

	TSharedPtr<SImGuiCanvasControl> CanvasControlWidget;
	TWeakPtr<SWidget> PreviousUserFocusedWidget;
};
