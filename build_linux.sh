#!/bin/bash
set -e

cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux --target MiniMetersServer_VST3
cmake --build build-linux --target MiniMetersServer_CLAP
#zip
echo "Zipping to plugin_linux.zip"
mkdir -p ./releases
rm -r -f ./releases/plugin-linux.zip
cp -r ./build-linux/MiniMetersServer_artefacts/Release/VST3/MiniMetersServer.vst3 ./MiniMetersServer.vst3
cp -r ./build-linux/MiniMetersServer_artefacts/Release/CLAP/MiniMetersServer.clap ./
zip -r ./releases/plugin-linux.zip ./MiniMetersServer.vst3 ./MiniMetersServer.clap
rm -r ./MiniMetersServer.vst3
rm -r ./MiniMetersServer.clap