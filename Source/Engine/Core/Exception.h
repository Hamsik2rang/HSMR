#ifndef __HS_EXCEPTION_H__
#define __HS_EXCEPTION_H__

#include "Precompile.h"

HS_NS_BEGIN

class Exception
{
public:
	Exception(const char* file, unsigned int line, const char* fmt, ...);
	~Exception();

	const char* const what() const;

private:
	char* _message;

};

HS_NS_END

#endif