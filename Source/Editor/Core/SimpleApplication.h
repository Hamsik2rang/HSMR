#ifndef __HS_EDITOR_SIMPLE_APPLICATION_H__
#define __HS_EDITOR_SIMPLE_APPLICATION_H__

#include "Precompile.h"

#include "Engine/Application.h"

namespace hs { namespace editor { class GUIContext; } }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API SimpleApplication : public Application
{
public:
    SimpleApplication(const char* appName) noexcept;
    ~SimpleApplication() override;

    void Run() override;
    void Shutdown() override;

    GUIContext* GetGUIContext();

private:
    GUIContext* _guiContext = nullptr;
    float _deltaTime = 0.0f;
};

HS_NS_EDITOR_END

#endif // __HS_EDITOR_SIMPLE_APPLICATION_H__
