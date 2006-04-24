#include "stdafx.h"
#include "resource.h"

#include "DlgSettingsBehavior.h"

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

DlgSettingsBehavior::DlgSettingsBehavior(CComPtr<IXMLDOMElement>& pOptionsRoot)
: DlgSettingsBase(pOptionsRoot)
{
	IDD = IDD_SETTINGS_BEHAVIOR;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_behaviorSettings.Load(m_pOptionsRoot);

	m_nCopyOnSelect	= m_behaviorSettings.copyPasteSettings.bCopyOnSelect ? 1 : 0;
	m_nNoWrap		= m_behaviorSettings.copyPasteSettings.bNoWrap ? 1 : 0;
	m_nTrimSpaces	= m_behaviorSettings.copyPasteSettings.bTrimSpaces ? 1 : 0;

	m_nMouseDrag	= m_behaviorSettings.mouseDragSettings.bMouseDrag ? 1 : 0;
	m_nInverseShift	= m_behaviorSettings.mouseDragSettings.bInverseShift ? 1 : 0;

	DoDataExchange(DDX_LOAD);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

LRESULT DlgSettingsBehavior::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wID == IDOK)
	{
		DoDataExchange(DDX_SAVE);

		m_behaviorSettings.copyPasteSettings.bCopyOnSelect	= (m_nCopyOnSelect > 0);
		m_behaviorSettings.copyPasteSettings.bNoWrap		= (m_nNoWrap > 0);
		m_behaviorSettings.copyPasteSettings.bTrimSpaces	= (m_nTrimSpaces > 0);

		m_behaviorSettings.mouseDragSettings.bMouseDrag		= (m_nMouseDrag > 0);
		m_behaviorSettings.mouseDragSettings.bInverseShift	= (m_nInverseShift > 0);

		BehaviorSettings& behaviorSettings = g_settingsHandler->GetBehaviorSettings();

		behaviorSettings.copyPasteSettings.bCopyOnSelect	= m_behaviorSettings.copyPasteSettings.bCopyOnSelect;
		behaviorSettings.copyPasteSettings.bNoWrap			= m_behaviorSettings.copyPasteSettings.bNoWrap;
		behaviorSettings.copyPasteSettings.bTrimSpaces		= m_behaviorSettings.copyPasteSettings.bTrimSpaces;

		behaviorSettings.mouseDragSettings.bMouseDrag		= m_behaviorSettings.mouseDragSettings.bMouseDrag;
		behaviorSettings.mouseDragSettings.bInverseShift	= m_behaviorSettings.mouseDragSettings.bInverseShift;

		m_behaviorSettings.Save(m_pOptionsRoot);
	}

	DestroyWindow();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

