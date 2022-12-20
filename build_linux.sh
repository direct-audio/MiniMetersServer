set -e
cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --target MiniMetersServer_VST3
cmake --build build-linux --target MiniMetersServer_CLAP
#zip
echo "Zipping to plugin_linux.zip"
mkdir -p ./releases
rm -r -f ./releases/plugin-linux.zip
zip -r ./releases/plugin-linux.zip ./build-linux/MiniMetersServer_artefacts/Release/VST3/MiniMetersServer.vst3 ./build-linux/MiniMetersServer_artefacts/Release/CLAP/MiniMetersServer.clap