#pragma once
#include <vector>
#include <Vector3.h>
namespace NCL {
	using namespace NCL::Maths;
	class Helper
	{
	public:
		template<typename T>
		static T GetRandomDataFromVector(const std::vector<T>& vec) 
		{
			// Seed the random number generator
			std::srand(static_cast<unsigned int>(std::time(nullptr)));

			// Generate a random index within the bounds of the vector
			int randomIndex = rand() % vec.size();

			// Return the data at the random index
			return vec[randomIndex];
		}

		static int GetRandomEnumValue(const int& inMaxEnum) 
		{
			std::srand(static_cast<unsigned int>(std::time(nullptr)));
			return (std::rand() % inMaxEnum);
		}

		// Template function to clamp a value between a minimum and maximum
		template<typename T>
		static const T& Clamp(const T& value, const T& min, const T& max) {
			return std::max(min, std::min(value, max));
		}

	protected:
		Helper() {}
		~Helper() {}
	};
}