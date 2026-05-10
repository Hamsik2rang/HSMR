#include "Core/Exception.h"

#include "Core/Log.h"

#include <cstring>
#include <sstream>
#include <cstdarg>

HS_NS_BEGIN

#if defined(__APPLE__)
static int _vscprintf(const char* format, va_list pargs)
{
	int retval;
	va_list argcopy;
	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}
#endif

Exception::Exception(const char* file, unsigned int line, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	va_list argsCopy;
	va_copy(argsCopy, args);
	const int fmtLen = _vscprintf(fmt, argsCopy);
	va_end(argsCopy);

	if (fmtLen >= 0)
	{
		std::vector<char> buffer(static_cast<size_t>(fmtLen) + 1);
		const int r = vsnprintf(buffer.data(), buffer.size(), fmt, args);
		if (r >= 0)
		{
			_message.assign(buffer.data(), static_cast<size_t>(r));
		}
	}

	va_end(args);

	HS_LOG(debug, "[EXCEPTION] %s (%s:%u)", _message.c_str(), file, line);
}

const char* const Exception::what() const
{
	return _message.c_str();
}


HS_NS_END
