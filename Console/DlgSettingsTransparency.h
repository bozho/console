
#pragma once

#include "DlgSettingsBase.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class DlgSettingsTransparency
	: public DlgSettingsBase
{
	public:

		DlgSettingsTransparency(CComPtr<IXMLDOMElement>& pOptionsRoot);

		BEGIN_DDX_MAP(DlgSettingsTransparency)
			DDX_RADIO(IDC_TRANSPARENCY_TYPE, reinterpret_cast<int&>(m_transparencySettings.transType))
		END_DDX_MAP()

		BEGIN_MSG_MAP(DlgSettingsTransparency)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_HANDLER(IDC_KEY_COLOR, BN_CLICKED, OnClickedKeyColor)
			COMMAND_RANGE_CODE_HANDLER(IDC_TRANSPARENCY_TYPE, IDC_TRANSPARENCY_TYPE5, BN_CLICKED, OnClickedTransType)
		END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//		LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//		LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//		LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedKeyColor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/);
		LRESULT OnClickedTransType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	private:

		void UpdateSliderText(HWND hwnd);
		void EnableTransparencyControls();

	private:

		TransparencySettings		m_transparencySettings;

		CTrackBarCtrl				m_sliderActiveAlpha;
		CTrackBarCtrl				m_sliderInactiveAlpha;
};

//////////////////////////////////////////////////////////////////////////////
