#include "stdafx.h"
#include "colormap.h"
#include "spectrum_buffer.h"
#include "spectrum_buffer_impl.h"

spectrum_buffer_metrics::spectrum_buffer_metrics(t_size p_size, t_size p_sample_count, t_spectrum_timestamp p_start_time)
: m_size(p_size), m_sample_count(p_sample_count)
{
	m_start_time = m_end_time = p_start_time;
	m_offset = 0;
}

spectrum_buffer_metrics::spectrum_buffer_metrics(const spectrum_buffer_metrics & p_source, t_size p_new_size)
: m_size(p_new_size), m_sample_count(p_source.get_sample_count())
{
	m_end_time = p_source.get_end_time();
	m_start_time = m_end_time - pfc::min_t(m_size, p_source.get_length());
	m_offset = 0;
}

void spectrum_buffer_metrics::advance_time()
{
	++m_end_time;
	if (m_end_time - m_start_time > m_size)
	{
		++m_start_time;
		++m_offset;
		if (m_offset > m_size)
			m_offset = 0;
	}
}

t_size spectrum_buffer_metrics::get_index_for_time(t_spectrum_timestamp p_time) const
{
	if (p_time < m_start_time || p_time > m_end_time)
		throw pfc::exception_overflow();

	// degenerate border case
	if (m_size == 0)
		return 0;

	return ((t_size)(p_time - m_start_time) + m_offset) % m_size;
}


spectrum_buffer_impl::spectrum_buffer_impl()
{
	m_metrics.new_t(0, 128);
	m_aux_info.new_t(m_metrics);
	update_bitmap_info();
	set_colormap(service_ptr_t<colormap>());
}

void spectrum_buffer_impl::set_size(t_size p_size)
{
	if (p_size != get_size())
	{
		// SetDIBits requires scan lines to be aligned on a DWORD boundary
		p_size = ((p_size * sizeof(spectrum_sample) + 3) & ~3) / sizeof(spectrum_sample);

		// create new metrics
		pfc::rcptr_t<spectrum_buffer_metrics> old_metrics = m_metrics;
		m_metrics.new_t(*old_metrics, p_size);

		// create new channel buffers
		for (t_size n = 0; n < m_channel.get_size(); n++)
		{
			pfc::rcptr_t<spectrum_channel_buffer> temp = m_channel[n];
			if (temp.is_valid())
			{
				m_channel[n].new_t(m_metrics, *temp);
			}
		}

		// create new auxialliary buffer
		m_aux_info.new_t(m_metrics, *m_aux_info);

		update_bitmap_info();
	}
}

void spectrum_buffer_impl::set_sample_count(t_size p_sample_count)
{
	if (p_sample_count != get_sample_count())
	{
		// create new metrics
		m_metrics.new_t(get_size(), p_sample_count, get_end_time());

		// create new channel buffers
		for (t_size n = 0; n < m_channel.get_size(); n++)
		{
			m_channel[n].new_t(m_metrics);
		}

		// create new auxialliary buffer
		m_aux_info.new_t(m_metrics);

		update_bitmap_info();
	}
}

void spectrum_buffer_impl::update_bitmap_info()
{
	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = (LONG)get_size();
	m_bmi.bmiHeader.biHeight = (LONG)get_sample_count();
	m_bmi.bmiHeader.biPlanes = 1;
	m_bmi.bmiHeader.biBitCount = 8;
	m_bmi.bmiHeader.biCompression = BI_RGB;
	m_bmi.bmiHeader.biSizeImage = 0;
	m_bmi.bmiHeader.biClrUsed = 256;
	m_bmi.bmiHeader.biClrImportant = 0;
}

void spectrum_buffer_impl::set_colormap(const service_ptr_t<colormap> & p_colormap)
{
	if (p_colormap.is_empty())
	{
		for (unsigned n = 0; n < 256; n++)
			m_bmi.bmiColors[n] = CRGBQuad(n, n, n);
	}
	else
	{
		audio_sample samples[256];
		COLORREF colors[256];
		for (unsigned n = 0; n < 256; n++)
			samples[n] = n / (audio_sample)255;
		p_colormap->map(samples, colors, 256);
		for (unsigned n = 0; n < 256; n++)
			m_bmi.bmiColors[n] = CRGBQuad(colors[n]);
	}
}

