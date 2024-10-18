#pragma once

#include <iostream>


namespace Editor {

	class EditorLayer 
	{
	public:
		EditorLayer();
		~EditorLayer();

		void onStart();
		void onUpdate();
		void onDelete();
		void onEvent();

	private:
		void onMouseButtonPress();
		void onMouseButtonRelease();
		void onMouseScroll();
		void onKeyPress();
		void onKeyRelease(); 
		void onKeyTap();

	private:

	};

}