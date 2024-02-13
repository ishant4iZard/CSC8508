#pragma once
#include "UIBase.h"
#include "Debug.h"
#include "Window.h"
#include <string>
#include <unordered_map>

class UIPlaystation : public UIBase {
public:
	static UIPlaystation* GetInstance();
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

protected:
	UIPlaystation() {};
	~UIPlaystation() {};
	static UIPlaystation* ui;

	std::vector<UIElementProps*> uiElements;
};

extern std::unordered_map<NCL::KeyCodes::Type, std::string> keycodeToStringMap;