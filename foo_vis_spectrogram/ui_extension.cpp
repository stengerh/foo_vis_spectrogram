#include "stdafx.h"

#ifdef HAVE_COLUMNSUI_SUPPORT

#include "../columns_ui-sdk/ui_extension.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "CSpectrogramView.h"
#include "config.h"
#include "CVisualisationHost.h"

class uie_window_spectrogram : public uie::window
{
public:
    uie_window_spectrogram()
    {
        m_panel = NULL;
    }

    const GUID & get_extension_guid(void) const
    {
        // {99F54514-51E1-49E4-B0B3-5C1D2E5EB87E}
        static const GUID guid = 
        { 0x99f54514, 0x51e1, 0x49e4, { 0xb0, 0xb3, 0x5c, 0x1d, 0x2e, 0x5e, 0xb8, 0x7e } };

        return guid;
    }

    void get_name(pfc::string_base & p_out) const
    {
        p_out = "Spectrogram";
    }

    void get_category(pfc::string_base & p_out) const
    {
        p_out = "Visualisations";
    }

    bool is_available(const ui_extension::window_host_ptr &) const
    {
        return true;
    }

    HWND create_or_transfer_window(HWND parent ,const ui_extension::window_host_ptr & p_host,const ui_helpers::window_position_t & p_position)
    {
        if (m_panel == NULL)
        {
            m_panel = new CVisualisationPanelHost();
            CRect rc;
            p_position.convert_to_rect(rc);
            return m_panel->Create(parent, rc);
        }
        return NULL;
    }

    unsigned int get_type(void) const
    {
        return uie::type_panel;
    }

    void destroy_window(void)
    {
        if (m_panel != NULL && m_panel->IsWindow())
        {
            m_panel->DestroyWindow();
        }
        m_panel = NULL;
    }

    HWND get_wnd(void) const
    {
        if (m_panel != NULL && m_panel->IsWindow())
            return m_panel->m_hWnd;
        return NULL;
    }

private:
    CVisualisationPanelHost * m_panel;
};

static uie::window_factory<uie_window_spectrogram> foo_uie_window;

#endif
