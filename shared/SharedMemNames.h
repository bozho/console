#pragma once

//////////////////////////////////////////////////////////////////////////////

class SharedMemNames
{
	public:

		static boost::wformat formatConsoleParams;
		static boost::wformat formatInfo;
		static boost::wformat formatCursorInfo;
		static boost::wformat formatBuffer;
		static boost::wformat formatCopyInfo;
		static boost::wformat formatTextInfo;
		static boost::wformat formatMouseEvent;
		static boost::wformat formatNewConsoleSize;
		static boost::wformat formatNewScrollPos;
		static boost::wformat formatWatchdog;

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

boost::wformat SharedMemNames::formatConsoleParams(L"Console2_params_%1%");
boost::wformat SharedMemNames::formatInfo(L"Console2_consoleInfo_%1%");
boost::wformat SharedMemNames::formatCursorInfo(L"Console2_cursorInfo_%1%");
boost::wformat SharedMemNames::formatBuffer(L"Console2_consoleBuffer_%1%");
boost::wformat SharedMemNames::formatCopyInfo(L"Console2_consoleCopyInfo_%1%");
boost::wformat SharedMemNames::formatTextInfo(L"Console2_consoleTextInfo_%1%");
boost::wformat SharedMemNames::formatMouseEvent(L"Console2_consoleMouseEvent_%1%");
boost::wformat SharedMemNames::formatNewConsoleSize(L"Console2_newConsoleSize_%1%");
boost::wformat SharedMemNames::formatNewScrollPos(L"Console2_newScrollPos_%1%");
boost::wformat SharedMemNames::formatWatchdog(L"Local\\Console2_parentProcessExit_%1%");

//////////////////////////////////////////////////////////////////////////////
