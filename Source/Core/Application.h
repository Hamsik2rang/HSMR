//
//  Application.h
//  Core
//
//  Created by Yongsik Im on 2/7/2025
//
#ifndef __HS_APPLICATION_H__
#define __HS_APPLICATION_H__

#include "Precompile.h"

#include "HAL/SystemContext.h"
/*#include "Engine/EngineContext.h"*/ namespace HS { struct EngineContext; }
/*#include "Renderer/RenderPath.h"*/ namespace HS { class RenderPath; }

HS_NS_BEGIN

class RenderPath;
class Scene;
class Window;

class HS_ENGINE_API Application
{
public:
	Application(const char* appName, EngineContext* engineContext) noexcept;
	virtual ~Application() = default;

	virtual void Run() = 0;
	virtual void Shutdown() = 0;

	EngineContext* GetEngineContext();

protected:
	const char* _name;

	EngineContext* _engineContext;
	Window* _window;
};

HS_NS_END

#endif
