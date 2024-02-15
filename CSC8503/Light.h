#pragma once
#include <Vector3.h>
#include <Vector4.h>
namespace NCL 
{
	namespace CSC8503 
	{
		class Light
		{
		protected:
			NCL::Maths::Vector4 color;
		public:
			Light(const NCL::Maths::Vector4& inColor)
			{
				color = inColor;
			}

			const NCL::Maths::Vector4& GetColor()  { return color; }
			void SetColor(const NCL::Maths::Vector4& inColor) { color = inColor; }
		};
	}
}