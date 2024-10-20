#include <events/event.h> 

namespace Engine
{
	std::string Event::ToString() const
	{
		return GetName();
	} 

	bool Event::IsInCategory(EventCategory category)
	{
		return GetCategoryFlags() & category;
	}
}