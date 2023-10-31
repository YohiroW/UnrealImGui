#pragma once

#include "Widgets/SImGuiBaseWidget.h"
#include "ImGuiDelegates.h"

// To see if ImGuiContext is actually necessary later
//DECLARE_DELEGATE_OneParam(FOnImGui, FImGuiContextProxy*);
DECLARE_DELEGATE(FOnImGui)

class IMGUI_API SImGuiEditorWidget : public SImGuiBaseWidget
{
	typedef SImGuiBaseWidget Super;

public:
	SLATE_BEGIN_ARGS(SImGuiEditorWidget)
	{}
	SLATE_ARGUMENT(int32, ContextIndex)
	SLATE_EVENT(FSimpleDelegate, OnImGui)
	SLATE_END_ARGS()

	~SImGuiEditorWidget();

	void Construct(const FArguments& InArgs);

	virtual int32 GetContextIndex() const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const override;

private:
	virtual void CreateInputHandler(const FSoftClassPath& HandlerClassReference) override;
	void UpdateInputState();
	UPackage* GetInputHandlerOuterPkg();

	void RegisterDelegates();
	void UnregisterDelegates();

	void OnPostImGuiUpdate();

private:
	// In which ImGui embedded code begin
	FOnImGui OnImGui;
	FImGuiDelegateHandle OnImGuiHandle;
};
