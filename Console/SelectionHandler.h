#pragma once

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

class SelectionHandler
{
	public:

		enum SelectionState
		{
			selstateNoSelection		= 0x00,
			selstateStartedSelecting= 0x01,
			selstateSelecting		= 0x02,
			selstateSelected		= 0x03,
			selstateSelectWord		= 0x04,
		};

	public:
		SelectionHandler(
			const CWindow& consoleView, 
#ifndef _USE_AERO
			const CDC& dcConsoleView, 
			const CRect& rectConsoleView, 
#endif //_USE_AERO
			ConsoleHandler& consoleHandler,
			SharedMemory<ConsoleParams>& consoleParams, 
			SharedMemory<ConsoleInfo>& consoleInfo, 
			SharedMemory<ConsoleCopy>& consoleCopyInfo, 
			int nCharWidth, 
			int nCharHeight,
			int nVInsideBorder,
			int nHInsideBorder,
			std::shared_ptr<TabData> tabData);

		~SelectionHandler();

	public:

		void StartSelection(const COORD& coordInit, CharInfo screenBuffer [], SelectionType selectionType);
		void SelectWord(const COORD& coordInit);
		void UpdateSelection(const COORD& coordCurrent, CharInfo screenBuffer []);
		void UpdateSelection();
		bool CopySelection(const COORD& coordCurrent);
		void CopySelection();
		void EndSelection();
		void ClearSelection();
		void SelectAll();

		inline SelectionState GetState() const;
		DWORD GetSelectionSize(void);

#ifdef _USE_AERO
		void Draw(CDC& offscreenDC);
#else //_USE_AERO
		void BitBlt(CDC& offscreenDC);

	private:
		void GetFillRect(const COORD& coordStart, const COORD& coordEnd, CRect& fillRect);
#endif //_USE_AERO

	private:

		void GetSelectionCoordinates(COORD& coordStart, COORD& coordEnd);

	private:

		CWindow			m_consoleView;

#ifndef _USE_AERO
		CDC				m_dcSelection;
		CBitmap			m_bmpSelection;

		CRect			m_rectConsoleView;

		CBrush			m_paintBrush;
		CBrush			m_backgroundBrush;
#endif //_USE_AERO

		ConsoleHandler&               m_consoleHandler;
		SharedMemory<ConsoleParams>&  m_consoleParams;
		SharedMemory<ConsoleInfo>&    m_consoleInfo;
		SharedMemory<ConsoleCopy>&    m_consoleCopyInfo;
		int				m_nCharWidth;
		int				m_nCharHeight;
		int				m_nVInsideBorder;
		int				m_nHInsideBorder;

		SelectionState m_selectionState;
		SelectionType  m_selectionType;

		COORD			m_coordInitial;
		COORD			m_coordCurrent;

		SHORT			m_coordInitialXLeading;
		SHORT			m_coordInitialXTrailing;

		std::shared_ptr<TabData>      m_tabData;
};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

inline SelectionHandler::SelectionState SelectionHandler::GetState() const
{ 
	return m_selectionState;
}

//////////////////////////////////////////////////////////////////////////////

