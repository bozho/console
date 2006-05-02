
#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class PageSettingsTabs2
	: public CDialogImpl<PageSettingsTabs2>
	, public CWinDataExchange<PageSettingsTabs2>
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_2 };

		PageSettingsTabs2();

		BEGIN_DDX_MAP(PageSettingsTabs2)
			DDX_RADIO(IDC_RADIO_BK_TYPE, m_nBkType)
			DDX_TEXT(IDC_BK_IMAGE, m_strBkImage)
			DDX_CHECK(IDC_CHECK_BK_RELATIVE, m_nRelative)
			DDX_CHECK(IDC_CHECK_BK_EXTEND, m_nExtend)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs2)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_RANGE_CODE_HANDLER(IDC_RADIO_BK_TYPE, IDC_RADIO_BK_TYPE3, BN_CLICKED, OnClickedBkType)
			COMMAND_HANDLER(IDC_BK_COLOR, BN_CLICKED, OnClickedBkColor)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_BK, OnBtnBrowseImage)
			COMMAND_HANDLER(IDC_TINT_COLOR, BN_CLICKED, OnClickedTintColor)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnClickedBkType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBkColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedTintColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);

	public:

		void UpdateSliderText();
		void EnableControls();

	public:

		shared_ptr<TabData>	m_tabData;

		CComboBox		m_comboBkPosition;
		CTrackBarCtrl	m_sliderTintOpacity;
		CStatic			m_staticTintOpacity;

		CStatic			m_staticCursorColor;
		CStatic			m_staticBkColor;
		CStatic			m_staticTintColor;

		int				m_nBkType;
		CString			m_strBkImage;
		int				m_nRelative;
		int				m_nExtend;
};

//////////////////////////////////////////////////////////////////////////////
