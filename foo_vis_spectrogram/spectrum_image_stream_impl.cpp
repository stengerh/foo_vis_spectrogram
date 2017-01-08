#include "stdafx.h"
#include "colormap.h"
#include "spectrum_buffer.h"
#include "spectrum_buffer_impl.h"
#include "spectrum_image_stream.h"
#include "spectrum_image_stream_impl.h"

static void g_preprocess_chunk(audio_chunk & p_chunk, bool p_adjust_offset)
{
	static double log2_div = 1.0/log(2.0);

	audio_sample * data = p_chunk.get_data();

	if (p_adjust_offset)
	{
		unsigned channel_count = p_chunk.get_channels();
		t_size sample_count = p_chunk.get_sample_count();

		pfc::array_t<audio_sample> offset;
		audio_sample * current;
#ifdef HAVE_FAST_OFFSET_ADJUSTMENT
		offset.set_data_fromptr(&data[sample_count * channel_count - channel_count], channel_count);
#else
		offset.set_data_fromptr(&data[sample_count * channel_count - channel_count], channel_count);
		current = data;
		for (t_size sample = 0; sample < sample_count; sample++)
		{
			for (unsigned channel = 0; channel < channel_count; channel++)
			{
				audio_sample val = *current++;
				if (val < offset[channel]) offset[channel] = val;
			}
		}
#endif

		current = data;
		for (t_size sample = 0; sample < sample_count; sample++)
		{
			for (unsigned channel = 0; channel < channel_count; channel++)
			{
				*current++ -= offset[channel];
			}
		}
	}

	t_size data_length = p_chunk.get_used_size();
	for (t_size n = 0; n < data_length; n++)
	{
		audio_sample val = data[n];
#if 0
		val = pfc::abs_t(val);
		val = (audio_sample)(sqrt(log((val * 0.5) + 1)) * log2_div);
		//val *= 0.5;
#endif
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




void spectrum_image_stream_manager_impl::create_compatible_stream(service_ptr_t<spectrum_image_stream> & p_out, HWND p_hWnd, unsigned p_fft_size, unsigned p_sample_rate, unsigned p_refresh_rate_hint)
{
	p_out = new service_impl_t<spectrum_image_stream_impl>(p_hWnd, p_fft_size, p_sample_rate, p_refresh_rate_hint);
}




spectrum_image_stream_impl::spectrum_image_stream_impl(HWND p_hWnd, unsigned p_fft_size, unsigned p_sample_rate, unsigned p_refresh_rate_hint)
: m_image_size(0, 0)
{
	m_hWnd = p_hWnd;
	m_fft_size = p_fft_size;
	m_sample_rate = p_sample_rate;
	m_refresh_rate_hint = p_refresh_rate_hint;

	static_api_ptr_t<visualisation_manager>()->create_stream(m_stream, 0);

	m_last_time = 0;

	m_buffer = new service_impl_t<spectrum_buffer_impl>();

	m_current_image_time = m_buffer->get_start_time();
}

void spectrum_image_stream_impl::set_sample_rate(unsigned p_sample_rate)
{
	m_sample_rate = pfc::clip_t<unsigned>(p_sample_rate, 1, 200);
}

unsigned spectrum_image_stream_impl::get_sample_rate()
{
	return m_sample_rate;
}

void spectrum_image_stream_impl::hint_refresh_rate(unsigned p_refresh_rate)
{
	m_refresh_rate_hint = p_refresh_rate;
}

void spectrum_image_stream_impl::set_image_size(SIZE p_size)
{
	if (m_image_size != p_size)
	{
		m_buffer->set_size(p_size.cx);

		CClientDC dc(m_hWnd);

		CBitmapHandle bitmap;
		bitmap.CreateCompatibleBitmap(dc, p_size.cx, p_size.cy);

		if (!m_bitmap.IsNull())
			m_bitmap.DeleteObject();

		m_bitmap.Attach(bitmap.Detach());

		m_image_size = p_size;

		m_current_image_time = m_buffer->get_start_time();
	}
}

SIZE spectrum_image_stream_impl::get_image_size()
{
	return m_image_size;
}

image_stream::t_timestamp spectrum_image_stream_impl::refresh()
{
	PFC_ASSERT(m_stream.is_valid());

	double time_increment = 1.0 / pfc::clip_t<unsigned>(m_sample_rate, 1, 200);
	double refresh_increment = 1.0 / pfc::clip_t<unsigned>(m_refresh_rate_hint, 1, 100);
	double backlog_increment = 0.1;
	double time;
	if (m_stream->get_absolute_time(time))
	{
		if (m_last_time > time + 2 * time_increment)
		{
			console::info("spectrum visualisation: forcing last processed position to stream position (backward)");
			m_last_time = time;
		}
		else if (m_last_time < time - pfc::max_t(2 * refresh_increment, backlog_increment))
		{
			m_last_time = pfc::max_t(0.0, time - backlog_increment);
		}

		audio_chunk_impl chunk;
		for (; m_last_time < time; m_last_time += time_increment)
		{
			if (m_buffer->get_size() > 0 && m_stream->get_spectrum_absolute(chunk, m_last_time, m_fft_size))
			{
				g_preprocess_chunk(chunk, true);

				m_buffer->add_chunk(chunk);
			}
		}
	}

	return m_buffer->get_end_time();
}

void spectrum_image_stream_impl::draw(HDC hdc, int xdst, int ydst, int width, int height, int xsrc, int ysrc, COLORREF clrBackground)
{
	CDC dcMem;
	dcMem.CreateCompatibleDC(hdc);
	CBitmapHandle oldBitmap = dcMem.SelectBitmap(m_bitmap);

	if (m_current_image_time < m_buffer->get_end_time())
	{
	}

	CRect rcTarget(CPoint(xdst, ydst), CSize(width, height));
	CRect rcSource(CPoint(xsrc, ysrc), CSize(width, height));

	if (rcSource.left < 0)
	{
	}
	if (rcSource.right > m_image_size.cx)
	{
	}
	if (rcSource.top < 0)
	{
	}
	if (rcSource.bottom > m_image_size.cy)
	{
	}

	CRect rcBltSource;
	if (rcBltSource.IntersectRect(&rcSource, CRect(CPoint(0, 0), m_image_size)))
	{
		CRect rcBltTarget = rcTarget;
		rcBltTarget.OffsetRect(rcBltSource.TopLeft() - rcSource.TopLeft());
		::BitBlt(hdc, rcBltTarget.left, rcBltTarget.top, rcBltSource.Width(), rcBltSource.Height(),
			dcMem, rcBltSource.left, rcBltTarget.top, SRCCOPY);
	}

	dcMem.SelectBitmap(oldBitmap);

	throw pfc::exception_not_implemented();
}

void spectrum_image_stream_impl::suspend(bool p_state)
{
	throw pfc::exception_not_implemented();
}

bool spectrum_image_stream_impl::is_suspended()
{
	throw pfc::exception_not_implemented();
}

void spectrum_image_stream_impl::set_fft_size(unsigned p_fft_size)
{
	if (p_fft_size != 0)
		throw pfc::exception_bug_check("FFT size must be greater than zero", 0);

	if (((p_fft_size - 1) & p_fft_size) != 0)
		throw pfc::exception_bug_check("FFT size must be a power of two", 0);

	m_fft_size = p_fft_size;
}

unsigned spectrum_image_stream_impl::get_fft_size()
{
	return m_fft_size;
}

void spectrum_image_stream_impl::set_colormap(const service_ptr_t<colormap> & p_colormap)
{
	m_buffer->set_colormap(p_colormap);

	clear_image();
}

bool spectrum_image_stream_impl::hittest(int x, int y, unsigned & p_frequency, unsigned p_channel_config)
{
	throw pfc::exception_not_implemented();
}

void spectrum_image_stream_impl::clear_image()
{
	CDC dcMem;
	if (m_hWnd != NULL)
	{
		CClientDC dcClient(m_hWnd);
		dcMem.CreateCompatibleDC(dcClient);
	}
	else
	{
		dcMem.CreateCompatibleDC();
	}

	CBitmapHandle oldBitmap = dcMem.SelectBitmap(m_bitmap);

	dcMem.FillSolidRect(CRect(CPoint(0, 0), m_image_size), RGB(0, 0, 0));

	dcMem.SelectBitmap(oldBitmap);

	m_current_image_time = m_buffer->get_start_time();
}
