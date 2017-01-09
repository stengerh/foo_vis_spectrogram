#include "stdafx.h"
#include "colormap.h"

// {0E485447-6E24-4100-8E52-B4404C0930F1}
const GUID colormap::class_guid = 
{ 0xe485447, 0x6e24, 0x4100, { 0x8e, 0x52, 0xb4, 0x40, 0x4c, 0x9, 0x30, 0xf1 } };


COLORREF colormap_impl_builtin::map(audio_sample p_sample)
{
    int val = pfc::clip_t((int)(p_sample * 255), 0, 255);
    return RGB(val, val, val);
}

colormap_impl_memoize::colormap_impl_memoize(service_ptr_t<colormap> p_mapper, t_size p_table_size)
{
    m_table.set_size(p_table_size);
    pfc::array_t<audio_sample> temp;
    temp.set_size(p_table_size);
    for (unsigned n = 0; n < p_table_size; n++)
        temp[n] = n / (audio_sample)p_table_size;
    p_mapper->map(temp.get_ptr(), m_table.get_ptr(), p_table_size);
}

COLORREF colormap_impl_memoize::map(audio_sample p_sample)
{
    int index = pfc::clip_t<int>((int)(p_sample * m_table.get_size()), 0, m_table.get_size() - 1);
    return m_table[index];
}

colormap_impl_blend_rgb::colormap_impl_blend_rgb(COLORREF p_colorLow, COLORREF p_colorHigh)
{
    m_colorLow = p_colorLow;
    m_colorHigh = p_colorHigh;
}

struct rgb_color_t
{
    double r, g, b;
};

struct hsv_color_t
{
    double h, s, v;
};

// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = 0 (undefined)
static void convert_rgb_to_hsv(const rgb_color_t & in, hsv_color_t & out)
{

    double min, max, delta;

    min = in.r;
    if (min > in.g) min = in.g;
    if (min > in.b) min = in.b;
    max = in.r;
    if (max < in.g) max = in.g;
    if (max < in.b) max = in.b;
    out.v = max;				// v

    delta = max - min;

    if( max != 0 )
        out.s = delta / max;		// s
    else {
        // r = g = b = 0		// s = 0, v is undefined
        out.s = 0;
        out.h = 0;
        return;
    }

    if (delta > 0)
    {
        if( in.r == max )
            out.h = ( in.g - in.b ) / delta;		// between yellow & magenta
        else if( in.g == max )
            out.h = 2 + ( in.b - in.r ) / delta;	// between cyan & yellow
        else
            out.h = 4 + ( in.r - in.g ) / delta;	// between magenta & cyan

        out.h *= 60;				// degrees
        if( out.h < 0 )
            out.h += 360;
    }
    else
        out.h = 0;
}

static void convert_hsv_to_rgb(const hsv_color_t & in, rgb_color_t & out)
{
    int i;
    double f, p, q, t;
    double h;

    if( in.s == 0 ) {
        // achromatic (grey)
        out.r = out.g = out.b = in.v;
        return;
    }

    h = in.h / 60;			// sector 0 to 5
    i = (int)floor( h );
    f = h - i;			// factorial part of h
    p = in.v * ( 1 - in.s );
    q = in.v * ( 1 - in.s * f );
    t = in.v * ( 1 - in.s * ( 1 - f ) );

    switch( i ) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;
    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    default:		// case 5:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
}

COLORREF colormap_impl_blend_rgb::map(audio_sample p_sample)
{
    audio_sample f, finv;
    f = pfc::clip_t<audio_sample>(p_sample, 0, 1);
    finv = 1 - f;
    return RGB(
        GetRValue(m_colorHigh) * f + GetRValue(m_colorLow) * finv,
        GetGValue(m_colorHigh) * f + GetGValue(m_colorLow) * finv,
        GetBValue(m_colorHigh) * f + GetBValue(m_colorLow) * finv);
}

colormap_impl_blend_hsv::colormap_impl_blend_hsv(COLORREF p_colorLow, COLORREF p_colorHigh, bool p_clockwise)
{
    rgb_color_t low_rgb, high_rgb;

    low_rgb.r = GetRValue(p_colorLow) / 255.0;
    low_rgb.g = GetGValue(p_colorLow) / 255.0;
    low_rgb.b = GetBValue(p_colorLow) / 255.0;

    high_rgb.r = GetRValue(p_colorHigh) / 255.0;
    high_rgb.g = GetGValue(p_colorHigh) / 255.0;
    high_rgb.b = GetBValue(p_colorHigh) / 255.0;

    hsv_color_t low_hsv, high_hsv;

    convert_rgb_to_hsv(low_rgb, low_hsv);
    convert_rgb_to_hsv(high_rgb, high_hsv);

    m_hueLow = low_hsv.h;
    m_satLow = low_hsv.s;
    m_valLow = low_hsv.v;

    m_hueHigh = high_hsv.h;
    m_satHigh = high_hsv.s;
    m_valHigh = high_hsv.v;

    m_clockwise = p_clockwise;

    // blending involving achromatic colors
    if (m_satLow == 0)
    {
        m_hueLow = m_hueHigh;
    }
    else if (m_satHigh == 0)
    {
        m_hueHigh = m_hueLow;
    }

    // preprocessing for blending in selected direction
    if (m_clockwise)
    {
        if (m_hueHigh > m_hueLow)
        {
            m_hueHigh -= 360;
        }
    }
    else
    {
        if (m_hueHigh < m_hueLow)
        {
            m_hueLow -= 360;
        }
    }
}

COLORREF colormap_impl_blend_hsv::map(audio_sample p_sample)
{
    audio_sample f, finv;
    f = pfc::clip_t<audio_sample>(p_sample, 0, 1);
    finv = 1 - f;

    hsv_color_t out_hsv;
    out_hsv.s = m_satHigh * f + m_satLow * finv;
    out_hsv.v = m_valHigh * f + m_valLow * finv;

    out_hsv.h = m_hueHigh * f + m_hueLow * finv;
    if (out_hsv.h < 0) out_hsv.h += 360;

    rgb_color_t out_rgb;
    convert_hsv_to_rgb(out_hsv, out_rgb);

    return RGB(out_rgb.r * 255, out_rgb.g * 255, out_rgb.b * 255);
}
