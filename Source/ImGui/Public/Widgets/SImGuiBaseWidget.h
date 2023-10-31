// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include <Rendering/RenderingCommon.h>
#include <UObject/WeakObjectPtr.h>
#include <Widgets/DeclarativeSyntaxSupport.h>
#include <Widgets/SCompoundWidget.h>

// Hide ImGui Widget debug in non-developer mode.
#define IMGUI_WIDGET_DEBUG IMGUI_MODULE_DEVELOPER

enum EImGuiWidgetType
{
	Runtime,
	Editor,
	Unknown    // Indicate uninitialized for now
};

namespace ImGuiWidget
{
	FORCEINLINE FVector2D MaxVector(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y));
	}

	FORCEINLINE FVector2D RoundVector(const FVector2D& Vector)
	{
		return FVector2D(FMath::RoundToFloat(Vector.X), FMath::RoundToFloat(Vector.Y));
	}

	FORCEINLINE FSlateRenderTransform RoundTranslation(const FSlateRenderTransform& Transform)
	{
		return FSlateRenderTransform(Transform.GetMatrix(), RoundVector(Transform.GetTranslation()));
	}
}

class FImGuiModuleManager;
class UImGuiInputHandler;

//
// SImGuiBaseWidget should never be used directly! As input handler need to be create via derived class.
// Be sure InputHandler is initialized in CreateInputHandler once derived class constructed.
//
class IMGUI_API SImGuiBaseWidget : public SCompoundWidget
{
	typedef SCompoundWidget Super;

public:
	SImGuiBaseWidget();
	~SImGuiBaseWidget();

	// Get index of the context that this widget is targeting.
	virtual int32 GetContextIndex() const { return ContextIndex; }

	//----------------------------------------------------------------------------------------------------
	// SWidget overrides
	//----------------------------------------------------------------------------------------------------
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual bool   SupportsKeyboardFocus() const override;
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void   OnMouseLeave(const FPointerEvent& MouseEvent) override;

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent) override;
	virtual void   OnFocusLost(const FFocusEvent& FocusEvent) override;

	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent) override;

	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;
	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent) override;

protected:
	virtual void CreateInputHandler(const FSoftClassPath& HandlerClassReference) = 0;
	virtual void ReleaseInputHandler();

	// Update visibility based on input state.
	void UpdateVisibility();

	// Update cursor based on input state.
	void UpdateMouseCursor();
	void SetHideMouseCursor(bool bHide);

	FVector2D TransformScreenPointToImGui(const FGeometry& MyGeometry, const FVector2D& Point) const;
	FORCEINLINE void SetImGuiTransform(const FSlateRenderTransform& Transform) { ImGuiTransform = Transform; }

protected:
	// Slate args
	FImGuiModuleManager* ModuleManager = nullptr;
	int32 ContextIndex = 0;

	TEnumAsByte<EImGuiWidgetType> ImGuiWidgetType;

	// Input
	bool bInputEnabled = false;
	bool bHideMouseCursor = true;
	bool bTransparentMouseInput = false;
	TWeakObjectPtr<UImGuiInputHandler> InputHandler;

	// Rendering resources
	FSlateRenderTransform ImGuiTransform;
	FSlateRenderTransform ImGuiRenderTransform;
	mutable TArray<FSlateVertex> VertexBuffer;
	mutable TArray<SlateIndex> IndexBuffer;
};
