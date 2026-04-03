#!/bin/bash
set -euo pipefail

# ------------------------------------------------------------------
# Dist308: build, sign, notarize, and package as a DMG
#
# Prerequisites:
#   - Apple Developer account with "Developer ID Application" cert
#     installed in your keychain
#   - An app-specific password stored in keychain:
#     xcrun notarytool store-credentials "notarytool-profile" \
#         --apple-id "you@email.com" \
#         --team-id "YOURTEAMID" \
#         --password "xxxx-xxxx-xxxx-xxxx"
#
# Usage:
#   ./scripts/package.sh
# ------------------------------------------------------------------

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${REPO_ROOT}"

PLUGIN_NAME="Dist308"
VERSION="1.1.0"
BUILD_DIR="build"
ARTEFACTS="${BUILD_DIR}/Dist308_artefacts/Release"
AU_SRC="${ARTEFACTS}/AU/${PLUGIN_NAME}.component"
VST3_SRC="${ARTEFACTS}/VST3/${PLUGIN_NAME}.vst3"
DMG_STAGING="${BUILD_DIR}/dmg-staging"
DMG_OUT="${BUILD_DIR}/${PLUGIN_NAME}-${VERSION}.dmg"

# ---- Signing identity ----
# Change this to match your Developer ID certificate name.
# Run: security find-identity -v -p codesigning
SIGN_ID="Developer ID Application"

# ---- Notarization profile ----
# Name of the keychain profile created by xcrun notarytool store-credentials
NOTARY_PROFILE="notarytool-profile"

# ---- Dependency paths ----
JUCE_DIR="${JUCE_DIR:-${REPO_ROOT}/../JUCE}"

# ------------------------------------------------------------------
# 1. Build
# ------------------------------------------------------------------
echo "==> Building ${PLUGIN_NAME} v${VERSION}..."
cmake -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$(xcrun -f clang)" \
    -DCMAKE_CXX_COMPILER="$(xcrun -f clang++)"

cmake --build "${BUILD_DIR}" --config Release

# ------------------------------------------------------------------
# 2. Code sign (hardened runtime + timestamp, required for notarization)
# ------------------------------------------------------------------
echo "==> Signing..."
codesign --force --sign "${SIGN_ID}" \
    --options runtime --timestamp --deep \
    "${AU_SRC}"

codesign --force --sign "${SIGN_ID}" \
    --options runtime --timestamp --deep \
    "${VST3_SRC}"

echo "==> Verifying signatures..."
codesign --verify --strict "${AU_SRC}"
codesign --verify --strict "${VST3_SRC}"

# ------------------------------------------------------------------
# 3. Create DMG
# ------------------------------------------------------------------
echo "==> Building DMG..."
rm -rf "${DMG_STAGING}" "${DMG_OUT}"
mkdir -p "${DMG_STAGING}"

# Copy signed plugins into staging
cp -R "${AU_SRC}" "${DMG_STAGING}/"
cp -R "${VST3_SRC}" "${DMG_STAGING}/"

# Add a readme
cat > "${DMG_STAGING}/README.txt" << README
Dist308 v${VERSION}
by Corvid Audio

Install by copying the plugins to:
  AU:   /Library/Audio/Plug-Ins/Components/
  VST3: /Library/Audio/Plug-Ins/VST3/

Restart your DAW after installation.
README

# Add symlinks to install destinations for drag-and-drop
ln -s "/Library/Audio/Plug-Ins/Components" "${DMG_STAGING}/AU Plugins"
ln -s "/Library/Audio/Plug-Ins/VST3" "${DMG_STAGING}/VST3 Plugins"

# Create the DMG
hdiutil create -volname "${PLUGIN_NAME} ${VERSION}" \
    -srcfolder "${DMG_STAGING}" \
    -ov -format UDZO \
    "${DMG_OUT}"

# Sign the DMG itself
codesign --force --sign "${SIGN_ID}" --timestamp "${DMG_OUT}"

# ------------------------------------------------------------------
# 4. Notarize
# ------------------------------------------------------------------
echo "==> Submitting for notarization..."
xcrun notarytool submit "${DMG_OUT}" \
    --keychain-profile "${NOTARY_PROFILE}" \
    --wait

echo "==> Stapling notarization ticket..."
xcrun stapler staple "${DMG_OUT}"

# ------------------------------------------------------------------
# 5. Verify
# ------------------------------------------------------------------
echo "==> Final verification..."
spctl --assess --type open --context context:primary-signature "${DMG_OUT}"
echo ""
echo "==> Done! Distributable DMG: ${DMG_OUT}"
