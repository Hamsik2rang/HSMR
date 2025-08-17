#include "HAL/SystemContext.h"

HS_NS_BEGIN

SystemContext* SystemContext::s_instance;

SystemContext* SystemContext::Get()
{
    if(!s_instance)
    {
        s_instance = new SystemContext();
    }
    
    return s_instance;
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
        delete s_instance;
        s_instance = nullptr;
    }
}


HS_NS_END

