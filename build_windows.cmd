mkdir .\releases
Xcopy /E /I .\cmake-build-release\MiniMetersServer_artefacts\Release\VST3\MiniMetersServer.vst3 .\releases\MiniMetersServer.vst3
copy .\cmake-build-release\MiniMetersServer_artefacts\Release\CLAP\MiniMetersServer.clap .\releases
cd releases
tar.exe -a -c -f plugin-windows.zip MiniMetersServer.clap MiniMetersServer.vst3
cd ..
rmdir /s /Q .\releases\MiniMetersServer.vst3
del .\releases\MiniMetersServer.clap