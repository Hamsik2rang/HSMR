#include "Core/HAL/CommandLine.h"

#include "Core/Log.h"

HS_NS_BEGIN

std::string              CommandLine::s_programPath;
std::vector<std::string> CommandLine::s_args;
bool                     CommandLine::s_initialized = false;

void CommandLine::Initialize(int argc, char* argv[])
{
    if (s_initialized)
    {
        return;
    }

    if (argc > 0 && argv[0])
    {
        s_programPath = argv[0];
    }

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i])
        {
            s_args.emplace_back(argv[i]);
        }
    }

    s_initialized = true;
}

int CommandLine::GetArgCount()
{
    return static_cast<int>(s_args.size());
}

const std::string& CommandLine::GetArg(int index)
{
    static const std::string s_empty;
    if (index < 0 || index >= static_cast<int>(s_args.size()))
    {
        HS_LOG(warning, "CommandLine::GetArg: index %d out of range (count: %zu)", index, s_args.size());
        return s_empty;
    }
    return s_args[index];
}

const std::vector<std::string>& CommandLine::GetArgs()
{
    return s_args;
}

const std::string& CommandLine::GetProgramPath()
{
    return s_programPath;
}

bool CommandLine::HasFlag(const std::string& flag)
{
    for (const auto& arg : s_args)
    {
        if (arg == flag)
        {
            return true;
        }
    }
    return false;
}

std::string CommandLine::GetFlagValue(const std::string& flag)
{
    for (size_t i = 0; i < s_args.size(); ++i)
    {
        if (s_args[i] == flag && i + 1 < s_args.size())
        {
            return s_args[i + 1];
        }
    }
    return "";
}

HS_NS_END
