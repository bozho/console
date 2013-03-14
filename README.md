ConsoleZ
========

This is a modified version of Console 2 for a better experience under Windows Vista/7/8 and a better visual rendering.

This fork supports:

        + splittable tabs (horizontally and vertically)
        + Windows Vista aero glass theme
        + Windows 7 jumplist
        + Windows 8 wallpaper positions and slideshow
        + Zooming with Ctrl-Mouse
        + Quake style console animation
        + strict monospace font rendering

Changelog
=========

        * : fixed
        - : removed
        ! : changed
        + : added

Changes in 1.06.0 beta (14 Mar 2013)

        ! the cloned tab title is suffixed with (2)
        ! about dialog box improvement
        ! hotkeys are displayed in the menu
        ! restart is no longer needed for these appearance settings:
          "Caption"
          "Resizable"
          "Border"
          "Taskbar button"
        + new option "Net Only" to "Run as user"
          the user credentials are used only to access network resources
          the user is displayed in tab title surrounded with braces
        + "Run as user" supports UNC user names
        + new buttons in the toolbar:
          "Rename tab"
          "Split Horizontally"
          "Split Vertically"
        + new hotkeys:
          "New Tab 11" (default: CTRL+F11)
          "New Tab 12" (default: CTRL+F12)
          "Switch to left view" (default: ALT+LEFT)
          "Switch to right view" (default: ALT+RIGHT)
          "Switch to top view" (default: ALT+UP)
          "Switch to bottom view" (default: ALT+DOWN)
        + Windows key can be used in global hotkeys
        + The glass frame extended in client area (toolbar and tabs) can be used to
          move the window.
        + The glass frame extended in client area (tabs only) can be used to
          maximize or restore the window by double clicking.
        * If the only one tab was cloned, the "delete" button remained disabled.
        * Under Windows 8, the call to GetTextMetrics function failed with some fonts
          The return code wasn't checked and char metrics wasn't correctly initialized.
        * wrong tabs scrolling
        * close button drawing
        * background of tab was black with basic Windows theme
        * error handling and return code checking
        * Re-add tray icon if Windows Explorer restarts.

Changes in 1.05.0 (2 Feb 2013)

        ! FreeImage 3.15.4
        ! Visual Studio 2012
        ! WTL 8.1.12085
        ! force monospace displaying
          by adjusting the size of chars larger than average width font          
        ! limit vertical scrolling to the furthest buffer location viewed        
        + Windows 8 can use a wallpaper per monitor
        + display a closing confirmation when there is only one tab
          but multiple views
        + improve the status bar with:
          console buffer size, console screen size,
          selection size and console PID
        * dropping a file affects all views in a group
        * aero glass margins are resetted when composition change
          (hibernation disables DWM, at wakeup ConsoleZ was fully transparent)
