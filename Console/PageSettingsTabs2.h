
#pragma once

#include "PageSettingsTab.h"
#include "CFileNameEdit.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class PageSettingsTabs2
	: public CDialogImpl<PageSettingsTabs2>
	, public CWinDataExchange<PageSettingsTabs2>
	, public PageSettingsTab
{
	public:

		enum { IDD = IDD_SETTINGS_TABS_2 };

		PageSettingsTabs2(ConsoleSettings &consoleSettings);

		BEGIN_DDX_MAP(PageSettingsTabs2)
			if(nCtlID == (UINT)-1 || nCtlID == IDC_RADIO_BK_TYPE)
			{
				int var = static_cast<int>(m_tabData->backgroundImageType);
				DDX_Radio(IDC_RADIO_BK_TYPE, var, bSaveAndValidate);
				m_tabData->backgroundImageType = static_cast<BackgroundImageType>(var);
			}
			DDX_CHECK(IDC_CHECK_BK_RELATIVE, m_tabData->imageData.bRelative)
			DDX_CHECK(IDC_CHECK_BK_EXTEND, m_tabData->imageData.bExtend)
			DDX_CHECK(IDC_BTN_INHERIT_BACKGROUND, m_tabData->bInheritedBackground)
			DDX_TEXT(IDC_BK_IMAGE, m_strBkImage)
		END_DDX_MAP()

		BEGIN_MSG_MAP(PageSettingsTabs2)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_RANGE_CODE_HANDLER(IDC_RADIO_BK_TYPE, IDC_RADIO_BK_TYPE3, BN_CLICKED, OnClickedBkType)
			COMMAND_HANDLER(IDC_BK_COLOR, BN_CLICKED, OnClickedBkColor)
			COMMAND_ID_HANDLER(IDC_BTN_BROWSE_BK, OnBtnBrowseImage)
			COMMAND_HANDLER(IDC_TINT_COLOR, BN_CLICKED, OnClickedTintColor)
			COMMAND_HANDLER(IDC_BTN_INHERIT_BACKGROUND, BN_CLICKED, OnClickedBtnInheritBackground)
			COMMAND_HANDLER(IDC_BTN_SET_AS_DEFAULT_BACKGROUND, BN_CLICKED, OnClickedBtnSetAsDefaultBackground)
			COMMAND_HANDLER(IDC_CHECK_BK_RELATIVE, BN_CLICKED, OnClickedBkType)
			COMMAND_HANDLER(IDC_CHECK_BK_EXTEND, BN_CLICKED, OnClickedBkType)
			COMMAND_HANDLER(IDC_COMBO_BK_POS, CBN_SELCHANGE, OnCbnSelchangeComboBkPos)
			COMMAND_HANDLER(IDC_BK_IMAGE, EN_CHANGE, OnChangeBkImage)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		LRESULT OnClickedBkType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBkColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnBtnBrowseImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedTintColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnCbnSelchangeComboBkPos(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnChangeBkImage(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnClickedBtnInheritBackground(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedBtnSetAsDefaultBackground(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	public:

		void Load(std::shared_ptr<TabData>& tabData);
		void Save();

	private:
		void UpdateSliderText();
		void EnableControls();
		void Load();

		ConsoleSettings	&m_consoleSettings;
		std::shared_ptr<TabData>	m_tabData;

		CComboBox		m_comboBkPosition;
		CTrackBarCtrl	m_sliderTintOpacity;
		CStatic			m_staticTintOpacity;

		CStatic			m_staticBkColor;
		CStatic			m_staticTintColor;

		CFileNameEdit m_bkImageEdit;

		CString			m_strBkImage;
};

//////////////////////////////////////////////////////////////////////////////
