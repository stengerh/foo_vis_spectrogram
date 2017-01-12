#include "stdafx.h"
#include "colormap.h"
#include "config.h"

static const GUID guid_cfg_popup_enabled = { 0xea683940, 0x198d, 0x4ad2, { 0xaa, 0x80, 0x7c, 0xd7, 0x16, 0xcb, 0x98, 0x47 } };
static const GUID guid_cfg_popup_window_placement = { 0x52e15603, 0x264c, 0x473c, { 0xb9, 0xfe, 0x53, 0x72, 0xb9, 0xea, 0xaa, 0x93 } };

cfg_bool cfg_popup_enabled(guid_cfg_popup_enabled, false);
cfg_window_placement cfg_popup_window_placement(guid_cfg_popup_window_placement);

static const GUID guid_cfg_spectrum_color_info = { 0x1d342c4c, 0x450c, 0x4703, { 0x9c, 0xa5, 0x5b, 0x93, 0x25, 0x56, 0xf, 0xdb } };

cfg_struct_t<t_spectrum_color_info> cfg_spectrum_color_info(guid_cfg_spectrum_color_info,
    t_spectrum_color_info(blend_mode_counterclockwise, RGB(64, 0, 128), RGB(255, 255, 217)));

static const GUID guid_cfg_frames_per_second = { 0xd658d398, 0xd0da, 0x4bcb, { 0x8f, 0xcf, 0x29, 0xe2, 0x34, 0x57, 0x7e, 0x4 } };
static const GUID guid_cfg_lines_per_second = { 0x992af11b, 0xb317, 0x4b79, { 0xbf, 0x9d, 0xd9, 0xd7, 0xed, 0x67, 0x59, 0x86 } };

cfg_uint cfg_frames_per_second(guid_cfg_frames_per_second, 100);
cfg_uint cfg_lines_per_second(guid_cfg_lines_per_second, 50);

// {BA28E013-BC84-4074-9FC0-008437996930}
static const GUID guid_cfg_low_power_centibel = 
{ 0xba28e013, 0xbc84, 0x4074, { 0x9f, 0xc0, 0x0, 0x84, 0x37, 0x99, 0x69, 0x30 } };

// {9FBCCCD8-3F04-4A1A-99CF-707A1FB14CEF}
static const GUID guid_cfg_high_power_centibel = 
{ 0x9fbcccd8, 0x3f04, 0x4a1a, { 0x99, 0xcf, 0x70, 0x7a, 0x1f, 0xb1, 0x4c, 0xef } };

cfg_int cfg_low_power_centibel(guid_cfg_low_power_centibel, -800);
cfg_int cfg_high_power_centibel(guid_cfg_high_power_centibel, 0);

void g_create_mapper(service_ptr_t<colormap> & p_out, t_spectrum_color_info p_info)
{
    switch (p_info.m_blend_mode)
    {
    case blend_mode_clockwise:
        p_out = new service_impl_t<colormap_impl_blend_hsv>(p_info.m_low_energy, p_info.m_high_energy, true);
        break;
    case blend_mode_counterclockwise:
        p_out = new service_impl_t<colormap_impl_blend_hsv>(p_info.m_low_energy, p_info.m_high_energy, false);
        break;
    default:
        p_out = new service_impl_t<colormap_impl_blend_rgb>(p_info.m_low_energy, p_info.m_high_energy);
        break;
    }
}

t_spectrum_color_info::t_spectrum_color_info() : m_blend_mode(blend_mode_linear), m_low_energy(RGB(0, 0, 0)), m_high_energy(RGB(255, 255, 255)) {}
t_spectrum_color_info::t_spectrum_color_info(t_uint32 p_blend_mode, COLORREF p_low_energy, COLORREF p_high_energy) : m_blend_mode(p_blend_mode), m_low_energy(p_low_energy), m_high_energy(p_high_energy) {}

namespace pfc
{
    class format_html_color
    {
    public:
        format_html_color(COLORREF p_val);
        format_html_color(const format_int & p_source) {*this = p_source;}
        inline const char * get_ptr() const {return m_buffer;}
        inline operator const char*() const {return m_buffer;}
    private:
        char m_buffer[8];
    };

}

static char format_hex_char(unsigned p_val)
{
    PFC_ASSERT(p_val < 16);
    return (p_val < 10) ? p_val + '0' : p_val - 10 + 'A';
}

pfc::format_html_color::format_html_color(COLORREF p_val)
{
    m_buffer[0] = '#';
    m_buffer[1] = format_hex_char((GetRValue(p_val) >> 4) & 0xf);
    m_buffer[2] = format_hex_char(GetRValue(p_val) & 0xf);
    m_buffer[3] = format_hex_char((GetGValue(p_val) >> 4) & 0xf);
    m_buffer[4] = format_hex_char(GetGValue(p_val) & 0xf);
    m_buffer[5] = format_hex_char((GetBValue(p_val) >> 4) & 0xf);
    m_buffer[6] = format_hex_char(GetBValue(p_val) & 0xf);
    m_buffer[7] = 0;
}

void t_spectrum_color_info::to_text(pfc::string_base & p_text_out, const char * p_comment)
{
    p_text_out.reset();
    switch (m_blend_mode)
    {
    case blend_mode_linear:
        p_text_out << "lin";
        break;
    case blend_mode_clockwise:
        p_text_out << "cw";
        break;
    case blend_mode_counterclockwise:
        p_text_out << "ccw";
        break;
    default:
        console::formatter() << "invalid blend mode constant, defaulting to linear: " << m_blend_mode;
        p_text_out << "lin";
        break;
    }
    p_text_out << " " << pfc::format_html_color(m_low_energy);
    p_text_out << " " << pfc::format_html_color(m_high_energy);
}