void spectrum_buffer_impl::draw(HDC p_hdc, CRect p_rcTarget, t_spectrum_timestamp p_base_time, COLORREF clrBackground, t_scaling_mode p_quality) const
{
	CDCHandle dc(p_hdc);
	CRect rcDst(p_rcTarget);

	t_spectrum_timestamp from_time_requested = p_base_time;
	t_spectrum_timestamp to_time_requested   = p_base_time + rcDst.Width();

	t_spectrum_timestamp from_time = pfc::clip_t(from_time_requested, get_start_time(), get_end_time());
	t_spectrum_timestamp to_time   = pfc::clip_t(to_time_requested,   get_start_time(), get_end_time());

	if (from_time > from_time_requested)
	{
		CRect rc(rcDst.left, rcDst.top, rcDst.left + (int)(from_time - from_time_requested), rcDst.bottom);
		dc.FillSolidRect(&rc, clrBackground);
	}

	if (to_time < to_time_requested)
	{
		CRect rc(rcDst.left + (int)(to_time_requested - to_time), rcDst.top, rcDst.right, rcDst.bottom);
		dc.FillSolidRect(&rc, clrBackground);
	}

	if (from_time < to_time)
	{
		t_size from_index = get_index_for_time(from_time);
		t_size to_index   = get_index_for_time(to_time);

		CRect rcSrc;
		rcSrc.top    = 0;
		rcSrc.bottom = (int)get_sample_count();

		CRect rc;
		rc.top    = rcDst.top;
		rc.bottom = rcDst.bottom;


		if (from_index < to_index)
		{
			// single blit
			rcSrc.left  = (int)from_index;
			rcSrc.right = (int)to_index;
			rc.left  = rcDst.left + (int)(from_time - from_time_requested);
			rc.right = rc.left + rcSrc.Width();
			draw_channels(dc, rc, rcSrc, p_quality);
		}
		else
		{
			// two blits
			rcSrc.left  = (int)from_index;
			rcSrc.right = (int)get_size();
			rc.left  = rcDst.left + (int)(from_time - from_time_requested);
			rc.right = rc.left + rcSrc.Width();
			draw_channels(dc, rc, rcSrc, p_quality);

			rcSrc.left  = 0;
			rcSrc.right = (int)to_index;
			rc.right = rcDst.right - (int)(to_time_requested - to_time);
			rc.left  = rc.right - rcSrc.Width();
			draw_channels(dc, rc, rcSrc, p_quality);
		}
	}
}

void spectrum_buffer_impl::draw_channels(CDCHandle dc, CRect rcDst, CRect rcSrc, t_scaling_mode p_quality) const
{
	int n, start = 0;
	unsigned channel_count = m_aux_info->get_data(start).m_channel_count;
	for (n = start + 1; n < rcSrc.Width(); n++)
	{
		unsigned current_channel_count = m_aux_info->get_data(n + rcSrc.left).m_channel_count;
		if (current_channel_count != channel_count)
		{
			draw_channels_uniform(dc,
				CRect(start + rcDst.left, rcDst.top, n + rcDst.left, rcDst.bottom),
				CRect(start + rcSrc.left, rcSrc.top, n + rcSrc.left, rcSrc.bottom),
				p_quality);
			start = n;
			channel_count = current_channel_count;
		}
	}
	if (n > start)
		draw_channels_uniform(dc,
			CRect(start + rcDst.left, rcDst.top, n + rcDst.left, rcDst.bottom),
			CRect(start + rcSrc.left, rcSrc.top, n + rcSrc.left, rcSrc.bottom),
			p_quality);
}

void spectrum_buffer_impl::draw_channels_uniform(CDCHandle dc, CRect rcDst, CRect rcSrc, t_scaling_mode p_quality) const
{
	int y0 = rcDst.top;
	int height = rcDst.Height();
	unsigned count = m_aux_info->get_data(rcSrc.left).m_channel_count;
	for (unsigned n = 0; n < count; n++)
	{
		int y1 = rcDst.top + MulDiv(height, n + 1, (int)count);
		draw_channel(dc, CRect(rcDst.left, y0, rcDst.right, y1), rcSrc, n, p_quality);
		y0 = y1;
	}
}

