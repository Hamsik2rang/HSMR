//
//  GUIContext.h
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_GUI_CONTEXT_H__
#define __HS_GUI_CONTEXT_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

#include "Core/SystemContext.h"

#include <string>

namespace hs
{
class Window;
class Swapchain;
} // namespace hs

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API GUIContext
{
public:
    GUIContext();
    ~GUIContext();

    void Initialize();
    void Finalize();
    void NextFrame();
    void SetColorTheme(bool useWhite);
    void SetFont(const std::string& fontPath, float defaultFontSize = 16.0f);

    HS_FORCEINLINE float GetScaleFactor() const { return _scaleFactor; }
    void SetScaleFactor(float scale);
    void LoadLayout(const std::string& layoutPath);
    void SaveLayout(const std::string& layoutPath);

    void BeginRender(Swapchain* swapchain);
    void EndRender();

private:
    ImGuiContext* _context; // 네임스페이스 없이 사용
    ImFont* _font;
    std::string _assetDirectory;

    float _scaleFactor;
};

HS_NS_EDITOR_END

#endif
