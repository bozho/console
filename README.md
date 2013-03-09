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

Changes in 1.06.0 beta (9 Mar 2013)

        ! the cloned tab title is suffixed with (2)
        ! about dialog box improvement
        + new option "Net Only" to "Run as user"
          the user credentials are used only to access network resources
          the user is displayed in tab title surrounded with braces
        + "Run as user" supports UNC user names
        + hotkeys are displayed in the menu
        + new buttons in the toolbar:
          rename tab
          split horizontally
          split vertically
        + new hotkeys:
          switch to left view (default ALT+LEFT)
          switch to right view (default ALT+RIGHT)
          switch to top view (default ALT+UP)
          switch to bottom view (default ALT+DOWN)
        * If the only one tab was cloned, the "delete" button remained disabled.
        * Under Windows 8, the call to GetTextMetrics function failed with some fonts
          The return code wasn't checked and char metrics wasn't correctly initialized.
        * wrong tabs scrolling
        * close button drawing
        * background of tab was black with basic Windows theme
        * error handling and return code checking

Changes in 1.05.0 (2 Feb 2013)

        ! FreeImage 3.15.4
        ! Visual Studio 2012
        ! force monospace displaying
          by adjusting the size of chars larger than average width font
        ! limit vertical scrolling to the furthest buffer location viewed
        ! WTL 8.1.12085
        + Windows 8 can use a wallpaper per monitor
        + display a closing confirmation when there is only one tab
          but multiple views
        + improve the status bar with:
          console buffer size, console screen size,
          selection size and console PID
        * dropping a file affects all views in a group
        * aero glass margins are resetted when composition change
          (hibernation disables DWM, at wakeup ConsoleZ was fully transparent)
