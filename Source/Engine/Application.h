//
//  Application.h
//  Core
//
//  Created by Yongsik Im on 2/7/2025
//
#ifndef __HS_APPLICATION_H__
#define __HS_APPLICATION_H__

#include "Precompile.h"

#include "Core/SystemContext.h"

HS_NS_BEGIN

class RenderPath;
class Scene;
class Window;

class HS_API Application
{
public:
	Application(const char* appName) noexcept;
	virtual ~Application() = default;

	virtual void Run() = 0;
	virtual void Shutdown() = 0;

protected:
	const char* _name;

	Window* _window;
};

HS_NS_END

#endif
