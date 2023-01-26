echo "Building"
if not exist .\build mkdir .\build
cd .\build
cmake ..\
cmake --build . --config Release
cd ..
echo "Copying to zip"
mkdir .\releases
Xcopy /E /I .\build\MiniMetersServer_artefacts\Release\VST3\MiniMetersServer.vst3 .\releases\MiniMetersServer.vst3
copy .\build\MiniMetersServer_artefacts\Release\CLAP\MiniMetersServer.clap .\releases
cd releases
tar.exe -a -c -f plugin-windows.zip MiniMetersServer.clap MiniMetersServer.vst3
cd ..
rmdir /s /Q .\releases\MiniMetersServer.vst3
del .\releases\MiniMetersServer.clap