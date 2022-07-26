set -e
cmake -B build-macos -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos --target MiniMetersServer_VST3
cmake --build build-macos --target MiniMetersServer_AU
#zip
echo "Zipping to plugin_macos.zip"
mkdir -p ./releases
rm -r -f ./releases/plugin_macos.zip
zip -r ./releases/plugin_macos.zip ./build-macos/MiniMetersServer_artefacts/Release/VST3/MiniMetersServer.vst3 ./build-macos/MiniMetersServer_artefacts/Release/AU/MiniMetersServer.component