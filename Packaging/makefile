!INCLUDE ../shared/version.h

release:
	@echo "$(MAKEDIR)\..\bin\ConsoleZ.$(VERSION).7z"
	"C:\Program Files\7-Zip\7z.exe" a -t7z -m9=lzma2 -mx -mmt2 "$(MAKEDIR)\..\bin\ConsoleZ.$(VERSION).7z" -xr!*.xml "$(MAKEDIR)\..\bin\Win32" "$(MAKEDIR)\..\bin\x64"
	@echo "$(MAKEDIR)\..\bin\ConsoleZ.x86.$(VERSION).zip"
	"C:\Program Files\7-Zip\7z.exe" a -tzip -mx9 -mmt2 "$(MAKEDIR)\..\bin\ConsoleZ.x86.$(VERSION).zip" -xr!*.pdb -xr!*.xml "$(MAKEDIR)\..\bin\Win32\Release\*"
	@echo "$(MAKEDIR)\..\bin\ConsoleZ.x64.$(VERSION).zip"
	"C:\Program Files\7-Zip\7z.exe" a -tzip -mx9 -mmt2 "$(MAKEDIR)\..\bin\ConsoleZ.x64.$(VERSION).zip" -xr!*.pdb -xr!*.xml "$(MAKEDIR)\..\bin\x64\Release\*"


clean:
	@echo clean
