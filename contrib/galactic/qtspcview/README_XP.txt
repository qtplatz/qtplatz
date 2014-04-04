http://stackoverflow.com/questions/13130713/how-to-compile-for-win-xp-with-visual-studio-2012

Two things should be done:

Configuration Properties > General page, change Platform Toolset to: Visual Studio 2012 - Windows XP (v110_xp);

Linker > System. Change Subsystem to: Console/Windows.

Details explanation here: http://software.intel.com/en-us/articles/linking-applications-using-visual-studio-2012-to-run-on-windows-xp
