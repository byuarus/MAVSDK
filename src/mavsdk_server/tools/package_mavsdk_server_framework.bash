#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../../../build"
IOS_DEVICE_BACKEND_DIR="${BUILD_DIR}/ios-device/src/mavsdk_server/src"
IOS_SIMULATOR_ARM64_BACKEND_DIR="${BUILD_DIR}/ios-simulator-arm64/src/mavsdk_server/src"
IOS_SIMULATOR_X86_64_BACKEND_DIR="${BUILD_DIR}/ios-simulator-x86_64/src/mavsdk_server/src"
IOS_SIMULATOR_UNIVERSAL_DIR="${BUILD_DIR}/ios-simulator-universal/src/mavsdk_server/src"
MACOS_ARM64_BACKEND_DIR="${BUILD_DIR}/macos-framework-arm64/src/mavsdk_server/src"
MACOS_X86_64_BACKEND_DIR="${BUILD_DIR}/macos-framework-x86_64/src/mavsdk_server/src"
MACOS_UNIVERSAL_DIR="${BUILD_DIR}/macos-framework-universal/src/mavsdk_server/src"

if [ -d "${BUILD_DIR}/mavsdk_server.xcframework" ]; then
    echo "${BUILD_DIR}/mavsdk_server.xcframework already exists! Aborting..."
    exit 1
fi

echo "Preparing universal iOS simulator framework..."
rm -rf "${IOS_SIMULATOR_UNIVERSAL_DIR}"
mkdir -p "${IOS_SIMULATOR_UNIVERSAL_DIR}"
cp -R "${IOS_SIMULATOR_ARM64_BACKEND_DIR}/mavsdk_server.framework" "${IOS_SIMULATOR_UNIVERSAL_DIR}/mavsdk_server.framework"
lipo -create \
    "${IOS_SIMULATOR_ARM64_BACKEND_DIR}/mavsdk_server.framework/mavsdk_server" \
    "${IOS_SIMULATOR_X86_64_BACKEND_DIR}/mavsdk_server.framework/mavsdk_server" \
    -output "${IOS_SIMULATOR_UNIVERSAL_DIR}/mavsdk_server.framework/mavsdk_server"

echo "Preparing universal macOS framework..."
rm -rf "${MACOS_UNIVERSAL_DIR}"
mkdir -p "${MACOS_UNIVERSAL_DIR}"
cp -R "${MACOS_ARM64_BACKEND_DIR}/mavsdk_server.framework" "${MACOS_UNIVERSAL_DIR}/mavsdk_server.framework"
lipo -create \
    "${MACOS_ARM64_BACKEND_DIR}/mavsdk_server.framework/mavsdk_server" \
    "${MACOS_X86_64_BACKEND_DIR}/mavsdk_server.framework/mavsdk_server" \
    -output "${MACOS_UNIVERSAL_DIR}/mavsdk_server.framework/mavsdk_server"
ln -sf Versions/Current/Modules "${MACOS_UNIVERSAL_DIR}/mavsdk_server.framework"

echo "Creating xcframework..."
xcodebuild -create-xcframework \
    -framework "${IOS_DEVICE_BACKEND_DIR}/mavsdk_server.framework" \
    -debug-symbols "${IOS_DEVICE_BACKEND_DIR}/mavsdk_server.framework.dSYM" \
    -framework "${IOS_SIMULATOR_UNIVERSAL_DIR}/mavsdk_server.framework" \
    -framework "${MACOS_UNIVERSAL_DIR}/mavsdk_server.framework" \
    -output "${BUILD_DIR}/mavsdk_server.xcframework"

find "${BUILD_DIR}/mavsdk_server.xcframework" -path '*/mavsdk_server.framework/mavsdk_server' -exec chmod +x {} +

cd "${BUILD_DIR}"
zip -9 -r mavsdk_server.xcframework.zip mavsdk_server.xcframework

shasum -a 256 mavsdk_server.xcframework.zip | awk '{ print $1 }' > mavsdk_server.xcframework.zip.sha256

echo "Success! You will find the xcframework in ${BUILD_DIR}!"
