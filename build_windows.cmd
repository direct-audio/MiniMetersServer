mkdir .\releases
Xcopy /E /I .\cmake-build-release\MiniMetersServer_artefacts\Release\VST3\MiniMetersServer.vst3 .\releases\MiniMetersServer.vst3
copy .\cmake-build-release\MiniMetersServer_artefacts\Release\CLAP\MiniMetersServer.clap .\releases
tar.exe -a -c -f .\releases\plugin-windows.zip .\releases\MiniMetersServer.clap .\releases\MiniMetersServer.vst3

rmdir /s /Q .\releases\MiniMetersServer.vst3
del .\releases\MiniMetersServer.clap