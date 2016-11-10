#pragma once

#include "Window.h"
#include "Render.h"
#include "InputMgr.h"
#include "Log.h"

namespace D3D11Framework
{
	struct FrameworkDesc
	{
		DescWindow wnd;
		Render *render;
	};

	class Framework
	{
	public:
		Framework();
		~Framework();

		bool Init(const FrameworkDesc &desc);
		void Run();
		void Close();

		void AddInputListener(InputListener *listener);

	protected:
		bool m_frame();

		FrameworkDesc m_desc;
		Window *m_wnd;
		Render *m_render;
		InputMgr *m_input;
		Log m_log;
		bool m_init; // если было инициализировано
	};
}