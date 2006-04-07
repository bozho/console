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
			selstateSelected		= 0x03
		};

	public:
		SelectionHandler(HWND hwndConsoleView, const CDC& dcConsoleView, const CRect& rectConsoleView, int nCharWidth, int nCharHeight, COLORREF crSelectionColor);
		~SelectionHandler();

	public:

		void StartSelection(const CPoint& pointInitial, SHORT sXMax, SHORT sYMax);
		void UpdateSelection(const CPoint& point);
		void CopySelection(const CPoint* pPoint, const SharedMemory<CHAR_INFO>& consoleBuffer);
		void EndSelection();
		void ClearSelection();

		inline SelectionState GetState() const;

		void BitBlt(CDC& offscreenDC);

	private:

		void GetSelectionCoordinates(COORD& coordStart, COORD& coordEnd);

	private:

		CWindow			m_consoleView;

		CDC				m_dcSelection;
		CBitmap			m_bmpSelection;

		CRect			m_rectConsoleView;

		int				m_nCharWidth;
		int				m_nCharHeight;

		CBrush			m_paintBrush;
		CBrush			m_backgroundBrush;

		SelectionState	m_selectionState;

		COORD			m_coordInitial;
		COORD			m_coordCurrent;
		SHORT			m_sXMax;
		SHORT			m_sYMax;

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

