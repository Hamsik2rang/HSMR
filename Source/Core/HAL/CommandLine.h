//
//  CommandLine.h
//  HSMR
//
#ifndef __HS_COMMAND_LINE_H__
#define __HS_COMMAND_LINE_H__

#include "Precompile.h"

HS_NS_BEGIN

class HS_CORE_API CommandLine
{
public:
    static void Initialize(int argc, char* argv[]);

    static int GetArgCount();
    static const std::string& GetArg(int index);
    static const std::vector<std::string>& GetArgs();
    static const std::string& GetProgramPath();

    static bool HasFlag(const std::string& flag);
    static std::string GetFlagValue(const std::string& flag);

private:
    static std::string              s_programPath;
    static std::vector<std::string> s_args;
    static bool                     s_initialized;
};

HS_NS_END

#endif // __HS_COMMAND_LINE_H__
