#include "src/core/application.h"
#include "src/core/assert.h"

#ifdef VULKAN_BACKEND
#include "platform/Vulkan/VulkanDevice.h"
#include "platform/Vulkan/VulkanWindow.h" 
#include "platform/Vulkan/VulkanRenderer.h" 
#include "platform/Vulkan/resources/VkResourceManager.h"
#endif // VULKAN_BACKEND

namespace Engine
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{   
		ENGINE_ASSERT(!s_Instance, "Application instance already exists!");
		s_Instance = this;  

		SystemClock::instance = new SystemClock(); 

		#ifdef VULKAN_BACKEND 
		{
			Window::instance = new VulkanWindow(WindowConfig());
			Device::instance = new VulkanDevice(); 
			Renderer::instance = new VulkanRenderer(); 
			ResourceManager::instance = new VkResourceManager();
		}
		#else  
		ENGINE_ASSERT(false, "Chose an apropriate backend for the engine in the build system!");
		#endif  

		JobManager::instance = new JobManager();
		
		Window::instance->SetEventCallback(BIND_EVENT(Application::OnEvent));  
		m_time_elapsed = SystemClock::instance->GetTime();
		m_running = true;  
	}

	Application::~Application()
	{  
		ENGINE_CORE_WARN("Shutting down gfx renderer!"); 
		Renderer::instance->ShutDown(); 
		delete Renderer::instance; 

		ENGINE_CORE_WARN("Shutting down gfx Resource Manager!");
		ResourceManager::instance->ShutDown();
		delete ResourceManager::instance;

		ENGINE_CORE_WARN("Shutting down gfx device!");
		Device::instance->ShutDown(); 
		delete Device::instance; 

		ENGINE_CORE_WARN("Shutting down gfx window!");
		Window::instance->ShutDown();
		delete Window::instance; 

		ENGINE_CORE_WARN("Shutting down system clock!");
		SystemClock::instance->ShutDown();
		delete SystemClock::instance; 

		ENGINE_CORE_WARN("Shutting down job manager!");
		JobManager::instance->ShutDown();
		delete JobManager::instance;
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::PushLayer(Layer* l)
	{ 
		m_layers.PushLayer(l);
	} 

	void Application::PushOverlay(Layer* l)
	{ 
		m_layers.PushOverlay(l);
	}

	void Application::RemoveLayer(Layer* l)
	{ 
		m_layers.PopLayer(l);
	}

	void Application::RemoveOverlay(Layer* l)
	{ 
		m_layers.PopOverlay(l);
	}

	void Application::Run()
	{   
		SystemClock::instance->Init();
		Window::instance->Init();
		Device::instance->Init(); 
		ResourceManager::instance->Init();
		Renderer::instance->Init();
		JobManager::instance->Init();
 
		for (auto layer = m_layers.begin(); layer != m_layers.end(); layer++)
		{
			(*layer)->OnStart();
		}

		while (m_running)
		{ 
			uint64_t time = SystemClock::instance->GetTime();
			uint64_t timestep = time - m_time_elapsed;
			m_time_elapsed = time; 

			Window::instance->Update(timestep); 

			for (auto layer = m_layers.begin(); layer != m_layers.end(); layer++)
			{
				(*layer)->OnUpdate();
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{ 
		m_running = false; 
		return true;
	} 

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{  
		ENGINE_CORE_TRACE("Width: {0}, Height: {1}", Window::instance->GetWidth(), Window::instance->GetHeight());
		return true;
	}


	void Application::OnEvent(Event& e)
	{ 
		EventDispatcher dispatcher(e); 
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(Application::OnWindowResize)); 

		for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); layer++)
		{
			if (e.Handled) 
				break;  

			(*layer)->OnEvent(e);
		}
	}
}

