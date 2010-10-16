========================================================================
    DYNAMIC LINK LIBRARY : console_ru Project Overview
========================================================================

This project is resources of Console2, localized to Russian.
It can also be used as a sample for localization to other languages.

WHAT DO I NEED TO LOCALIZE CONSOLE2?

- this project;
- Visual Studio 2008
	See below, if you do not have one.

	If you use Express Edition, please, make sure that you can build
	Win32 native applications.
- few hours and a lot of will.


HOW DO I LOCALIZE CONSOLE2?

- make a copy of this project's folder;
- rename the project to console_TWOLETTERSLANGUAGE
	Just to be sure, consult the array of languages in Console.cpp.
	
	To rename the project:
	- rename the folder;
	- rename .vcproj file;
	- open renamed .vcproj in a text editor, find all instances of
	  the original name - "console_ru" - and replace them with your
	  new name.
	BE CAREFUL: some of them are in upper case.
- (just to be sure) copy console.rc and resource.h from main source tree;
- open the project, switch to Resource View and have fun;

If you use Git (http://code.google.com/p/msysgit/), add resource.h from
your new folder to .gitignore.

CAN I "LOCALIZE" TOOLBAR?

I thought you'd never ask :)
Yes, that's why toolbar.bmp is in the project's folder.


WHAT TO WATCH FOR DURING LOCALIZATION?

You need to follow these simple rules:
- do NOT modify resource.h;
- make sure that your .rc file is in the correct language
	To do that, you'll need to be sure that the block, similar to:

/////////////////////////////////////////////////////////////////////////////
// Russian resources

#if !defined(AFX_RESOURCE_DLL) || defined(AFX_TARG_RUS)
#ifdef _WIN32
LANGUAGE LANG_RUSSIAN, SUBLANG_DEFAULT
#pragma code_page(1251)
#endif //_WIN32

	has correct LANG_, SUBLANG_ and code_page values
- do NOT modify IDC_COMBO_* DLGINIT
	Basically, do not try to localize combo boxes in dialog editor.
	VS and WTL does not handle those resource in localization-friendly
	way, so you'll likely end up with garbage (?????) in your combos

You need to make sure that the resulting binaries are placed next to
the main executable. In the default tree, it would be bin\<platform>\release.


I AM DONE, BUT CONSOLE IS STILL IN ENGLISH. WHAT'S WRONG?

There are at least two possible explanations:
- do you run English version of Windows?
	If so, you can use an environment variable to force the Console
	into using your language.

	You can set it up permanently from My Computer's properties or
	just fire up Console, execute "set LANG=ru" (without quotes) and
	start another instance of Console.

- are you sure that you used the correct two letters?
	If you just tried to guess, you might be surprised.
	For example, do you know that Georgian has symbol "ka"? And
	Kannada is "kn"?

	What you can do to force your localization to be used is
	- set LANG=somespecialword
	- rename console_ru.dll console_somespecialword.dll
	- run the Console


HOW DO I LOCALIZE CONSOLE IF I DO NOT HAVE VISUAL STUDIO?

I guess you can ask me for help. Just keep in mind that:
- I might not speak your language, so it'll take several iterations;
- You will still need to translate strings in console.rc.


DO I GET A CREDIT?

Well... You better ask Marko (bozho), but I put my name into VERSION
resource and on ABOUTBOX.
