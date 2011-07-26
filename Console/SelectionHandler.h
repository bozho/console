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
			const CDC& dcConsoleView, 
			const CRect& rectConsoleView, 
			ConsoleHandler& consoleHandler,
			SharedMemory<ConsoleParams>& consoleParams, 
			SharedMemory<ConsoleInfo>& consoleInfo, 
			SharedMemory<ConsoleCopy>& consoleCopyInfo, 
			int nCharWidth, 
			int nCharHeight);

		~SelectionHandler();

	public:

		void StartSelection(const COORD& coordInit, COLORREF crSelectionColor, shared_array<CharInfo> screenBuffer);
		void SelectWord(const COORD& coordInit, COLORREF crSelectionColor, shared_array<CharInfo> screenBuffer);
		void UpdateSelection(const COORD& coordCurrent, shared_array<CharInfo> screenBuffer);
		void UpdateSelection();
		bool CopySelection(const COORD& coordCurrent);
		void CopySelection();
		void EndSelection();
		void ClearSelection();

		inline SelectionState GetState() const;

		void BitBlt(CDC& offscreenDC);

	private:

		void GetSelectionCoordinates(COORD& coordStart, COORD& coordEnd);
		void GetFillRect(const COORD& coordStart, const COORD& coordEnd, CRect& fillRect);

	private:

		CWindow			m_consoleView;

		CDC				m_dcSelection;
		CBitmap			m_bmpSelection;

		CRect			m_rectConsoleView;

		ConsoleHandler&								m_consoleHandler;
		SharedMemory<ConsoleParams>&				m_consoleParams;
		SharedMemory<ConsoleInfo>&	m_consoleInfo;
		SharedMemory<ConsoleCopy>&					m_consoleCopyInfo;
		int				m_nCharWidth;
		int				m_nCharHeight;

		CBrush			m_paintBrush;
		CBrush			m_backgroundBrush;

		SelectionState	m_selectionState;

		COORD			m_coordInitial;
		COORD			m_coordCurrent;
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

