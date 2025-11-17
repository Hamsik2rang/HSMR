#include "Platform/SystemContext.h"

HS_NS_BEGIN

Scoped<SystemContext> SystemContext::s_instance;

SystemContext* SystemContext::Get()
{
    if (!s_instance)
    {
        s_instance = MakeScoped<SystemContext>();
    }

    return s_instance.get();
}

SystemContext::SystemContext()
{
    Initialize();
}

SystemContext::~SystemContext()
{
    if (s_instance)
    {
        Finalize();
    }
}

HS_NS_END
