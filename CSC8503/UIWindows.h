#pragma once
#include "UIBase.h"

#include <windows.h>
#include "Win32Window.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

class UIWindows : public UIBase {
public :	
	~UIWindows();
	static UIWindows* GetInstance();
	void DrawStringText(
		std::string text,
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = UIBase::YELLOW
	)  override;
	void RenderUI()  override;
	bool DrawButton(
		std::string text,
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = UIBase::YELLOW
	) override;

protected :
	UIWindows();
	ImFont* font;
	static UIWindows* ui;  
	std::vector<UIElementProps*> uiElements;
};