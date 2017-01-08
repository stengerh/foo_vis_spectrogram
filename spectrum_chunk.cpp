#include "stdafx.h"
#include "colormap.h"
#include "spectrum_chunk.h"

void spectrum_chunk::set_channels(unsigned val)
{
	set_channels(val, audio_chunk::g_guess_channel_config(val));
}

void spectrum_chunk::set_data(const COLORREF * p_source, t_size p_sample_count, unsigned p_channel_count, unsigned p_sample_rate, unsigned p_channel_config)
{
	throw pfc::exception_not_implemented();
}

void spectrum_chunk::set_data(const audio_chunk & p_source, service_ptr_t<colormap> p_mapper)
{
	set_data_size(p_source.get_data_length());
	set_sample_count(p_source.get_sample_count());
	set_sample_rate(p_source.get_sample_rate());
	set_channels(p_source.get_channels(), p_source.get_channel_config());

	p_mapper->map(p_source.get_data(), get_data(), get_data_length());
}

void spectrum_chunk::draw(HDC hdc, int x, int y, int height, const pfc::list_base_const_t<unsigned> & p_channels)
{
	struct BITMAPINFO3
	{
		BITMAPINFOHEADER bmiHeader;
		COLORREF bmiColors[3];
	};

	BITMAPINFO3 bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = get_channels();
	bmi.bmiHeader.biHeight = (LONG)get_sample_count();
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB | BI_BITFIELDS;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	bmi.bmiColors[0] = RGB(255, 0, 0);
	bmi.bmiColors[1] = RGB(0, 255, 0);
	bmi.bmiColors[2] = RGB(0, 0, 255);

	int y0 = y;
	for (unsigned n = 0; n < p_channels.get_count(); n++)
	{
		int y1 = y + MulDiv(height, n + 1, (int)p_channels.get_count());
		StretchDIBits(hdc,
			x, y0, 1, y1 - y0,
			p_channels[n], 0, 1, bmi.bmiHeader.biHeight,
			get_data(), (BITMAPINFO *)&bmi,
			DIB_RGB_COLORS,
			SRCCOPY);
		y0 = y1;
	}
}

template <typename T>
class channel_auto_list_t : public pfc::list_base_const_t<T>
{
private:
	t_size m_count;

public:
	channel_auto_list_t(t_size p_count) : m_count(p_count) {}
	virtual t_size get_count() const {return m_count;}
	virtual void get_item_ex(T & p_out, t_size n) const {PFC_ASSERT(n < m_count); p_out = (T)n;}
};

typedef channel_auto_list_t<unsigned> channel_auto_list;

void spectrum_chunk::draw_auto(HDC hdc, int x, int y, int height)
{
	return draw(hdc, x, y, height, channel_auto_list(get_channels()));
}
