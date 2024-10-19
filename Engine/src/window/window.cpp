#include <window/window.h>
#include <window/windowAPI.h>

namespace Engine
{
	Scope<Window> Window::Create(const WindowConfig& config)
	{
		return MakeScope<WindowAPI>(config);
	}
}
