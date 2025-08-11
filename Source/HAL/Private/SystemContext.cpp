#include "HAL/SystemContext.h"

HS_NS_BEGIN

SystemContext* SystemContext::Get()
{
    if(!s_instance)
    {
        s_instance = new SystemContext();
    }
    
    return s_instance;
}

bool SystemContext::Initialize()
{
    
    
    
    return true;
}

void SystemContext::Finalize()
{
    
}

HS_NS_END

