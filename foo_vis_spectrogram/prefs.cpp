#include "stdafx.h"
#include "colormap.h"
#include "config.h"
#include "resource.h"

struct t_custom_colors
{
	DWORD m_colors[16];

	t_custom_colors()
	{
		pfc::fill_array_t(m_colors, RGB(255, 255, 255));
	}
};

static const GUID guid_cfg_custom_colors = { 0xa89b6fef, 0x3266, 0x4f1c, { 0x81, 0xbd, 0x3e, 0x65, 0x73, 0x69, 0x93, 0xf4 } };

cfg_struct_t<t_custom_colors> cfg_custom_colors(guid_cfg_custom_colors, t_custom_colors());

class CSpectrumPrefsDialog :
	public CDialogImpl<CSpectrumPrefsDialog>
{
public:
	enum {IDD = IDD_CONFIG};

	BEGIN_MSG_MAP(CSpectrumPrefsDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnButtonClicked)
		COMMAND_HANDLER_EX(IDC_STYLELINE, CBN_EDITCHANGE, OnStyleLineEditChange)
		COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnEditChange)
	END_MSG_MAP();

private:
	virtual void OnFinalMessage(HWND hWnd);

	LRESULT OnInitDialog(HWND hWndFocus, LPARAM lParamInit);
	void OnDestroy();
	void OnButtonClicked(UINT nCode, int nId, HWND hWndFrom);
	void OnEditChange(UINT nCode, int nId, HWND hWndFrom);
	void OnStyleLineEditChange(UINT nCode, int nId, HWND hWndFrom);

	void UpdateButtonBitmaps();
	void UpdateStyleLine();

	void EditColor(COLORREF & p_color);

    static void CreateButtonBitmap(CButton & button, CBitmap & bitmap, int width, int height);
    static void DrawSolidButtonBitmap(CButton & button, CBitmap & bitmap, COLORREF color);
    static void DrawGradientButtonBitmap(CButton & button, CBitmap & bitmap, service_ptr_t<colormap> mapper);

	WTL::CButton
		m_wndLowEnergy,
		m_wndHighEnergy,
		m_wndBlendLinear,
		m_wndBlendCW,
		m_wndBlendCCW;

	CBitmap
		m_bmLowEnergy,
		m_bmHighEnergy,
		m_bmBlendLinear,
		m_bmBlendCW,
		m_bmBlendCCW;
};

void CSpectrumPrefsDialog::OnFinalMessage(HWND hWnd)
{
	delete this;
}

LRESULT CSpectrumPrefsDialog::OnInitDialog(HWND hWndFocus, LPARAM lParamInit)
{
	m_wndLowEnergy.Attach(GetDlgItem(IDC_LOW_ENERGY));
	m_wndHighEnergy.Attach(GetDlgItem(IDC_HIGH_ENERGY));

	m_wndBlendLinear.Attach(GetDlgItem(IDC_BLEND_LINEAR));
	m_wndBlendCW.Attach(GetDlgItem(IDC_BLEND_CW));
	m_wndBlendCCW.Attach(GetDlgItem(IDC_BLEND_CCW));

    int iconWidth = GetSystemMetrics(SM_CXSMICON);
    int iconHeight = GetSystemMetrics(SM_CYSMICON);

    CreateButtonBitmap(m_wndLowEnergy, m_bmLowEnergy, iconWidth, iconHeight);
    CreateButtonBitmap(m_wndHighEnergy, m_bmHighEnergy, iconWidth, iconHeight);
   
    CreateButtonBitmap(m_wndBlendLinear, m_bmBlendLinear, iconWidth * 3, iconHeight);
    CreateButtonBitmap(m_wndBlendCW, m_bmBlendCW, iconWidth * 3, iconHeight);
    CreateButtonBitmap(m_wndBlendCCW, m_bmBlendCCW, iconWidth * 3, iconHeight);

    UpdateButtonBitmaps();
	UpdateStyleLine();

	switch (cfg_spectrum_color_info.get_value().m_blend_mode)
	{
	case blend_mode_clockwise:
		m_wndBlendCW.SetCheck(BST_CHECKED);
		break;
	case blend_mode_counterclockwise:
		m_wndBlendCCW.SetCheck(BST_CHECKED);
		break;
	default:
		m_wndBlendLinear.SetCheck(BST_CHECKED);
		break;
	}

	SetDlgItemInt(IDC_REDRAW_PER_SECOND, cfg_frames_per_second, FALSE);
	SetDlgItemInt(IDC_LINES_PER_SECOND, cfg_lines_per_second, FALSE);

	return 0;
}