class string_source
{
public:
    string_source(const char * p_data, t_size p_length)
    {
        m_data = p_data;
        m_length = pfc::strlen_max(p_data, p_length);
        m_position = 0;
    }

    string_source(pfc::string_base & p_data)
    {
        m_data = p_data.get_ptr();
        m_length = p_data.get_length();
        m_position = 0;
    }

    void advance(t_size p_length)
    {
        pfc::bug_check_assert(p_length + m_position <= m_length);

        m_position += p_length;
    }

    bool skip(const char * p_text, t_size p_text_length = pfc::infinite_size)
    {
        bool rv = test(p_text, p_text_length);
        if (rv) advance(pfc::strlen_max(p_text, p_text_length));
        return rv;
    }

    inline string_source & operator ++ () {advance(1); return *this;}


    bool test(const char * p_text, t_size p_text_length = pfc::infinite_size) const
    {
        return pfc::strcmp_partial_ex(&m_data[m_position], m_length - m_position, p_text, p_text_length) == 0;
    }

    inline char operator [] (t_size p_index) const {if (p_index + m_position < m_length) return m_data[p_index + m_position]; else return 0;}

private:
    const char * m_data;
    t_size m_length, m_position;
};

// look ahead
inline bool operator / (const string_source & p_source, const char * p_text) {return p_source.test(p_text);}

// consume
inline string_source & operator >> (string_source & p_source, const char * p_text) {if (!p_source.skip(p_text)) throw exception_io_data(); return p_source;}

bool ishexdigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

BYTE hextoi(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    throw pfc::exception_bug_check();
}

class parse_color_code
{
public:
    parse_color_code(COLORREF & p_color) : m_color(p_color) {}

    bool test(const string_source & p_source) const
    {
        if (p_source[0] == '#')
        {
            if (!ishexdigit(p_source[1])) return false;
            if (!ishexdigit(p_source[2])) return false;
            if (!ishexdigit(p_source[3])) return false;
            if (!ishexdigit(p_source[4])) return true;
            if (!ishexdigit(p_source[5])) return false;
            if (!ishexdigit(p_source[6])) return false;
            if (!ishexdigit(p_source[7])) return true;
        }
        else if (p_source[0] == '\3')
        {
            if (!ishexdigit(p_source[1])) return false;
            if (!ishexdigit(p_source[2])) return false;
            if (!ishexdigit(p_source[3])) return false;
            if (!ishexdigit(p_source[4])) return false;
            if (!ishexdigit(p_source[5])) return false;
            if (!ishexdigit(p_source[6])) return false;
            if (p_source[7] == '\3') return true;
        }
        return false;
    }

    void parse(string_source & p_source)
    {
        if (p_source[0] == '#')
        {
            if (ishexdigit(p_source[1]) && ishexdigit(p_source[2]) && ishexdigit(p_source[3]))
            {
                if (ishexdigit(p_source[4]))
                {
                    if (ishexdigit(p_source[5]) && ishexdigit(p_source[6]) && !ishexdigit(p_source[7]))
                    {
                        m_color = RGB(
                            hextoi(p_source[1]) * 16 + hextoi(p_source[2]),
                            hextoi(p_source[3]) * 16 + hextoi(p_source[4]),
                            hextoi(p_source[5]) * 16 + hextoi(p_source[6]));
                        p_source.advance(7);
                    }
                    else
                        throw exception_io_data();
                }
                else
                {
                    m_color = RGB(
                        hextoi(p_source[1]) * 17,
                        hextoi(p_source[2]) * 17,
                        hextoi(p_source[3]) * 17);
                    p_source.advance(4);
                }
            }
            else
                throw exception_io_data();
        }
        else if (p_source[0] == '\3')
        {
            if (ishexdigit(p_source[1]) && ishexdigit(p_source[2]) &&
                ishexdigit(p_source[3]) && ishexdigit(p_source[4]) && 
                ishexdigit(p_source[5]) && ishexdigit(p_source[6]) && 
                p_source[7] == '\3')
            {
                m_color = RGB(
                    hextoi(p_source[5]) * 16 + hextoi(p_source[6]),
                    hextoi(p_source[3]) * 16 + hextoi(p_source[4]),
                    hextoi(p_source[1]) * 16 + hextoi(p_source[2]));
                p_source.advance(8);
            }
            else
                throw exception_io_data();
        }
        else
            throw exception_io_data();
    }

private:
    COLORREF & m_color;
};

inline bool operator / (const string_source & p_source, const parse_color_code & p_parser) {return p_parser.test(p_source);}
inline string_source & operator >> (string_source & p_source, parse_color_code & p_parser) {p_parser.parse(p_source); return p_source;}

void t_spectrum_color_info::from_text(const char * p_text, pfc::string_base & p_comment_out)
{
    pfc::string8 text;
    static_api_ptr_t<titleformat_compiler> tfc;
    tfc->run(NULL, text, p_text);

    string_source source(text);

    t_blend_mode blend_mode;
    COLORREF low_energy, high_energy;

    // skip whitespace
    while (source.skip(" ")) {}

    // detect blend mode
    if (source.skip("lin"))
        blend_mode = blend_mode_linear;
    else if (source.skip("cw"))
        blend_mode = blend_mode_clockwise;
    else if (source.skip("ccw"))
        blend_mode = blend_mode_counterclockwise;
    else
        throw exception_io_data();

    // skip whitespace
    source >> " ";
    while (source.skip(" ")) {}

    source >> parse_color_code(low_energy);

    // skip whitespace
    source >> " ";
    while (source.skip(" ")) {}

    source >> parse_color_code(high_energy);

    m_blend_mode = blend_mode;
    m_low_energy = low_energy;
    m_high_energy = high_energy;
}
