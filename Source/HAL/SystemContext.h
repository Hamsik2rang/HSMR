//
//  SystemContext.h
//  HAL
//
//  Created by Yongsik Im on 8/9/2025
//
#ifndef __HS_SYSTEM_CONTEXT_H__
#define __HS_SYSTEM_CONTEXT_H__

#include "Precompile.h"


HS_NS_BEGIN

class SystemContext
{
public:
    bool Initialize();
    void Finalize();

    static SystemContext* Get();
    
    std::string executablePath = "";
    std::string executableDirectory = "";
    std::string resourceDirectory = "";

private:
    SystemContext();
    ~SystemContext();
    
    static Scoped<SystemContext> s_instance;
};

HS_NS_END

#endif
