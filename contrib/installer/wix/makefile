
CANDLE = "$(WIX)bin\Candle.exe"
LIGHT = "$(WIX)\bin\Light.exe"
LIT = "$(WIX)\bin\Lit.exe"
DARK = "$(WIX)\bin\Dark.exe"
HEAT = "$(WIX)\bin\Heat.exe"
MELT = "$(WIX)\bin\Melt.exe"
TORCH = "$(WIX)\bin\Torch.exe"
SMOKE = "$(WIX)\bin\Smoke.exe"
PYRO = "$(WIX)\bin\Pyro.exe"

all: qtplatz.msi

clean:
	rm -f *.msi *.wixobj version.wxi *.zip *.bat *.exe *.obj

veryclean: clean
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Clean /p:Configuration=Release

qtplatz:
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Rebuild /p:Configuration=Release

version.exe: version.c
	cl version.c

version.wxi: version.exe
	git describe | version > version.wxi

copy.bat: version.exe
	git describe | version -b > copy.bat

qtplatz.msi: qtplatz.wixobj
	$(LIGHT) $?
	copy.bat

qtplatz.wixobj: version.wxi rename.bat qtplatz.wxs
	$(CANDLE) qtplatz.wxs

