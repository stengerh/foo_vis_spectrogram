#include "stdafx.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "spectrum_buffer.h"
#include "CSpectrogramView.h"
#include "config.h"

static COLORREF cfg_spectrum_background = RGB(0, 0, 0);

#define cfg_max_backlog 100

pfc::instance_tracker_server_t<CSpectrogramView> CSpectrogramView::g_instances;

static void g_preprocess_chunk(audio_chunk & p_chunk)
{
	static double log2_div = 1.0/log(2.0);

	audio_sample * data = p_chunk.get_data();

	t_size data_length = p_chunk.get_used_size();
	for (t_size n = 0; n < data_length; n++)
	{
		audio_sample val = pfc::abs_t(data[n]);
		val = (audio_sample)(sqrt(log((val * 0.5) + 1)) * log2_div);
		//val *= 0.5;
		if (val > 1.0) val = 1.0;
		data[n] = val;
	}

#if 0
	audio_sample minval = data[0], maxval = data[0];
	for (t_size n = 0; n < data_length; n++)
	{
		audio_sample val = pfc::abs_t(data[n]);
		if (val < minval) minval = val;
		if (val > maxval) maxval = val;
	}
	//uDebugLog() << "min = " << minval << ", max = " << maxval;
#endif
}

CSpectrogramView::CSpectrogramView()
{
	service_enum_create_t(m_spectrum_cache, 0);
	m_last_displayed_time = m_spectrum_cache->get_start_time();

	m_play_callback_registered = false;
}

CSpectrogramView::~CSpectrogramView()
{
	if (m_play_callback_registered)
		unregister_play_callback();
}

LRESULT CSpectrogramView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_column = 0;
	m_last_time = 0;

	if (DefWindowProc() != 0)
		return -1;

	//m_scaling_quality = spectrum_buffer::scaling_mode_gdi_fast;

    UpdateColorMapper();

	CRect rcClient;
	GetClientRect(&rcClient);
	m_size_changing = false;
	OnSize(SIZE_RESTORED, rcClient.Size());

	register_play_callback();

	m_instance_tracker.initialize(this);

	return 0;
}

void CSpectrogramView::OnDestroy()
{
	m_instance_tracker.uninitialize();

	unregister_play_callback();

	DisableUpdates();
}

