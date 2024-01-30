#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "SimpleFont.h"

namespace NCL {
	using namespace NCL::Maths;
	using namespace NCL::Rendering;
	class Debug
	{
	public:
		struct DebugStringEntry {
			std::string	data;
			Vector2 position;
			Vector4 colour;
		};

		struct DebugLineEntry {
			Vector3 start;
			float	padding;
			Vector4 colourA;
			Vector3 end;
			float	time;
			Vector4 colourB;
		};

		static void Print(const std::string& text, const Vector2& pos, const Vector4& colour = Vector4(1, 1, 1, 1));
		static void DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);

		static void DrawAxisLines(const Matrix4& modelMatrix, float scaleBoost = 1.0f, float time = 0.0f);

		static void UpdateRenderables(float dt);

		static SimpleFont* GetDebugFont();

		static void CreateDebugFont(const std::string& dataFile, Texture& tex);

		static const std::vector<DebugStringEntry>& GetDebugStrings();
		static const std::vector<DebugLineEntry>& GetDebugLines();


		static const Vector4 RED;
		static const Vector4 GREEN;
		static const Vector4 BLUE;

		static const Vector4 BLACK;
		static const Vector4 WHITE;

		static const Vector4 YELLOW;
		static const Vector4 MAGENTA;
		static const Vector4 CYAN;

	protected:
		Debug() {}
		~Debug() {}

		static std::vector<DebugStringEntry>	stringEntries;
		static std::vector<DebugLineEntry>		lineEntries;

		static SimpleFont* debugFont;
		static Texture* fontTexture;
	};
}

