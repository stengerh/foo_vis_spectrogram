#ifndef __FOO_VIS_SPECTRUM__CSPECTRUMVIEW_H__
#define __FOO_VIS_SPECTRUM__CSPECTRUMVIEW_H__

#include "spectrum_buffer.h"
#include "visualisation_timer.h"

typedef dsp_chunk_list visualisation_chunk_list;
typedef dsp_chunk_list_impl visualisation_chunk_list_impl;

class unscaled_spectrum_buffer
{
public:
};

class CSpectrumView :
	public CWindowImpl<CSpectrumView>,
	private play_callback,
	private visualisation_timer_callback
{
public:
	CSpectrumView();
	~CSpectrumView();

	BEGIN_MSG_MAP(CSpectrumView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_SIZE(OnSize)
		MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
	END_MSG_MAP()

	static ATL::CWndClassInfo& GetWndClassInfo();

	static pfc::instance_tracker_server_t<CSpectrumView> g_instances;

	void on_spectrum_colors_changed(const t_spectrum_color_info & p_info);

	//spectrum_buffer::t_scaling_mode get_scaling_quality() {return m_scaling_quality;}
	//void set_scaling_quality(spectrum_buffer::t_scaling_mode p_quality);

	bool get_reduce_banding() {return m_reduce_banding;}
	void set_reduce_banding(bool p_state) {m_reduce_banding = p_state;}

	bool get_smooth_scaling() {return m_smooth_scaling;}
	void set_smooth_scaling(bool p_state);

private:
	pfc::instance_tracker_client_t<CSpectrumView, g_instances> m_instance_tracker;

	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnPaint(HDC hDC);
	void OnSize(UINT nType, CSize size);
	void OnEnterSizeMove();
	void OnExitSizeMove();

	bool EnableUpdates();
	bool DisableUpdates();

	void CheckStreamUpdates();

	void ResizeDisplayCache(CSize size);
	void ClearDisplayCache();

	void UpdateColorMapper();

	void register_play_callback();
	void unregister_play_callback();

	// play_callback methods

	virtual void FB2KAPI on_playback_starting(play_control::t_track_command p_command, bool p_paused);
	virtual void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track) {}
	virtual void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason);
	virtual void FB2KAPI on_playback_seek(double p_time);
	virtual void FB2KAPI on_playback_pause(bool p_state);
	virtual void FB2KAPI on_playback_edited(metadb_handle_ptr p_track) {}
	virtual void FB2KAPI on_playback_dynamic_info(const file_info & p_info) {}
	virtual void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info) {}
	virtual void FB2KAPI on_playback_time(double p_time) {}
	virtual void FB2KAPI on_volume_change(float p_new_val) {}

	// visualisation_timer_callback methods

	virtual void on_visualisation_timer();

	// fields

	bool m_play_callback_registered;

	unsigned m_frames_per_second;

	//spectrum_buffer::t_scaling_mode m_scaling_quality;
	bool m_reduce_banding, m_smooth_scaling;

	service_ptr_t<visualisation_stream> m_stream;
	double m_last_time;
	int m_column;

	service_ptr_t<visualisation_timer> m_timer;

	service_ptr_t<spectrum_buffer> m_spectrum_cache;

	CBitmap m_display_cache;
	t_spectrum_timestamp m_last_displayed_time;

	bool m_size_changing;
	CSize m_size;

	service_ptr_t<colormap> m_mapper;
};

#endif
