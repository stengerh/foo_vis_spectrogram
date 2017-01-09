#ifndef __FOO_VIS_SPECTRUM__COLORMAP_H__
#define __FOO_VIS_SPECTRUM__COLORMAP_H__

struct CRGBQuad : public RGBQUAD
{
    CRGBQuad() {rgbRed = 0; rgbGreen = 0; rgbBlue = 0; rgbReserved = 0;}
    CRGBQuad(BYTE p_rgbRed, BYTE p_rgbGreen, BYTE p_rgbBlue) {rgbRed = p_rgbRed; rgbGreen = p_rgbGreen; rgbBlue = p_rgbBlue; rgbReserved = 0;}
    CRGBQuad(COLORREF clr) {rgbRed = GetRValue(clr); rgbGreen = GetGValue(clr); rgbBlue = GetBValue(clr); rgbReserved = 0;}
};

struct t_spectrum_color_info
{
    t_uint32 m_blend_mode;
    COLORREF m_low_energy, m_high_energy;

    t_spectrum_color_info();
    t_spectrum_color_info(t_uint32 p_blend_mode, COLORREF p_low_energy, COLORREF p_high_energy);

    void to_text(pfc::string_base & p_text_out, const char * p_comment);
    void from_text(const char * p_text, pfc::string_base & p_comment_out);

    inline void to_text(pfc::string_base & p_text_out) {to_text(p_text_out, NULL);}
    inline void from_text(const char * p_text) {from_text(p_text, pfc::string8());}
};

class NOVTABLE colormap : public service_base
{
public:
    virtual void map(const audio_sample * p_samples, COLORREF * p_colors, t_size p_count) = 0;

    FB2K_MAKE_SERVICE_INTERFACE(colormap, service_base);
};

template <class T>
class colormap_impl_t : public colormap
{
public:
    virtual void map(const audio_sample * p_samples, COLORREF * p_colors, t_size p_count)
    {
        T * _this = (T *)this;
        for (t_size n = 0; n < p_count; n++)
            p_colors[n] = _this->map(p_samples[n]);
    }

    COLORREF map(audio_sample p_sample) {return RGB(0, 0, 0);}
};

class colormap_impl_builtin : public colormap_impl_t<colormap_impl_builtin>
{
public:
    COLORREF map(audio_sample p_sample);
};

class colormap_impl_memoize : public colormap_impl_t<colormap_impl_memoize>
{
public:
    colormap_impl_memoize(service_ptr_t<colormap> p_mapper, t_size p_table_size);

    COLORREF map(audio_sample p_sample);

private:
    pfc::array_t<COLORREF> m_table;
};

class colormap_impl_blend_rgb : public colormap_impl_t<colormap_impl_blend_rgb>
{
public:
    colormap_impl_blend_rgb(COLORREF p_colorLow, COLORREF p_colorHigh);

    COLORREF map(audio_sample p_sample);

private:
    COLORREF m_colorLow, m_colorHigh;
};

class colormap_impl_blend_hsv : public colormap_impl_t<colormap_impl_blend_hsv>
{
public:
    colormap_impl_blend_hsv(COLORREF p_colorLow, COLORREF p_colorHigh, bool p_clockwise);

    COLORREF map(audio_sample p_sample);

private:
    double m_hueLow, m_satLow, m_valLow,
        m_hueHigh, m_satHigh, m_valHigh;
    bool m_clockwise;
};

#endif
