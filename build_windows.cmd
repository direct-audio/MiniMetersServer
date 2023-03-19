echo "Building"
:: make build folder
if not exist .\build mkdir .\build
cd .\build

:: build
cmake ..\ || exit /b
cmake --build . --config Release || exit /b

cd ..
echo "Copying to zip"

mkdir .\releases

Xcopy /E /I .\build\MiniMetersServer_artefacts\Release\VST3\MiniMetersServer.vst3 .\releases\MiniMetersServer.vst3
copy .\build\MiniMetersServer_artefacts\Release\CLAP\MiniMetersServer.clap .\releases
cd releases
tar.exe -a -c -f plugin-windows.zip MiniMetersServer.clap MiniMetersServer.vst3
cd ..

if exist .\releases\MiniMetersServer.vst3 rmdir /s /Q .\releases\MiniMetersServer.vst3
if exist .\releases\MiniMetersServer.clap del .\releases\MiniMetersServer.clap