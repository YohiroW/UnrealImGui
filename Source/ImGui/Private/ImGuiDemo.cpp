// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiDemo.h"

#include "ImGuiModuleProperties.h"

#include <implot.h>
#include <CoreGlobals.h>

#define IMGUI_EDITOR_WINDOW_TEST 1

#if IMGUI_EDITOR_WINDOW_TEST
#define IMGUI_PLOT_CONTEXT_INDEX   500
#define IMGUI_CUSTOM_CONTEXT_INDEX 501
#endif

// Demo copied (with minor modifications) from ImGui examples. See https://github.com/ocornut/imgui.
void FImGuiDemo::DrawControls(int32 ContextIndex)
{
	if (Properties.ShowDemo())
	{
		const int32 ContextBit = ContextIndex < 0 ? 0 : 1 << ContextIndex;

		// 1. Show a simple window
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			static float f = 0.0f;
			ImGui::Text("Hello, world!");
			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
			ImGui::ColorEdit3("clear color", (float*)&ClearColor);

			if (ContextBit)
			{
				if (ImGui::Button("Demo Window")) ShowDemoWindowMask ^= ContextBit;
				if (ImGui::Button("Another Window")) ShowAnotherWindowMask ^= ContextBit;
			}
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// 2. Show another simple window, this time using an explicit Begin/End pair
		if (ShowAnotherWindowMask & ContextBit)
		{
			ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
			ImGui::Begin("Another Window");
			ImGui::Text("Hello");
			ImGui::End();
		}

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		if (ShowDemoWindowMask & ContextBit)
		{
			// If more than one demo window is opened display warning about running ImGui examples in multiple contexts.

			// For everything, but the first windows in this frame we assume warning.
			bool bWarning = true;
			if (GFrameNumber > LastDemoWindowFrameNumber)
			{
				// If this is the first window in this frame, then we need to look at the last frame to see whether
				// there were more than one windows. Higher frame distance automatically means that there were not.
				bWarning = ((GFrameNumber - LastDemoWindowFrameNumber) == 1) && (DemoWindowCounter > 1);
	
				LastDemoWindowFrameNumber = GFrameNumber;
				DemoWindowCounter = 0;
			}

			DemoWindowCounter++;

			if (bWarning)
			{
				ImGui::Spacing();

				ImGui::PushStyleColor(ImGuiCol_Text, { 1.f, 1.f, 0.5f, 1.f });				
				ImGui::TextWrapped("Demo Window is opened in more than one context, some of the ImGui examples may not work correctly.");
				ImGui::PopStyleColor();

				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(
						"Some of the ImGui examples that use static variables may not work correctly\n"
						"when run concurrently in multiple contexts.\n"
						"If you have a problem with an example try to run it in one context only.");
				}
			}

			// Draw demo window.
			ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
			ImGui::ShowDemoWindow();
		}
	}
}

// --------------------------------------------------------------------------------------
// @WangYuhao reserved for test

#include "Widgets/SImGuiEditorWidget.h"
#include "Widgets/SImGuiWidget.h"
#include "Widgets/SImGuiLayout.h"

static FAutoConsoleCommand ConsoleCommandImGuiEditorWindowTest(
	TEXT("Imgui.test"),
	TEXT("Invoke dock tab window in editor for debug usage."),
	FConsoleCommandDelegate::CreateStatic([]
	{
		static int32 Index = 0;
		Index++;

		const FString TestIdStr = TEXT("ImguiTest") + FString::FormatAsNumber(Index);
		FName TestId = FName(TestIdStr);

		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TestId, FOnSpawnTab::CreateLambda(
			[TestIdStr](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
			{
				UE_LOG(LogTemp, Warning, TEXT("[Debug] Invoke new tab with id %s"), *TestIdStr);

				return SNew(SDockTab)
					.TabRole(ETabRole::PanelTab)
					[
						SNew(SImGuiEditorWidget)
							.ContextIndex(Index)
							.OnImGui_Lambda(
								[]()
								{
									ImGui::ShowDemoWindow();
									ImPlot::ShowDemoWindow();
								})
					];

			})).SetMenuType(ETabSpawnerMenuType::Hidden);

			FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TestId))->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(
				[](TSharedRef<SDockTab> DockTab)
				{
					const FString TabLabel = DockTab->GetTabLabel().ToString();
					UE_LOG(LogTemp, Warning, TEXT("[Debug] Close tab with id %s"), *TabLabel);
				}));
	}));


