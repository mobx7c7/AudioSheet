#pragma once
#include <ostream>
#include <string>
#include <iomanip>
#include "SoundPlayer\AVSoundPlayer.h"
#include <glm/glm.hpp>

namespace StreamUtils
{
	namespace Metrics
	{
		struct Dimension
		{
			glm::vec2 value;
			std::string postfix;
			Dimension(const glm::vec2& value, std::string postfix = "") : value(value), postfix(postfix) {}
			friend std::ostream& operator<<(std::ostream& os, const Dimension& d)
			{
				return os << d.value.x << " x " << d.value.y << " " << d.postfix;
			}
		};

		template<typename T, typename = std::enable_if<std::is_pod<T>::value>::type>
		struct Value
		{
			T value;
			std::string unit;
			Value(const T& value, std::string postfix = "") : value(value), unit(postfix) {}
			friend std::ostream& operator<<(std::ostream& os, const Value& u)
			{
				return os << std::setprecision(2) << std::fixed << u.value << " " << u.unit;
			}
		};
	}
}
