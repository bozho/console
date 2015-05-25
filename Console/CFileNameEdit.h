#pragma once

namespace WTL
{

class CFileNameEdit:public CWindowImpl<CFileNameEdit,CEdit,CControlWinTraits >
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_FileNameEdit"),CEdit::GetWndClassName())

	BEGIN_MSG_MAP(CFileNameEdit)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_DROPFILES,OnDropFiles)

		//COMMAND_HANDLER(IDC_TAB_ICON, EN_CHANGE, OnTabIconChange)
	END_MSG_MAP()
	LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& /*bHandled*/)
	{
		LRESULT lRes=DefWindowProc(uMsg,wParam,lParam);
		return lRes;
	}

	LRESULT OnDropFiles(UINT /*uMsg*/,WPARAM wParam,LPARAM /*lParam*/,BOOL& /*bHandled*/)
	{
		HDROP hDrop = reinterpret_cast<HDROP>(wParam);

		SetActiveWindow();

		UINT iFileCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if(iFileCount>0)
		{
			CString	strFilename;
			::DragQueryFile(hDrop, 0, strFilename.GetBuffer(MAX_PATH), MAX_PATH);
			strFilename.ReleaseBuffer();

			if( this->OnDropFile(strFilename) )
				SetWindowText(strFilename);
		}
		::DragFinish(hDrop);
		return 0;
	}

	virtual bool OnDropFile(CString& /*strFilename*/) { return true; }
};

}; // namespace WTL