void ImGuiDrawCustom();

static FAutoConsoleCommand ConsoleCommandCustomEditorWindowTest(
	TEXT("imgui.custom"),
	TEXT("Invoke dock tab window in editor for debug usage."),
	FConsoleCommandDelegate::CreateStatic([]
		{
			const FString TestIdStr = TEXT("Custom");
			FName TestId = FName(TestIdStr);

			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TestId, FOnSpawnTab::CreateLambda(
				[TestIdStr](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
				{
					UE_LOG(LogTemp, Warning, TEXT("[Debug] Invoke new tab with id %s"), *TestIdStr);

					return SNew(SDockTab)
						.TabRole(ETabRole::NomadTab)
						[
							SNew(SImGuiEditorWidget)
								.ContextIndex(IMGUI_CUSTOM_CONTEXT_INDEX)
								.OnImGui(FOnImGui::CreateStatic(&ImGuiDrawCustom))
						];

				})).SetMenuType(ETabSpawnerMenuType::Hidden);

				FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TestId))->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(
					[](TSharedRef<SDockTab> DockTab)
					{
						const FString TabLabel = DockTab->GetTabLabel().ToString();
						UE_LOG(LogTemp, Warning, TEXT("[Debug] Close tab with id %s"), *TabLabel);
					}));
		}));

#include "Engine/Engine.h"
#include "Performance/EnginePerformanceTargets.h"

extern ENGINE_API float GAverageFPS;

ImVec4 ToVec4(FLinearColor InOther)
{
	return ImVec4(InOther.R, InOther.G, InOther.B, InOther.A);
}

ImVec4 MakeColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255)
{
	return ToVec4(FColor(InR, InG, InB, InA).ReinterpretAsLinear());
}

ImVec4 GetFPSColor(float InFPS)
{
	const float LCurrentMS = 1000 / InFPS;
	const float LUnacceptableMS = FEnginePerformanceTargets::GetUnacceptableFrameTimeThresholdMS();
	const float LTargetMS = FEnginePerformanceTargets::GetTargetFrameTimeThresholdMS();

	if (LCurrentMS > LUnacceptableMS)
	{
		return MakeColor(245, 68, 7);
	}

	if (LCurrentMS > LTargetMS)
	{
		return MakeColor(245, 202, 7);
	}

	return MakeColor(7, 245, 97);
}

struct FLineChartBuffer
{
public:
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;

	FLineChartBuffer(int InMaxSize = 5000)
	{
		MaxSize = InMaxSize;
		Offset = 0;
		Data.reserve(MaxSize);
	}

	void AddPoint(float InX, float InY)
	{
		if (Data.size() < MaxSize)
		{
			Data.push_back(ImVec2(InX, InY));
		}

		else
		{
			Data[Offset] = ImVec2(InX, InY);
			Offset = (Offset + 1) % MaxSize;
		}
	}

	void Erase()
	{	
		if (Data.size() > 0)
		{
			Data.shrink(0);
			Offset = 0;
		}
	}
};

