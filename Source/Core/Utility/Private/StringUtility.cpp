#include "Core/Utility/StringUtility.h"

HS_NS_BEGIN
namespace StringUtil
{

std::string Format(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	// Get the size of the formatted string
	int size = vsnprintf(nullptr, 0, fmt, args);
	if (size < 0)
	{
		va_end(args);
		return std::string();
	}
	std::string result(size, '\0');

	vsnprintf(result.data(), size + 1, fmt, args);

	va_end(args);

	return result;
}

}

HS_NS_END
