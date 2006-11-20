#pragma once

//////////////////////////////////////////////////////////////////////////////

class SharedMemNames
{
	public:

		static wformat formatConsoleParams;
		static wformat formatInfo;
		static wformat formatCursorInfo;
		static wformat formatBuffer;
		static wformat formatCopyInfo;
		static wformat formatPasteInfo;
		static wformat formatMouseEvent;
		static wformat formatNewConsoleSize;
		static wformat formatNewScrollPos;

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

wformat SharedMemNames::formatConsoleParams(L"Console2_params_%1%");
wformat SharedMemNames::formatInfo(L"Console2_consoleInfo_%1%");
wformat SharedMemNames::formatCursorInfo(L"Console2_cursorInfo_%1%");
wformat SharedMemNames::formatBuffer(L"Console2_consoleBuffer_%1%");
wformat SharedMemNames::formatCopyInfo(L"Console2_consoleCopyInfo_%1%");
wformat SharedMemNames::formatPasteInfo(L"Console2_consolePasteInfo_%1%");
wformat SharedMemNames::formatMouseEvent(L"Console2_formatMouseEvent_%1%");
wformat SharedMemNames::formatNewConsoleSize(L"Console2_newConsoleSize_%1%");
wformat SharedMemNames::formatNewScrollPos(L"Console2_newScrollPos_%1%");

//////////////////////////////////////////////////////////////////////////////
