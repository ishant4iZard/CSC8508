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
	static UIWindows* GetInstance();
	static void Destroy();
	void DrawStringText(
		std::string text,
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = UIBase::WHITE
	)  override;
	void RenderUI()  override;
	void DrawButton(
		std::string text,
		NCL::Maths::Vector2 position,
		std::function<void(void)> callback,
		NCL::Maths::Vector4 color = UIBase::WHITE,
		NCL::KeyCodes::Type keyCode = NCL::KeyCodes::RETURN // Keycode is NOT required for buttons created with imgui
	) override;

protected :
	UIWindows();
	~UIWindows();
	ImFont* font;
	static UIWindows* ui;  
	std::vector<UIElementProps*> uiElements;
};