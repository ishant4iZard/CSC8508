#pragma once
#include "Vector4.h"
#include "Vector2.h"
#include "Keyboard.h"
#include <functional>

enum ElementTypes {
	Button,
	Text
};

struct UIElementProps {
	ElementTypes elementType;
	std::string text;
	NCL::Maths::Vector2 position;
	std::function<void(void)> callback; // Only for buttons
	NCL::Maths::Vector4 color;
	NCL::KeyCodes::Type keyCode; // Only for buttons
	NCL::Maths::Vector2 size; // Only for buttons
};

class UIBase {

public:
	virtual void DrawStringText(
		std::string text,
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = WHITE
	) = 0;
	virtual void DrawButton(
		std::string text,
		NCL::Maths::Vector2 position,
		std::function<void(void)> callback,
		NCL::Maths::Vector4 color = WHITE,
		NCL::KeyCodes::Type keyCode = NCL::KeyCodes::RETURN,
		NCL::Maths::Vector2 size = NCL::Maths::Vector2(360, 50)
	) = 0;	
	virtual void RenderUI() = 0;

	static const NCL::Maths::Vector4 RED;
	static const NCL::Maths::Vector4 GREEN;
	static const NCL::Maths::Vector4 BLUE;
	static const NCL::Maths::Vector4 BLACK;
	static const NCL::Maths::Vector4 WHITE;
	static const NCL::Maths::Vector4 YELLOW;
	static const NCL::Maths::Vector4 MAGENTA;
	static const NCL::Maths::Vector4 CYAN;
};