void ImGuiDrawCustom()
{
	if (ImGui::Begin("Custom", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		//Basic
		if (ImGui::TreeNodeEx("Basic", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//FPS
			if (ImGui::TreeNodeEx("FPS", ImGuiTreeNodeFlags_DefaultOpen))
			{
				static float LMinFPS = -1;
				static float LMaxFPS = -1;
				static float LCurrentFPS = -1;
				const static ImVec4 LMinFPSColor = MakeColor(245, 140, 7);
				const static ImVec4 LMaxFPSColor = MakeColor(7, 245, 169);
				const static ImVec4 LCurrentFPSColor = MakeColor(149, 247, 9);

				LMinFPS = LMinFPS < 0 ? GAverageFPS : fmin(LMinFPS, GAverageFPS);
				LMaxFPS = fmax(LMaxFPS, GAverageFPS);
				LCurrentFPS = GAverageFPS;

				ImGui::Separator();

				ImGui::TextColored(LCurrentFPSColor, "Current:");
				ImGui::SameLine();
				ImGui::TextColored(GetFPSColor(LCurrentFPS), "%5.2f", LCurrentFPS);

				ImGui::TextColored(LMinFPSColor, "Minimum:");
				ImGui::SameLine();
				ImGui::TextColored(GetFPSColor(LMinFPS), "%5.2f", LMinFPS);

				ImGui::TextColored(LMaxFPSColor, "Maximum:");
				ImGui::SameLine();
				ImGui::TextColored(GetFPSColor(LMaxFPS), "%5.2f", LMaxFPS);

				ImGui::Separator();

				//Chart
				if (ImGui::TreeNodeEx("Chart", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (ImPlot::BeginPlot("##Basic-Chart", ImVec2(512, 256), ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
					{
						static float LChartTime = 0;
						static FLineChartBuffer LChartMinFPS;
						static FLineChartBuffer LChartMaxFPS;
						static FLineChartBuffer LChartCurrentFPS;

						LChartTime += ImGui::GetIO().DeltaTime * 4;
						LChartMinFPS.AddPoint(LChartTime, LMinFPS);
						LChartMaxFPS.AddPoint(LChartTime, LMaxFPS);
						LChartCurrentFPS.AddPoint(LChartTime, LCurrentFPS);

						ImPlot::SetupAxis(ImAxis_X1, 0, ImPlotAxisFlags_NoDecorations);
						ImPlot::SetupAxisLimits(ImAxis_X1, LChartTime - 20.0, LChartTime, ImGuiCond_Always);
						ImPlot::SetupAxisLimits(ImAxis_Y1, 0, LMaxFPS + 20, ImGuiCond_Always);

						ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.075f);

						if (LChartMaxFPS.Data.size() > 0)
						{
							ImPlot::PushStyleColor(ImPlotCol_Line, LMaxFPSColor);
							ImPlot::PlotLine(TCHAR_TO_ANSI(*FString("Maximum")), &LChartMaxFPS.Data[0].x, &LChartMaxFPS.Data[0].y, LChartMaxFPS.Data.size(), ImPlotLineFlags_Shaded, LChartMaxFPS.Offset, 2 * sizeof(float));
							ImPlot::PopStyleColor();
						}

						if (LChartCurrentFPS.Data.size() > 0)
						{
							ImPlot::PushStyleColor(ImPlotCol_Line, LCurrentFPSColor);
							ImPlot::PlotLine(TCHAR_TO_ANSI(*FString("Current")), &LChartCurrentFPS.Data[0].x, &LChartCurrentFPS.Data[0].y, LChartCurrentFPS.Data.size(), ImPlotLineFlags_Shaded, LChartCurrentFPS.Offset, 2 * sizeof(float));
							ImPlot::PopStyleColor();
						}

						if (LChartMinFPS.Data.size() > 0)
						{
							ImPlot::PushStyleColor(ImPlotCol_Line, LMinFPSColor);
							ImPlot::PlotLine(TCHAR_TO_ANSI(*FString("Minimum")), &LChartMinFPS.Data[0].x, &LChartMinFPS.Data[0].y, LChartMinFPS.Data.size(), ImPlotLineFlags_Shaded, LChartMinFPS.Offset, 2 * sizeof(float));
							ImPlot::PopStyleColor();
						}
						ImPlot::PopStyleVar();
						ImPlot::EndPlot();
					}

					ImGui::TreePop();
				}

				//Actions
				if (ImGui::TreeNode("Actions"))
				{
					static IConsoleVariable* LCVMaxFPS = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));

					if (LCVMaxFPS)
					{
						static float LCVMaxFPSValue = LCVMaxFPS->GetFloat();

						ImGui::Separator();
						ImGui::SliderFloat("Maximum FPS", &LCVMaxFPSValue, 10.f, 300.f, "%.0f");
						ImGui::Separator();

						if (LCVMaxFPSValue != LCVMaxFPS->GetFloat())
						{
							LCVMaxFPS->Set(LCVMaxFPSValue);
						}
					}
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
}
