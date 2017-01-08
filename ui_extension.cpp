#include "stdafx.h"

#ifdef HAVE_COLUMNSUI_SUPPORT

#include "../columns_ui-sdk/ui_extension.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "CSpectrumView.h"
#include "config.h"
#include "CVisualisationHost.h"

class uie_window_spectrum : public uie::window
{
public:
	uie_window_spectrum()
	{
		m_panel = NULL;
	}

	const GUID & get_extension_guid(void) const
	{
		// {1DA52098-6A6C-439a-8573-283B1A653C62}
		static const GUID guid = 
		{ 0x1da52098, 0x6a6c, 0x439a, { 0x85, 0x73, 0x28, 0x3b, 0x1a, 0x65, 0x3c, 0x62 } };

		return guid;
	}

  void get_name(pfc::string_base & p_out) const
	{
		p_out = "Spectrum";
	}

  void get_category(pfc::string_base & p_out) const
	{
		p_out = "Visualisations";
	}

  bool is_available(const ui_extension::window_host_ptr &) const
	{
		return true;
	}

  HWND create_or_transfer_window(HWND parent ,const ui_extension::window_host_ptr & p_host,const ui_helpers::window_position_t & p_position)
	{
		if (m_panel == NULL)
		{
			m_panel = new CVisualisationPanelHost();
			CRect rc;
			p_position.convert_to_rect(rc);
			return m_panel->Create(parent, rc);
		}
		return NULL;
	}

  unsigned int get_type(void) const
	{
		return uie::type_panel;
	}

  void destroy_window(void)
	{
		if (m_panel != NULL && m_panel->IsWindow())
		{
			m_panel->DestroyWindow();
		}
		m_panel = NULL;
	}

  HWND get_wnd(void) const
	{
		if (m_panel != NULL && m_panel->IsWindow())
			return m_panel->m_hWnd;
		return NULL;
	}

private:
	CVisualisationPanelHost * m_panel;
};

static uie::window_factory<uie_window_spectrum> foo_uie_window;

#endif
