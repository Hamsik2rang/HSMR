//
//  SystemContext.h
//  Core
//
//  Created by Yongsik Im on 8/13/25.
//
#ifndef __HS_SYSTEM_CONTEXT_H__
#define __HS_SYSTEM_CONTEXT_H__

#include "Precompile.h"

HS_NS_BEGIN

struct HS_API SystemContext
{
public:
    ~SystemContext();

    std::string executablePath      = "";
    std::string executableDirectory = "";
    std::string assetDirectory      = "";

    static SystemContext* Get();

    static bool Init();

private:
    SystemContext();

    bool initializePlatform();
    void finalizePlatform();

    static SystemContext* s_instance;
};

HS_NS_END

#endif