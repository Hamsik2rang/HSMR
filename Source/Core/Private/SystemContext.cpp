#include "Core/SystemContext.h"

HS_NS_BEGIN

SystemContext* SystemContext::s_instance = nullptr;

SystemContext::SystemContext()
{
    initializePlatform();
}

SystemContext::~SystemContext()
{
    finalizePlatform();
}

SystemContext* SystemContext::Get()
{
    return s_instance;
}

bool SystemContext::Init()
{
    if (s_instance == nullptr)
    {
        s_instance = new SystemContext();
        return true;
    }
	return false;
}

HS_NS_END