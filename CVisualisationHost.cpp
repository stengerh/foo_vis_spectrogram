#include "stdafx.h"
#include "colormap.h"
#include "spectrum_chunk.h"
#include "CSpectrumView.h"
#include "CVisualisationHost.h"
#include "config.h"

CVisualisationPopupHost *CVisualisationPopupHost::g_instance = NULL;

LRESULT CVisualisationPopupHost::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (baseClass::OnCreate(lpCreateStruct) != 0)
		return -1;

	cfg_popup_window_placement.on_window_creation(m_hWnd);

	return 0;
}

void CVisualisationPopupHost::OnDestroy()
{
	cfg_popup_window_placement.on_window_destruction(m_hWnd);

	if (g_instance == this)
		g_instance = NULL;
}

void CVisualisationPopupHost::OnClose()
{
	cfg_popup_enabled = false;
	DestroyWindow();
}

void CVisualisationPopupHost::OnKeyDown(TCHAR nChar, UINT nRepeatCount, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		OnClose();
	}
}
