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
		static boost::wformat formatMouseEvent;
		static boost::wformat formatNewConsoleSize;
		static boost::wformat formatNewScrollPos;
		static boost::wformat formatWatchdog;
		static boost::wformat formatAdmin;
		static boost::wformat formatPipeName;
		static boost::wformat formatMultipleInfo;

};

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

boost::wformat SharedMemNames::formatConsoleParams(L"ConsoleZ_params_%1%");
boost::wformat SharedMemNames::formatInfo(L"ConsoleZ_consoleInfo_%1%");
boost::wformat SharedMemNames::formatCursorInfo(L"ConsoleZ_cursorInfo_%1%");
boost::wformat SharedMemNames::formatBuffer(L"ConsoleZ_consoleBuffer_%1%");
boost::wformat SharedMemNames::formatCopyInfo(L"ConsoleZ_consoleCopyInfo_%1%");
boost::wformat SharedMemNames::formatMouseEvent(L"ConsoleZ_consoleMouseEvent_%1%");
boost::wformat SharedMemNames::formatNewConsoleSize(L"ConsoleZ_newConsoleSize_%1%");
boost::wformat SharedMemNames::formatNewScrollPos(L"ConsoleZ_newScrollPos_%1%");
boost::wformat SharedMemNames::formatWatchdog(L"Local\\ConsoleZ_parentProcessExit_%1%");
boost::wformat SharedMemNames::formatAdmin(L"ConsoleZ_admin_%1%");
boost::wformat SharedMemNames::formatPipeName(L"\\\\.\\pipe\\ConsoleZ_pipe_%1%");
boost::wformat SharedMemNames::formatMultipleInfo(L"ConsoleZ_multipleInfo_%1%");

//////////////////////////////////////////////////////////////////////////////
