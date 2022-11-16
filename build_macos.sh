set -e
cmake -B build-macos -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-macos --target MiniMetersServer_VST3
cmake --build build-macos --target MiniMetersServer_AU
cmake --build build-macos --target MiniMetersServer_CLAP

mkdir -p ./releases
#cleanupp
rm -r -f ./releases/plugin_macos.zip
#moving
cp -r ./build-macos/MiniMetersServer_artefacts/RelWithDebInfo/VST3/MiniMetersServer.vst3 ./releases
cp -r ./build-macos/MiniMetersServer_artefacts/RelWithDebInfo/AU/MiniMetersServer.component ./releases
cp -r ./build-macos/MiniMetersServer_artefacts/RelWithDebInfo/CLAP/MiniMetersServer.clap ./releases
#zipping
cd releases
echo "ZIP"
rm -r -f ./plugin_macos.zip
zip -r ./plugin_macos.zip ./MiniMetersServer.vst3 ./MiniMetersServer.component ./MiniMetersServer.clap
