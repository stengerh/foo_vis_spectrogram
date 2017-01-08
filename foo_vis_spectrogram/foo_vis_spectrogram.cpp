#include "stdafx.h"
#include "version.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "CSpectrogramView.h"
#include "CVisualisationHost.h"
#include "config.h"

CComModule _Module;

static void g_activate()
{
	if (!core_api::assert_main_thread()) return;

	cfg_popup_enabled = true;

	if (CVisualisationPopupHost::g_instance == NULL)
	{
		CRect rcMain;
		::GetClientRect(core_api::get_main_window(), &rcMain);
		rcMain.right = rcMain.left + 100;
		rcMain.bottom = rcMain.top + 100;
		CVisualisationPopupHost::g_instance = new CVisualisationPopupHost();
		CVisualisationPopupHost::g_instance->Create(core_api::get_main_window(), rcMain, _T("Spectrogram"));
		CVisualisationPopupHost::g_instance->ShowWindow(SW_SHOW);
	}
	else if (CVisualisationPopupHost::g_instance->IsWindow())
	{
		CVisualisationPopupHost::g_instance->ShowWindow(SW_SHOW);
		::SetForegroundWindow(CVisualisationPopupHost::g_instance->m_hWnd);
	}
}

static const GUID guid_main_vis_spectrum = { 0x79a656c6, 0x2e91, 0x4a4e, { 0xa7, 0xc5, 0x55, 0x22, 0x7c, 0x30, 0x4e, 0x59 } };

class mainmenu_commands_vis_spectrum : public mainmenu_commands
{
public:
	virtual t_uint32 get_command_count()
	{
		return 1;
	}

	virtual GUID get_command(t_uint32 p_index)
	{
		if (p_index == 0)
			return guid_main_vis_spectrum;
		return pfc::guid_null;
	}

	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = "Spectrogram";
	}

	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = "Show spectrogram visualisation.";
		else
			return false;
		return true;
	}

	virtual GUID get_parent()
	{
		return mainmenu_groups::view_visualisations;
	}

	virtual t_uint32 get_sort_priority() {return sort_priority_dontcare;}

	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
	{
		p_flags = 0;
		get_name(p_index,p_text);
		return true;
	}

	virtual void execute(t_uint32 p_index,service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0)
		{
			g_activate();
		}
	}
};

static mainmenu_commands_factory_t<mainmenu_commands_vis_spectrum> foo_main_menu_commands;

class initquit_vis_spectrum : public initquit
{
public:
	virtual void on_init()
	{
		if (cfg_popup_enabled)
			g_activate();
	}

	virtual void on_quit()
	{
		;
	}
};

static initquit_factory_t<initquit_vis_spectrum> foo_initquit;