void CSpectrogramView::CheckStreamUpdates()
{
	if (m_stream.is_valid())
	{
		double time_increment = 1.0 / pfc::clip_t<unsigned>(cfg_lines_per_second, 1, 200);
		double refresh_increment = 1.0 / pfc::clip_t<unsigned>(cfg_frames_per_second, 1, 100);
		double backlog_increment = cfg_max_backlog / 1000.0;
		double time;
		if (m_stream->get_absolute_time(time))
		{
			//uDebugLog() << m_last_time << " " << time;

			if (m_last_time > time + 2 * time_increment)
			{
				console::info("spectrogram visualisation: forcing last processed position to stream position (backward)");
				m_last_time = time;
			}
			else if (m_last_time < time - pfc::max_t(2 * refresh_increment, backlog_increment))
			{
				m_last_time = pfc::max_t(0.0, time - backlog_increment);
			}

			audio_chunk_impl chunk;
			for (; m_last_time < time; m_last_time += time_increment)
			{
				if (m_spectrum_cache->get_size() > 0 && m_stream->get_spectrum_absolute(chunk, m_last_time, 512))
				{
					g_preprocess_chunk(chunk);

					m_spectrum_cache->add_chunk(chunk);
				}
			}
		}

		if (m_spectrum_cache->get_end_time() > m_last_displayed_time)
		{
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}

void CSpectrogramView::OnPaint(HDC hDC)
{
	CPaintDC dc(m_hWnd);

	CRect rcClient;
	GetClientRect(&rcClient);
	if (m_display_cache.IsNull())
		ResizeDisplayCache(rcClient.Size());

	CRect rcPaint = dc.m_ps.rcPaint;

	CDC dcCache = ::CreateCompatibleDC(dc);
	CBitmapHandle orgBitmap = dcCache.SelectBitmap(m_display_cache);

	if (m_spectrum_cache->get_end_time() > m_last_displayed_time)
	{
		CRect rc;
		rc.left   = rcPaint.left;
		rc.right  = rcPaint.right;
		rc.top    = rcClient.top;
		rc.bottom = rcClient.bottom;

		if (!rc.IsRectEmpty())
		{
			int orgStretchMode = SetStretchBltMode(dcCache, STRETCH_HALFTONE);

			m_spectrum_cache->draw(dc, rc, m_spectrum_cache->get_end_time() - (rcClient.right - rc.left),
				cfg_spectrum_background,
				spectrum_buffer::scaling_mode_gdi_slow);

			SetStretchBltMode(dcCache, orgStretchMode);
		}

		m_last_displayed_time = m_spectrum_cache->get_end_time();
	}

	if (false)
	{
		CMemoryDC dcMem(dc, dc.m_ps.rcPaint);

#if 0
		CSize offset(rcClient.Size() - m_size);

		CRect rcSrcLeft, rcSrcRight;
		rcSrcLeft.SetRect(m_color_offset, 0, m_size.cx, m_size.cy);
		rcSrcRight.SetRect(0, 0, m_color_offset, m_size.cy);

		CRect rcDstLeft, rcDstRight;
		rcDstLeft.SetRect(0, 0, m_size.cx - m_color_offset, m_size.cy);
		rcDstRight.SetRect(m_size.cx - m_color_offset, 0, m_size.cx, m_size.cy);

		rcDstLeft.OffsetRect(offset.cx, 0);
		rcDstRight.OffsetRect(offset.cx, 0);

		dcMem.BitBlt(rcDstLeft.left, rcDstLeft.top, rcDstLeft.Width(), rcDstLeft.Height(),
			dcCache, rcSrcLeft.left, rcSrcLeft.top, SRCCOPY);

		dcMem.BitBlt(rcDstRight.left, rcDstRight.top, rcDstRight.Width(), rcDstRight.Height(),
			dcCache, rcSrcRight.left, rcSrcRight.top, SRCCOPY);
#endif

		if (rcPaint.bottom > m_size.cy)
		{
			dcMem.FillSolidRect(rcPaint.left, m_size.cy, rcPaint.Width(), rcPaint.bottom - m_size.cy, RGB(0, 0, 0));
		}
		if (rcClient.Width() > m_size.cx)
		{
			CRect rcFill;
			rcFill.SetRect(0, 0, rcClient.Width() - m_size.cx, m_size.cy);
			dcMem.FillSolidRect(&rcFill, RGB(0, 0, 0));
		}
	}
	
	dcCache.SelectBitmap(orgBitmap);
}

void CSpectrogramView::OnSize(UINT nType, CSize size)
{
	if (!m_size_changing || true)
	{
		if (size.cx != m_size.cx)
		{
			m_spectrum_cache->set_size(size.cx);

			m_last_displayed_time = m_spectrum_cache->get_start_time();
		}

		m_size = size;

		ResizeDisplayCache(size);
	}
}

void CSpectrogramView::OnEnterSizeMove()
{
	m_size_changing = true;
}

void CSpectrogramView::OnExitSizeMove()
{
	m_size_changing = false;
	CRect rcClient;
	GetClientRect(&rcClient);
	OnSize(SIZE_RESTORED, rcClient.Size());
}

void CSpectrogramView::on_visualisation_timer()
{
	if (cfg_frames_per_second != m_frames_per_second)
	{
		if (m_timer.is_valid())
		{
			m_timer->remove_callback(this);
			m_timer = NULL;
		}
		m_frames_per_second = cfg_frames_per_second;
		static_api_ptr_t<visualisation_timer_manager>()->create_timer_for_rate(m_timer, m_frames_per_second);
		m_timer->add_callback(this);
	}

	CheckStreamUpdates();
}

bool CSpectrogramView::EnableUpdates()
{
	//uDebugLog() << "EnableUpdates";

	try
	{
		static_api_ptr_t<visualisation_manager> vm;
		vm->create_stream(m_stream, 0);

		m_frames_per_second = cfg_frames_per_second;
		static_api_ptr_t<visualisation_timer_manager>()->create_timer_for_rate(m_timer, m_frames_per_second);
		m_timer->add_callback(this);

		return true;
	}
	catch (const exception_service_not_found &)
	{
		popup_message::g_show(
			"Visualisation support not available.",
			"Spectrogram Visualisation",
			popup_message::icon_error);
	}
	catch (const std::exception & exc)
	{
		popup_message::g_show(
			pfc::string8_fastalloc() << "Visualisation stream could not be created:\n" << exc,
			"Spectrogram Visualisation",
			popup_message::icon_error);
	}

	return false;
}

bool CSpectrogramView::DisableUpdates()
{
	//uDebugLog() << "DisableUpdates";

	m_stream = NULL;

	if (m_timer.is_valid())
	{
		m_timer->remove_callback(this);
		m_timer = NULL;
	}

	return true;
}

void CSpectrogramView::on_playback_starting(play_control::t_track_command p_command, bool p_paused)
{
	m_last_time = 0.0;
	EnableUpdates();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE);
}

void CSpectrogramView::on_playback_stop(play_control::t_stop_reason p_reason)
{
	DisableUpdates();
	ClearDisplayCache();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE);
}

