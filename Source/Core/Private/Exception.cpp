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
	: _message(nullptr)
{
	va_list args;
	va_start(args, fmt);

	std::string builder;

	const int fmtLen = _vscprintf(fmt, args);
	if (fmtLen == -1) return;

	const size_t size = static_cast<size_t>(fmtLen + 1);

	if (size < HS_CHAR_INIT_LENGTH)
	{
		char buf[HS_CHAR_INIT_LENGTH] = { 0, };
		vsnprintf(buf, HS_CHAR_INIT_LENGTH, fmt, args);

		builder.append(buf);

		_message = new char(builder.length() + 1);
		memset(_message, 0, builder.length() + 1);
		memcpy(_message, builder.c_str(), sizeof(char) * builder.length());
	}
	else
	{
		char* buf = new char[size];
		if (!buf) return;
		const int r = vsnprintf(buf, fmtLen + 1, fmt, args);
		if (r == -1)
		{
			delete[] buf;
			return;
		}
		builder.append(buf);

		_message = new char[builder.length() + 1];
		memset(_message, 0, builder.length() + 1);
		memcpy(_message, builder.c_str(), sizeof(char) * builder.length());
	}

	va_end(args);

	HS_LOG(debug, "[EXCEPTION] %s (%s:%u)", builder.c_str(), file, line);
}

const char* const Exception::what() const
{
	return _message;
}

Exception::~Exception()
{
	delete _message;
}


HS_NS_END
