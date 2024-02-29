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

protected:
	UIPlaystation() {};
	~UIPlaystation() {};
	static UIPlaystation* ui;

	std::vector<UIElementProps*> uiElements;
};

extern std::unordered_map<NCL::KeyCodes::Type, std::string> keycodeToStringMap;