void CSpectrogramView::on_playback_pause(bool p_state)
{
	if (p_state)
		DisableUpdates();
	else
		EnableUpdates();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE);
}

void CSpectrogramView::on_playback_seek(double p_time)
{
	m_last_time = 0.0;
}

ATL::CWndClassInfo& CSpectrogramView::GetWndClassInfo()
{
	static ATL::CWndClassInfo wc =
	{
		{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
		  0, 0, NULL, NULL, NULL, (HBRUSH)NULL, NULL, NULL, NULL },
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
	};
	return wc;
}

void CSpectrogramView::ResizeDisplayCache(CSize size)
{
	CWindowDC dcWin(m_hWnd);
	CDC dcMem = ::CreateCompatibleDC(dcWin);

	CBitmapHandle cache;
	cache.CreateCompatibleBitmap(dcWin, size.cx, size.cy);
	m_display_cache = cache;

	CBitmapHandle orgBitmap = dcMem.SelectBitmap(cache);
	dcMem.FillRect(&CRect(0, 0, size.cx, size.cy), CBrush(::CreateSolidBrush(cfg_spectrum_background)));
	dcMem.SelectBitmap(orgBitmap);

	m_last_displayed_time = m_spectrum_cache->get_start_time();
}

void CSpectrogramView::ClearDisplayCache()
{
	CWindowDC dcWin(m_hWnd);
	CDC dcMem = ::CreateCompatibleDC(dcWin);

	CRect rcClient;
	GetClientRect(&rcClient);

	CBitmapHandle orgBitmap = dcMem.SelectBitmap(m_display_cache);
	dcMem.FillRect(&rcClient, CBrush(::CreateSolidBrush(cfg_spectrum_background)));
	dcMem.SelectBitmap(orgBitmap);

	m_last_displayed_time = m_spectrum_cache->get_start_time();
}

void CSpectrogramView::on_spectrum_colors_changed(const t_spectrum_color_info & p_info)
{
	g_create_mapper(m_mapper, p_info);
	//m_mapper = new service_impl_t<colormap_impl_memoize>(m_mapper);

	m_spectrum_cache->set_colormap(m_mapper);

    if (m_hWnd)
    {
	    ClearDisplayCache();

	    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

void CSpectrogramView::UpdateColorMapper()
{
	on_spectrum_colors_changed(cfg_spectrum_color_info);
}

void CSpectrogramView::register_play_callback()
{
	static const unsigned flags =
		flag_on_playback_starting	|
		flag_on_playback_new_track |
		flag_on_playback_stop |
		flag_on_playback_seek |
		flag_on_playback_pause;

	static_api_ptr_t<play_callback_manager> pcm;
	pcm->register_callback(this, flags, true);
	m_play_callback_registered = true;
}

void CSpectrogramView::unregister_play_callback()
{
	static_api_ptr_t<play_callback_manager> pcm;
	pcm->unregister_callback(this);
	m_play_callback_registered = false;
}

void g_fire_spectrum_colors_changed(const t_spectrum_color_info & p_info)
{
	t_size count = CSpectrogramView::g_instances.get_count();
	for (t_size n = 0; n < count; n++)
		CSpectrogramView::g_instances[n]->on_spectrum_colors_changed(p_info);
}
