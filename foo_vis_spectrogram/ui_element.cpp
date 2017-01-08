#include "stdafx.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "CSpectrogramView.h"
#include "config.h"
#include "CVisualisationHost.h"

// {91BFE9EF-879B-4534-A930-51A8C6CE88B1}
static const GUID guid_element_spectrogram = 
{ 0x91bfe9ef, 0x879b, 0x4534, { 0xa9, 0x30, 0x51, 0xa8, 0xc6, 0xce, 0x88, 0xb1 } };

class spectrogram_instance : public ui_element_instance
{
	CVisualisationPanelHost m_wndView;
public:
	spectrogram_instance(HWND p_parent, ui_element_instance_callback_ptr p_callback) {
		m_wndView.set_callback(p_callback);
		HWND hWnd = m_wndView.Create(p_parent, 0, _T("Spectrogram"));
	}

	~spectrogram_instance() {
		if (m_wndView.IsWindow()) {
			m_wndView.DestroyWindow();
		}
	}

	virtual HWND get_wnd() {
		return m_wndView;
	}

	virtual void set_configuration(ui_element_config::ptr cfg) {
		ui_element_config_parser parser(cfg);
		t_uint32 flags = 0;
		try
		{
			parser.read_int(flags);
		}
		catch (const exception_io &)
		{
		}
		m_wndView.SetReduceBandingMode((flags & 1) != 0);
		m_wndView.SetSmoothScalingMode((flags & 2) != 0);
	}

	virtual ui_element_config::ptr get_configuration() {
		ui_element_config_builder builder;
		t_uint32 flags = 0;
		if (m_wndView.GetReduceBandingMode())
			flags |= 1;
		if (m_wndView.GetSmoothScalingMode())
			flags |= 2;
		builder.write_int(flags);
		return builder.finish(get_guid());
	}

	virtual GUID get_guid() {
		return guid_element_spectrogram;
	}

	virtual GUID get_subclass()
	{
		return ui_element_subclass_playback_visualisation;
	}

	//! Query whether instance wants to extend context menu in edit mode.
	//! @param p_point Context menu point in screen coordinates. Always within our window's rectangle.
	//! @return True to request edit_mode_context_menu_build() call to add our own items to the menu,
	//! false if we can't supply a context menu for this point.
	virtual bool edit_mode_context_menu_test(const POINT & p_point,bool p_fromkeyboard)
	{
		return true;
	}

	enum {
		ID_SETTINGS = 1,
		ID_REDUCE_BANDING,
		ID_SMOOTH_SCALING,
	};

	virtual void edit_mode_context_menu_build(const POINT & p_point, bool p_fromkeyboard, HMENU p_menu, unsigned p_id_base)
	{
		AppendMenu(p_menu, MF_STRING, p_id_base + ID_SETTINGS, _T("Settings..."));
#ifdef HAVE_REDUCE_BANDING_OPTION
		AppendMenu(p_menu, MF_STRING | (m_wndView.GetReduceBandingMode() ? MF_CHECKED : 0), p_id_base + ID_REDUCE_BANDING, _T("Reduce Banding"));
#endif
#ifdef HAVE_SMOOTH_SCALING_OPTION
		AppendMenu(p_menu, MF_STRING | (m_wndView.GetSmoothScalingMode() ? MF_CHECKED : 0), p_id_base + ID_SMOOTH_SCALING, _T("Smooth Scaling"));
#endif
	}

	virtual void edit_mode_context_menu_command(const POINT & p_point, bool p_fromkeyboard, unsigned p_id, unsigned p_id_base)
	{
		switch (p_id - p_id_base)
		{
		case ID_SETTINGS:
			static_api_ptr_t<ui_control>()->show_preferences(guid_prefs_vis_spectrum);
			break;
#ifdef HAVE_REDUCE_BANDING_OPTION
		case ID_REDUCE_BANDING:
			m_wndView.SetReduceBandingMode(!m_wndView.GetReduceBandingMode());
			break;
#endif
#ifdef HAVE_SMOOTH_SCALING_OPTION
		case ID_SMOOTH_SCALING:
			m_wndView.SetSmoothScalingMode(!m_wndView.GetSmoothScalingMode());
			break;
#endif
		}
	}

	//! @param p_point Receives the point to spawn context menu over when user has pressed the context menu key; in screen coordinates.
	virtual bool edit_mode_context_menu_get_focus_point(POINT & p_point) {return false;}

	virtual bool edit_mode_context_menu_get_description(unsigned p_id, unsigned p_id_base, pfc::string_base & p_out)
	{
		switch (p_id - p_id_base)
		{
		case ID_SETTINGS:
			p_out = "Shows settings for spectrogram visualisation.";
			return true;

#ifdef HAVE_REDUCE_BANDING_OPTION
		case ID_REDUCE_BANDING:
			p_out = "Enables postprocessing to reduce visual banding artifacts.";
			return true;
#endif

#ifdef HAVE_SMOOTH_SCALING_OPTION
		case ID_SMOOTH_SCALING:
			return false;
#endif

		default:
			return false;
		}
	}
};

class spectrogram : public ui_element {
public:
	virtual GUID get_guid() {
		return guid_element_spectrogram;
	}
	
	virtual GUID get_subclass()
	{
		return ui_element_subclass_playback_visualisation;
	}

	virtual void get_name(pfc::string_base & p_out) {
		p_out = "Spectrogram by foosion";
	}

	virtual ui_element_instance_ptr instantiate(HWND p_parent, ui_element_config::ptr cfg, ui_element_instance_callback_ptr p_callback) {
		service_ptr_t<spectrogram_instance> instance;
		instance = new service_impl_t<spectrogram_instance>(p_parent, p_callback);
		instance->set_configuration(cfg);
		return instance;
	}

	virtual ui_element_config::ptr get_default_configuration() {
		ui_element_config_builder builder;
		return builder.finish(get_guid());
	}

	virtual ui_element_children_enumerator_ptr enumerate_children(ui_element_config::ptr) {
		return 0;
	}

};

static service_factory_single_t<spectrogram> foo_spectrogram;
