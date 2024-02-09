#pragma once
#include "Vector4.h"
#include "Vector2.h"

enum ElementTypes {
	Button,
	Text
};

struct UIElementProps {
	ElementTypes elementType;
	std::string text;
	NCL::Maths::Vector2 position;
	NCL::Maths::Vector4 color;
};

class UIBase {

public:
	virtual void DrawStringText(
		std::string text,
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = YELLOW
	) = 0;
	virtual bool DrawButton(
		std::string text, 
		NCL::Maths::Vector2 position,
		NCL::Maths::Vector4 color = YELLOW
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