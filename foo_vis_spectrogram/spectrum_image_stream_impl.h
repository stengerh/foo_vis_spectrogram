#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_IMAGE_STREAM_IMPL_H__
#define __FOO_VIS_SPECTRUM__SPECTRUM_IMAGE_STREAM_IMPL_H__

#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_IMAGE_STREAM_H__
#error spectrum_image_stream_impl.h requires spectrum_image_stream.h to be included first.
#endif

#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_BUFFER_H__
#error spectrum_image_stream_impl.h requires spectrum_buffer.h to be included first.
#endif

class spectrum_image_stream_impl : public spectrum_image_stream
{
private:
    unsigned m_fft_size;
    unsigned m_sample_rate;
    unsigned m_refresh_rate_hint;

    CWindow m_hWnd;
    CSize m_image_size;

    service_ptr_t<visualisation_stream> m_stream;

    service_ptr_t<spectrum_buffer> m_buffer;
    CBitmap m_bitmap;

    double m_last_time;

    t_timestamp m_current_image_time;

    void clear_image();

public:
    spectrum_image_stream_impl(HWND p_hWnd, unsigned p_fft_size, unsigned p_sample_rate, unsigned p_refresh_rate_hint);

    virtual void set_sample_rate(unsigned p_sample_rate);
    virtual unsigned get_sample_rate();

    virtual void hint_refresh_rate(unsigned p_refresh_rate);

    virtual void set_image_size(SIZE p_size);
    virtual SIZE get_image_size();

    virtual t_timestamp refresh();
    virtual void draw(HDC hdc, int xdst, int ydst, int width, int height, int xsrc, int ysrc, COLORREF clrBackground);

    virtual void suspend(bool p_state);
    virtual bool is_suspended();

    virtual void set_fft_size(unsigned p_fft_size);
    virtual unsigned get_fft_size();

    virtual void set_colormap(const service_ptr_t<colormap> & p_colormap);

    virtual bool hittest(int x, int y, unsigned & p_frequency, unsigned p_channel_config);
};

class spectrum_image_stream_manager_impl : public spectrum_image_stream_manager
{
public:
    virtual void create_compatible_stream(service_ptr_t<spectrum_image_stream> & p_out, HWND p_hWnd, unsigned p_fft_size, unsigned p_sample_rate, unsigned p_refresh_rate_hint = 0);
};

#endif
