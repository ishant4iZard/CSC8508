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
		const std::string& text,
		const NCL::Maths::Vector2& position,
		const NCL::Maths::Vector4& color = WHITE
	)  override;
	virtual void DrawButton(
		const std::string& text,
		const NCL::Maths::Vector2& position,
		std::function<void(void)> callback,
		const NCL::Maths::Vector4& color = WHITE,
		const NCL::KeyCodes::Type& keyCode = NCL::KeyCodes::RETURN,
		const NCL::Maths::Vector2& size = NCL::Maths::Vector2(360, 50)
	) override;
	void RenderUI()  override;

protected :
	UIWindows();
	~UIWindows();
	ImFont* font;
	static UIWindows* ui;  
	std::vector<UIElementProps*> uiElements;
};