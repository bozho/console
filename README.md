ConsoleZ
========

![screenshot005](https://github.com/cbucher/console/wiki/screenshot001.png)

This is a modified version of Console 2 for a better experience under Windows Vista/7/8 and a better visual rendering.

This fork supports:

        + Splitting Tabs into views (horizontally and vertically)
        + Grouping views (so input sent to one goes to all of them)
        + Windows Vista aero glass theme
        + Windows 7 jumplist
        + Windows 7 wallpaper positions and slideshow
        + Windows 8 wallpapers on dual screen
        + Zooming with Ctrl-Mouse
        + Quake style console animation
        + Strict monospace font rendering
        + Settable opacity of text background color
        + Full screen
        + and more...

Changelog
=========

        * : fixed
        - : removed
        ! : changed
        + : added
        
Changes in 1.07.0 beta 3 (28 Apr 2013)

        ! Limits the maximized windows size on monitors with taskbar
          so ConsoleZ, with no border, no longer overlaps the taskbar when WIN+UP is pressed
        ! restart is no longer needed for this appearance setting: "Tabs on bottom"
        + Full screen support
        + Full screen button in taskbar
        + Full screen hot key (default: F11)
        + Full screen settings: can start in full screen and select the full screen monitor
        + Left and right delimiters for word selection can be set
        + View can be closed from the menu
        + New behavior settings:
          "Allow closing the last tab/view"
          "Confirm before closing multiple tabs/views"
        * caption was removed when border was unchecked

Changes in 1.06.0 (24 Mar 2013)

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
        * When files are dropped on a view, this view gets the focus.
        * Title was not updated when view changed.

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
