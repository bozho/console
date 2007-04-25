#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template <class T>
class CHotkeyEditT 
	: public CWindowImpl<CHotkeyEditT<T>, CEdit>
{
	public:

		CHotkeyEditT(HWND hWnd = NULL);

		BEGIN_MSG_MAP(CHotkeyEditT<T>)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
			MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
			MESSAGE_HANDLER(WM_SYSKEYUP, OnKeyUp)
			MESSAGE_HANDLER(WM_CHAR, OnChar)
			MESSAGE_HANDLER(WM_SYSCHAR, OnChar)
			MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		END_MSG_MAP()

		LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	public:

		void GetHotKey(UINT& uiVk, WORD& wModifiers);
		void SetHotKey(UINT uiVk, WORD wModifiers);
		
		CString GetHotKeyName();
		CString GetHotKeyName(UINT uiVk, WORD wModifiers);

	private:

		CString GetKeyName(UINT uiVk, BOOL bExtendedKey);
		void SetHotkeyText();

	private:

		bool	m_bCtrlDown;
		bool	m_bShiftDown;
		bool	m_bAltDown;
		bool	m_bWinDown;
		bool	m_bAppDown;

		UINT	m_uiVirtualKey;
		bool	m_bExtended;

		UINT	m_uiHotkeyVk;
		WORD	m_wHotkeyModifiers;
};

typedef CHotkeyEditT<CWindow> CHotkeyEdit;

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
CHotkeyEditT<T>::CHotkeyEditT(HWND hWnd /*= NULL*/)
: m_bCtrlDown(false)
, m_bShiftDown(false)
, m_bAltDown(false)
, m_uiVirtualKey(0)
, m_bExtended(false)
, m_uiHotkeyVk(0)
, m_wHotkeyModifiers(0)
{
	m_hWnd = hWnd;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
LRESULT CHotkeyEditT<T>::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// key down, clear hotkey variables
	m_uiHotkeyVk		= 0;
	m_wHotkeyModifiers	= 0;

	if (wParam == VK_CONTROL)	m_bCtrlDown	= true;
	if (wParam == VK_SHIFT)		m_bShiftDown= true;
	if (wParam == VK_MENU)		m_bAltDown	= true;

	if ((wParam != VK_CONTROL) && (wParam != VK_SHIFT) && (wParam != VK_MENU))
	{
		// non-modifier key pressed
		m_uiVirtualKey	= static_cast<UINT>(wParam);
		m_bExtended		= (lParam & 0x01000000L) ? true : false;
	}

	SetHotkeyText();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
LRESULT CHotkeyEditT<T>::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == VK_CONTROL)	m_bCtrlDown	= false;
	if (wParam == VK_SHIFT)		m_bShiftDown= false;
	if (wParam == VK_MENU)		m_bAltDown	= false;

	if ((wParam != VK_CONTROL) && (wParam != VK_SHIFT) && (wParam != VK_MENU))
	{
		// non-modifier key up, set hotkey variables
		m_uiHotkeyVk		= m_uiVirtualKey;
		m_wHotkeyModifiers	= 0;

		if (m_bCtrlDown)	m_wHotkeyModifiers |= HOTKEYF_CONTROL;
		if (m_bShiftDown)	m_wHotkeyModifiers |= HOTKEYF_SHIFT;
		if (m_bAltDown)		m_wHotkeyModifiers |= HOTKEYF_ALT;
		if (m_bExtended)	m_wHotkeyModifiers |= HOTKEYF_EXT;

		m_uiVirtualKey	= 0;
		m_bExtended		= false;
	}

	SetHotkeyText();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
LRESULT CHotkeyEditT<T>::OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
LRESULT CHotkeyEditT<T>::OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return DLGC_WANTALLKEYS;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
void CHotkeyEditT<T>::GetHotKey(UINT& uiVk, WORD& wModifiers)
{
	uiVk		= m_uiHotkeyVk;
	wModifiers	= m_wHotkeyModifiers;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
void CHotkeyEditT<T>::SetHotKey(UINT uiVk, WORD wModifiers)
{
	m_uiHotkeyVk		= uiVk;
	m_wHotkeyModifiers	= wModifiers;

	m_uiVirtualKey	= 0;
	m_bExtended		= false;

	CString strKeyName(GetHotKeyName(uiVk, wModifiers));

	SetWindowText(strKeyName);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
CString CHotkeyEditT<T>::GetHotKeyName()
{
	return GetHotKeyName(m_uiHotkeyVk, m_wHotkeyModifiers);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
CString CHotkeyEditT<T>::GetHotKeyName(UINT uiVk, WORD wModifiers)
{
	CString	strKeyName(L"");

	if ((uiVk == 0) && (wModifiers == 0)) return CString(L"None");

	if (wModifiers & HOTKEYF_CONTROL)
	{
		strKeyName += GetKeyName(VK_CONTROL, FALSE);
		strKeyName += L"+";
	}

	if (wModifiers & HOTKEYF_SHIFT)
	{
		strKeyName += GetKeyName(VK_SHIFT, FALSE);
		strKeyName += L"+";
	}

	if (wModifiers & HOTKEYF_ALT)
	{
		strKeyName += GetKeyName(VK_MENU, FALSE);
		strKeyName += L"+";
	}

	strKeyName += GetKeyName(uiVk, wModifiers & HOTKEYF_EXT);

	return strKeyName;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
CString CHotkeyEditT<T>::GetKeyName(UINT uiVk, BOOL bExtendedKey)
{
	LONG lScanCode = ::MapVirtualKey(uiVk, 0) << 16;

	// if it's an extended key, add extended flag (bit 24)
	if (bExtendedKey) lScanCode |= 0x01000000L;

	CString strKeyName(L"");

	::GetKeyNameText(lScanCode, strKeyName.GetBufferSetLength(255), 255);
	strKeyName.ReleaseBuffer();

	return strKeyName;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

template<class T>
void CHotkeyEditT<T>::SetHotkeyText()
{
	CString strKeyName;
	WORD	wModifiers = 0;

	if (m_uiHotkeyVk == 0)
	{

		// keys are down, get name from temp variables
		if (m_bCtrlDown)	wModifiers |= HOTKEYF_CONTROL;
		if (m_bShiftDown)	wModifiers |= HOTKEYF_SHIFT;
		if (m_bAltDown)		wModifiers |= HOTKEYF_ALT;

		if (m_bExtended)	wModifiers |= HOTKEYF_EXT;
	
		strKeyName = GetHotKeyName(m_uiVirtualKey, wModifiers);

	}
	else
	{
		// keys are up, get name from hotkey variables
		strKeyName = GetHotKeyName(m_uiHotkeyVk, m_wHotkeyModifiers);
	}

	SetWindowText(strKeyName);
}

//////////////////////////////////////////////////////////////////////////////
