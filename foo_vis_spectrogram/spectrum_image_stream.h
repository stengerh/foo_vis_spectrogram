#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_IMAGE_STREAM_H__
#define __FOO_VIS_SPECTRUM__SPECTRUM_IMAGE_STREAM_H__

#ifndef __FOO_VIS_SPECTRUM__COLORMAP_H__
#error spectrum_image_stream.h requires colormap.h to be included first.
#endif

class NOVTABLE image_stream : public service_base
{
public:
	typedef t_int64 t_timestamp;

	virtual void set_sample_rate(unsigned p_sample_rate) = 0;
	virtual unsigned get_sample_rate() = 0;

	virtual void hint_refresh_rate(unsigned p_refresh_rate) = 0;

	virtual void set_image_size(SIZE p_size) = 0;
	virtual SIZE get_image_size() = 0;

	virtual t_timestamp refresh() = 0;
	virtual void draw(HDC hdc, int xdst, int ydst, int width, int height, int xsrc, int ysrc, COLORREF clrBackground) = 0;

	virtual void suspend(bool p_state) = 0;
	virtual bool is_suspended() = 0;

	FB2K_MAKE_SERVICE_INTERFACE(image_stream, service_base);
};

class NOVTABLE spectrum_image_stream : public image_stream
{
public:
	virtual void set_fft_size(unsigned p_fft_size) = 0;
	virtual unsigned get_fft_size() = 0;

	virtual void set_colormap(const service_ptr_t<colormap> & p_colormap) = 0;

	virtual bool hittest(int x, int y, unsigned & p_frequency, unsigned p_channel_config) = 0;

	FB2K_MAKE_SERVICE_INTERFACE(spectrum_image_stream, image_stream);
};

class NOVTABLE spectrum_image_stream_manager : public service_base
{
public:
	virtual void create_compatible_stream(service_ptr_t<spectrum_image_stream> & p_out, HWND p_hWnd, unsigned p_fft_size, unsigned p_sample_rate, unsigned p_refresh_rate_hint = 0) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(spectrum_image_stream_manager);
};

#endif
