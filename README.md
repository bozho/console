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
