#ifndef __FOO_VIS_SPECTRUM__CVISUALISATIONHOST_H__
#define __FOO_VIS_SPECTRUM__CVISUALISATIONHOST_H__

// Options
#define HAVE_REDUCE_BANDING_OPTION
#define HAVE_SMOOTH_SCALING_OPTION
#define HAVE_FFT_RESOLUTION_OPTION

template <class T, class TBase /* = CWindow */, class TWinTraits /* = CControlWinTraits */>
class CVisualisationHostImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
	typedef CVisualisationHostImpl<T, TBase, TWinTraits> thisClass;

	BEGIN_MSG_MAP(thisClass)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		MSG_WM_CONTEXTMENU(OnContextMenu)
	END_MSG_MAP()

	void set_callback(ui_element_instance_callback_ptr p_callback) {m_callback = p_callback;}

	bool GetReduceBandingMode() {return m_view.get_reduce_banding();}
	void SetReduceBandingMode(bool p_state) {m_view.set_reduce_banding(p_state);}

	bool GetSmoothScalingMode() {return m_view.get_smooth_scaling();}
	void SetSmoothScalingMode(bool p_state) {m_view.set_smooth_scaling(p_state);}

protected:
	CSpectrogramView m_view;
	ui_element_instance_callback_ptr m_callback;

	// context menu command IDs
	enum {
		ID_CONFIG = 1,
		ID_SMOOTH_SCALING,
		ID_REDUCE_BANDING,

		ID_LAST_COMMAND
	};

	LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (DefWindowProc() != 0)
			return -1;

		CRect rcClient;
		GetClientRect(&rcClient);

		if (m_view.Create(m_hWnd, rcClient) == NULL)
			return -1;

		return 0;
	}

	void OnDestroy()
	{
	}

	void OnSize(UINT nType, CSize size)
	{
		if (nType != SIZE_MINIMIZED)
		{
			UpdateLayout();
		}
	}

	void OnEnterSizeMove()
	{
		if (m_view.IsWindow())
			m_view.SendMessage(WM_ENTERSIZEMOVE);
	}

	void OnExitSizeMove()
	{
		if (m_view.IsWindow())
			m_view.SendMessage(WM_EXITSIZEMOVE);
	}

	void OnContextMenu(HWND hWndFrom, CPoint ptScreen)
	{
		if (m_callback.is_valid() && m_callback->is_edit_mode_enabled()) {
			SetMsgHandled(FALSE);
			return;
		}

		CMenu menu;
		menu.CreatePopupMenu();

		//spectrum_buffer::t_scaling_mode scaling_quality = m_view.get_scaling_quality();

		menu.AppendMenu(MF_STRING, ID_CONFIG, _T("Settings..."));

#ifdef HAVE_REDUCE_BANDING_OPTION
		menu.AppendMenu(MF_STRING | (GetReduceBandingMode() ? MF_CHECKED : 0), ID_REDUCE_BANDING, _T("Reduce Banding"));
#endif

#ifdef HAVE_SMOOTH_SCALING_OPTION
		menu.AppendMenu(MF_STRING | (GetSmoothScalingMode() ? MF_CHECKED : 0), ID_SMOOTH_SCALING, _T("Smooth Scaling"));
#endif

		if (menu.GetMenuItemCount() > 0)
		{
			const int rv = (int)menu.TrackPopupMenu(TPM_RETURNCMD | TPM_RIGHTBUTTON |TPM_NONOTIFY, ptScreen.x, ptScreen.y, m_hWnd);

			if (rv == ID_CONFIG)
			{
				static_api_ptr_t<ui_control>()->show_preferences(guid_prefs_vis_spectrum);
			}
#ifdef HAVE_REDUCE_BANDING_OPTION
			else if (rv == ID_REDUCE_BANDING)
			{
				SetReduceBandingMode(!GetReduceBandingMode());
			}
#endif
#ifdef HAVE_SMOOTH_SCALING_OPTION
			else if (rv == ID_SMOOTH_SCALING)
			{
				SetSmoothScalingMode(!GetSmoothScalingMode());
			}
#endif
		}
	}

	void UpdateLayout()
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		if (m_view.IsWindow())
		{
			m_view.SetWindowPos(NULL, &rcClient, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
};


typedef CWinTraitsOR<0, WS_EX_STATICEDGE> CVisPanelWinTraits;

class CVisualisationPanelHost :
	public CVisualisationHostImpl<CVisualisationPanelHost, CWindow, CVisPanelWinTraits>
{
public:
	typedef CVisualisationHostImpl<CVisualisationPanelHost, CWindow, CVisPanelWinTraits> baseClass;

	BEGIN_MSG_MAP(CVisualisationPanelHost)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()

	DECLARE_WND_CLASS(_T("SpectrumPanelClass"));
};

#endif
