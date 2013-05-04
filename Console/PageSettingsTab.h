
#pragma once

class PageSettingsTab
{
public:
	virtual void Load(std::shared_ptr<TabData>& tabData) = 0;
	virtual void Save() = 0;
	void Show(int nCmdShow)
	{
		CWindow *wnd = dynamic_cast<CWindow *>(this);
		if (wnd)
			wnd->ShowWindow(nCmdShow);
	}
};
