
CANDLE = "$(WIX)bin\Candle.exe"
LIGHT = "$(WIX)\bin\Light.exe"
LIT = "$(WIX)\bin\Lit.exe"
DARK = "$(WIX)\bin\Dark.exe"
HEAT = "$(WIX)\bin\Heat.exe"
MELT = "$(WIX)\bin\Melt.exe"
TORCH = "$(WIX)\bin\Torch.exe"
SMOKE = "$(WIX)\bin\Smoke.exe"
PYRO = "$(WIX)\bin\Pyro.exe"

all: qtplatz.msi version.txt

clean:
	rm -f *.msi *.wixobj

version.txt:
	git describe >> version.txt

qtplatz.msi: qtplatz.wixobj
	$(LIGHT) $?

qtplatz.wixobj: qtplatz.wxs
	$(CANDLE) $?