void spectrum_buffer_impl::draw_channel(CDCHandle dc, CRect rcDst, CRect rcSrc, t_size p_channel, t_scaling_mode p_quality) const
{
	if (p_quality >= scaling_mode_software_linear)
	{
		int width  = ((rcDst.Width() * sizeof(spectrum_sample) + 3) & ~3) / sizeof(spectrum_sample);
		int dst_height = rcDst.Height();

		int src_height = rcSrc.Height();

		BITMAPINFOSPECTRUM bmi = m_bmi;
		bmi.bmiHeader.biWidth  = width;
		bmi.bmiHeader.biHeight = dst_height;

		pfc::array_t<spectrum_sample> data;
		data.set_size(width * dst_height);

		for (int x = 0; x < width; x++)
		{
			if (p_quality == scaling_mode_software_linear)
			{
				interpolate_linear_t(
					accessor_repeat_borders_t<spectrum_sample, subarray_accessor_t<spectrum_sample> >(
						m_channel[p_channel]->get_items(x + rcSrc.left), get_sample_count()),
					src_height,
					subarray_accessor_t<spectrum_sample>(&data[x], width), dst_height);
			}
			else
			{
				interpolate_cubic_t(
					accessor_repeat_borders_t<spectrum_sample, subarray_accessor_t<spectrum_sample> >(
						m_channel[p_channel]->get_items(x + rcSrc.left), get_sample_count()),
					src_height,
					subarray_accessor_t<spectrum_sample>(&data[x], width), dst_height);
			}
		}

		dc.SetDIBitsToDevice(
			rcDst.left, rcDst.top, rcDst.Width(), rcDst.Height(),
			0, 0, 0, dst_height,
			data.get_ptr(), &bmi, DIB_RGB_COLORS);
	}
	else
	{
		int oldStretchBltMode = dc.SetStretchBltMode(
			p_quality == scaling_mode_gdi_slow ? STRETCH_HALFTONE : STRETCH_DELETESCANS);

		dc.StretchDIBits(
			rcDst.left, rcDst.top, rcDst.Width(), rcDst.Height(),
			rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(),
			m_channel[p_channel]->get_ptr(), &m_bmi, DIB_RGB_COLORS, SRCCOPY);

		dc.SetStretchBltMode(oldStretchBltMode);
	}
}

void spectrum_buffer_impl::set_channels(t_size p_index, unsigned p_channel_count, unsigned p_channel_config)
{
	unsigned old_count = 0;
	old_count = m_aux_info->get_data(p_index).m_channel_count;

	if (old_count < p_channel_count)
	{
		for (unsigned n = old_count; n < p_channel_count; n++)
			channel_addref(n);
	}
	else if (old_count > p_channel_count)
	{
		for (unsigned n = p_channel_count; n < old_count; n++)
			channel_release(n);
	}

	m_aux_info->get_data(p_index) = t_aux_info(p_channel_count, p_channel_config);
}

long spectrum_buffer_impl::channel_addref(unsigned p_channel)
{
	unsigned min_channel_counnt = p_channel + 1;
	if (min_channel_counnt > m_channel_refcount.get_size())
	{
		m_channel.set_size(min_channel_counnt);

		t_size old_size = m_channel_refcount.get_size();
		m_channel_refcount.set_size(min_channel_counnt);
		for (t_size n = old_size; n < min_channel_counnt; n++)
			m_channel_refcount[n] = 0;
	}

	long rv = ++m_channel_refcount[p_channel];
	if (rv == 1)
		m_channel[p_channel].new_t(m_metrics);
	return rv;
}

long spectrum_buffer_impl::channel_release(unsigned p_channel)
{
	PFC_ASSERT(p_channel < m_channel_refcount.get_size());

	long rv = --m_channel_refcount[p_channel];
	if (rv <= 0)
		m_channel[p_channel].release();
	return rv;
}

t_spectrum_timestamp spectrum_buffer_impl::add_chunk(const audio_chunk & p_chunk)
{
	m_metrics->advance_time();
	t_spectrum_timestamp now = get_end_time();

	if (now > get_start_time())
	{
		set_sample_count(p_chunk.get_sample_count());

		t_size index = get_index_for_time(now - 1);

		unsigned channel_count = p_chunk.get_channels();
		set_channels(index, channel_count, p_chunk.get_channel_config());

		const audio_sample * source = p_chunk.get_data();
		t_size sample_count = p_chunk.get_sample_count();
		for (t_size sample = 0; sample < sample_count; sample++)
		{
			for (unsigned n = 0; n < channel_count; n++)
			{
				m_channel[n]->set_item(index, sample, (t_uint8)pfc::clip_t<audio_sample>(source[0] * 255, 0, 255));
				++source;
			}
		}
	}

	return now;
}

static service_factory_t<spectrum_buffer_impl> foo_spectrum_buffer;
