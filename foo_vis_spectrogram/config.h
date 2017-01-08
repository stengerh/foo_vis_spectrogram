#ifndef __FOO_VIS_SPECTRUM__CONFIG_H__
#define __FOO_VIS_SPECTRUM__CONFIG_H__

extern cfg_bool cfg_popup_enabled;
extern cfg_window_placement cfg_popup_window_placement;

enum t_blend_mode
{
	blend_mode_linear,
	blend_mode_clockwise,
	blend_mode_counterclockwise,
};

extern cfg_struct_t<t_spectrum_color_info> cfg_spectrum_color_info;

extern void g_fire_spectrum_colors_changed(const t_spectrum_color_info & p_info);

extern cfg_uint cfg_frames_per_second, cfg_lines_per_second;

extern void g_create_mapper(service_ptr_t<colormap> & p_out, t_spectrum_color_info p_info);
inline void g_create_mapper(service_ptr_t<colormap> & p_out, COLORREF p_colorLow, COLORREF p_colorHigh, unsigned p_blend_mode) {g_create_mapper(p_out, t_spectrum_color_info(p_blend_mode, p_colorLow, p_colorHigh));}

extern const GUID guid_prefs_vis_spectrum;

#endif