void CSpectrumPrefsDialog::OnDestroy()
{
}

void CSpectrumPrefsDialog::OnButtonClicked(UINT nCode, int nId, HWND hWndFrom)
{
	if (hWndFrom == m_wndLowEnergy)
	{
		EditColor(cfg_spectrum_color_info.get_value().m_low_energy);
	}
	else if (hWndFrom == m_wndHighEnergy)
	{
		EditColor(cfg_spectrum_color_info.get_value().m_high_energy);
	}
	else if (hWndFrom == m_wndBlendLinear)
	{
		cfg_spectrum_color_info.get_value().m_blend_mode = blend_mode_linear;
		m_wndBlendCW.SetCheck(BST_UNCHECKED);
		m_wndBlendCCW.SetCheck(BST_UNCHECKED);
		UpdateStyleLine();
		g_fire_spectrum_colors_changed(cfg_spectrum_color_info);
	}
	else if (hWndFrom == m_wndBlendCW)
	{
		cfg_spectrum_color_info.get_value().m_blend_mode = blend_mode_clockwise;
		m_wndBlendLinear.SetCheck(BST_UNCHECKED);
		m_wndBlendCCW.SetCheck(BST_UNCHECKED);
		UpdateStyleLine();
		g_fire_spectrum_colors_changed(cfg_spectrum_color_info);
	}
	else if (hWndFrom == m_wndBlendCCW)
	{
		cfg_spectrum_color_info.get_value().m_blend_mode = blend_mode_counterclockwise;
		m_wndBlendLinear.SetCheck(BST_UNCHECKED);
		m_wndBlendCW.SetCheck(BST_UNCHECKED);
		UpdateStyleLine();
		g_fire_spectrum_colors_changed(cfg_spectrum_color_info);
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

void CSpectrumPrefsDialog::OnStyleLineEditChange(UINT nCode, int nId, HWND hWndFrom)
{
	try
	{
		string_utf8_from_window style(hWndFrom);
		t_spectrum_color_info info;
		info.from_text(style);
		cfg_spectrum_color_info = info;
		UpdateButtonBitmaps();
		g_fire_spectrum_colors_changed(cfg_spectrum_color_info);
	}
	catch (std::exception)
	{
	}
}

void CSpectrumPrefsDialog::OnEditChange(UINT nCode, int nId, HWND hWndFrom)
{
	if (nId == IDC_REDRAW_PER_SECOND)
	{
		BOOL bTrans;
		UINT val = GetDlgItemInt(IDC_REDRAW_PER_SECOND, &bTrans, FALSE);
		if (bTrans) cfg_frames_per_second = val;
	}
	else if (nId == IDC_LINES_PER_SECOND)
	{
		BOOL bTrans;
		UINT val = GetDlgItemInt(IDC_LINES_PER_SECOND, &bTrans, FALSE);
		if (bTrans) cfg_lines_per_second = val;
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

void CSpectrumPrefsDialog::UpdateButtonBitmaps()
{
	t_spectrum_color_info info = cfg_spectrum_color_info;

	DrawSolidButtonBitmap(m_wndLowEnergy, m_bmLowEnergy, info.m_low_energy);
	DrawSolidButtonBitmap(m_wndHighEnergy, m_bmHighEnergy, info.m_high_energy);

	service_ptr_t<colormap> mapper;

	info.m_blend_mode = blend_mode_linear;
	g_create_mapper(mapper, info);
	DrawGradientButtonBitmap(m_wndBlendLinear, m_bmBlendLinear, mapper);
	m_wndBlendLinear.SetCheck((cfg_spectrum_color_info.get_value().m_blend_mode == blend_mode_linear) ? BST_CHECKED : BST_UNCHECKED);

	info.m_blend_mode = blend_mode_clockwise;
	g_create_mapper(mapper, info);
	DrawGradientButtonBitmap(m_wndBlendCW, m_bmBlendCW, mapper);
	m_wndBlendCW.SetCheck((cfg_spectrum_color_info.get_value().m_blend_mode == blend_mode_clockwise) ? BST_CHECKED : BST_UNCHECKED);

	info.m_blend_mode = blend_mode_counterclockwise;
	g_create_mapper(mapper, info);
	DrawGradientButtonBitmap(m_wndBlendCCW, m_bmBlendCCW, mapper);
	m_wndBlendCCW.SetCheck((cfg_spectrum_color_info.get_value().m_blend_mode == blend_mode_counterclockwise) ? BST_CHECKED : BST_UNCHECKED);
}

void CSpectrumPrefsDialog::UpdateStyleLine()
{
	pfc::string8 style;
	cfg_spectrum_color_info.get_value().to_text(style);
	uSetDlgItemText(m_hWnd, IDC_STYLELINE, style);
}

void CSpectrumPrefsDialog::EditColor(COLORREF & p_color)
{
	DWORD color = p_color;
	t_custom_colors custom_colors = cfg_custom_colors;
	if (uChooseColor(&color, m_hWnd, custom_colors.m_colors))
	{
		p_color = color;
		cfg_custom_colors = custom_colors;
		UpdateButtonBitmaps();
		UpdateStyleLine();
		g_fire_spectrum_colors_changed(cfg_spectrum_color_info);
	}
}

void CSpectrumPrefsDialog::CreateButtonBitmap(CButton & button, CBitmap & bitmap, int width, int height)
{
    CRect rcClient;
    button.GetClientRect(&rcClient);
    CClientDC dcWin(button);
    bitmap.CreateCompatibleBitmap(dcWin, width, height);

    button.SetBitmap(bitmap);
}

void CSpectrumPrefsDialog::DrawSolidButtonBitmap(CButton & button, CBitmap & bitmap, COLORREF color)
{
    CSize size;
    bitmap.GetSize(size);

	CClientDC dcWin(button);
	CDC dcMem;
	dcMem.CreateCompatibleDC(dcWin);

	CBitmapHandle orgBitmap = dcMem.SelectBitmap(bitmap);
    dcMem.FillSolidRect(CRect(CPoint(), size), 0xFF000000);
    dcMem.FillSolidRect(1, 1, size.cx - 2, size.cy - 2, color);
	dcMem.SelectBitmap(orgBitmap);

    button.SetBitmap(bitmap);
}

void CSpectrumPrefsDialog::DrawGradientButtonBitmap(CButton & button, CBitmap & bitmap, service_ptr_t<colormap> mapper)
{
    CSize size;
    bitmap.GetSize(size);

    CClientDC dcWin(button);
	CDC dcMem;
	dcMem.CreateCompatibleDC(dcWin);

	CBitmapHandle orgBitmap = dcMem.SelectBitmap(bitmap);

	pfc::array_t<audio_sample> samples;
	pfc::array_t<COLORREF> colors;

    int width = size.cx - 2;
    int height = size.cy - 2;

    dcMem.FillSolidRect(CRect(CPoint(), size), 0xFF000000);

	samples.set_size(width);
	colors.set_size(width);
	for (int n = 0; n < width; n++)
		samples[n] = n / (audio_sample)width;
	
	mapper->map(samples.get_ptr(), colors.get_ptr(), width);

	for (int n = 0; n < width; n++)
		dcMem.FillSolidRect(n + 1, 1, 1, height, colors[n]);

	dcMem.SelectBitmap(orgBitmap);

	button.SetBitmap(bitmap);
}

const GUID guid_prefs_vis_spectrum = { 0xed947acc, 0x53b8, 0x4cd4, { 0xbc, 0x34, 0x5b, 0x65, 0xc2, 0x7f, 0xc8, 0x94 } };

class preferences_page_vis_spectrum : public preferences_page
{
public:
	virtual HWND create(HWND parent)
	{
		CSpectrumPrefsDialog * dlg = new CSpectrumPrefsDialog();
		HWND hWnd = dlg->Create(parent);
		if (hWnd == NULL)
			delete dlg;
		return hWnd;
	}

	virtual const char * get_name()
	{
		return "Spectrum";
	}

	virtual GUID get_guid()
	{
		return guid_prefs_vis_spectrum;
	}

	virtual GUID get_parent_guid()
	{
		return guid_visualisations;
	}

	virtual bool reset_query()
	{
		return false;
	}

	virtual void reset()
	{
		// TODO
	}
};

static preferences_page_factory_t<preferences_page_vis_spectrum> foo_preferences_page;
