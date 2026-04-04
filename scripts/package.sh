#!/bin/bash
set -euo pipefail

# ------------------------------------------------------------------
# Corvid Audio: build, sign, notarize, and package a plugin as a DMG
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
#   ./scripts/package.sh <plugin>         # package one plugin
#   ./scripts/package.sh all              # package all plugins
#
#   Plugins: 2-OP, Dist308, Life, Loc-Box
# ------------------------------------------------------------------

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${REPO_ROOT}"

# ---- Signing identity ----
SIGN_ID="Developer ID Application"

# ---- Notarization profile ----
NOTARY_PROFILE="notarytool-profile"

BUILD_DIR="build"

# ------------------------------------------------------------------
# Plugin registry: PRODUCT_NAME  CMAKE_TARGET  ARTEFACT_PREFIX  VERSION  DESCRIPTION
# ------------------------------------------------------------------
declare -A PLUGIN_TARGET PLUGIN_ARTEFACT PLUGIN_VERSION PLUGIN_DESC

PLUGIN_TARGET[2-OP]="TwoOpFM"
PLUGIN_ARTEFACT[2-OP]="TwoOpFM"
PLUGIN_VERSION[2-OP]="1.1.0"
PLUGIN_DESC[2-OP]="2-operator FM synthesizer"

PLUGIN_TARGET[Dist308]="Dist308"
PLUGIN_ARTEFACT[Dist308]="Dist308"
PLUGIN_VERSION[Dist308]="1.1.0"
PLUGIN_DESC[Dist308]="ProCo Rat-inspired distortion"

PLUGIN_TARGET[Life]="Life"
PLUGIN_ARTEFACT[Life]="Life"
PLUGIN_VERSION[Life]="0.4.1"
PLUGIN_DESC[Life]="Analog character and warmth"

PLUGIN_TARGET[Loc-Box]="LocBox"
PLUGIN_ARTEFACT[Loc-Box]="LocBox"
PLUGIN_VERSION[Loc-Box]="0.3.1"
PLUGIN_DESC[Loc-Box]="Shure Level Loc brickwall limiter emulation"

ALL_PLUGINS=("2-OP" "Dist308" "Life" "Loc-Box")

# ------------------------------------------------------------------
# Helpers
# ------------------------------------------------------------------
usage() {
    echo "Usage: $0 <plugin|all>"
    echo ""
    echo "Plugins: ${ALL_PLUGINS[*]}"
    exit 1
}

configure() {
    echo "==> Configuring build..."
    cmake -B "${BUILD_DIR}" -G Ninja \
        -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER="$(xcrun -f clang)" \
        -DCMAKE_CXX_COMPILER="$(xcrun -f clang++)"
}

package_plugin() {
    local NAME="$1"
    local TARGET="${PLUGIN_TARGET[$NAME]}"
    local ARTEFACT="${PLUGIN_ARTEFACT[$NAME]}"
    local VERSION="${PLUGIN_VERSION[$NAME]}"
    local DESC="${PLUGIN_DESC[$NAME]}"

    local ARTEFACTS="${BUILD_DIR}/plugins/${NAME}/${ARTEFACT}_artefacts/Release"
    local AU_SRC="${ARTEFACTS}/AU/${NAME}.component"
    local VST3_SRC="${ARTEFACTS}/VST3/${NAME}.vst3"
    local DMG_STAGING="${BUILD_DIR}/dmg-staging-${NAME}"
    local DMG_OUT="${BUILD_DIR}/${NAME}-${VERSION}-Mac.dmg"

    # -- Build --
    echo ""
    echo "==> Building ${NAME} v${VERSION}..."
    cmake --build "${BUILD_DIR}" --config Release \
        --target "${TARGET}_AU" "${TARGET}_VST3"

    # -- Sign --
    echo "==> Signing ${NAME}..."
    codesign --force --sign "${SIGN_ID}" \
        --options runtime --timestamp --deep \
        "${AU_SRC}"

    codesign --force --sign "${SIGN_ID}" \
        --options runtime --timestamp --deep \
        "${VST3_SRC}"

    echo "==> Verifying signatures..."
    codesign --verify --strict "${AU_SRC}"
    codesign --verify --strict "${VST3_SRC}"

    # -- Stage --
    echo "==> Staging DMG..."
    rm -rf "${DMG_STAGING}" "${DMG_OUT}"
    mkdir -p "${DMG_STAGING}"

    cp -R "${AU_SRC}" "${DMG_STAGING}/"
    cp -R "${VST3_SRC}" "${DMG_STAGING}/"

    cat > "${DMG_STAGING}/README.txt" << README
${NAME} v${VERSION}
by Corvid Audio

${DESC}.

Install by copying the plugins to:
  AU:   /Library/Audio/Plug-Ins/Components/
  VST3: /Library/Audio/Plug-Ins/VST3/

Restart your DAW after installation.
README

    ln -s "/Library/Audio/Plug-Ins/Components" "${DMG_STAGING}/AU Plugins"
    ln -s "/Library/Audio/Plug-Ins/VST3" "${DMG_STAGING}/VST3 Plugins"

    # -- DMG --
    echo "==> Creating DMG..."
    hdiutil create -volname "${NAME} ${VERSION}" \
        -srcfolder "${DMG_STAGING}" \
        -ov -format UDZO \
        "${DMG_OUT}"

    codesign --force --sign "${SIGN_ID}" --timestamp "${DMG_OUT}"

    # -- Notarize --
    echo "==> Submitting for notarization..."
    xcrun notarytool submit "${DMG_OUT}" \
        --keychain-profile "${NOTARY_PROFILE}" \
        --wait

    echo "==> Stapling notarization ticket..."
    xcrun stapler staple "${DMG_OUT}"

    # -- Verify --
    echo "==> Verifying..."
    spctl --assess --type open --context context:primary-signature "${DMG_OUT}"

    rm -rf "${DMG_STAGING}"
    echo "==> Done: ${DMG_OUT}"
}

# ------------------------------------------------------------------
# Main
# ------------------------------------------------------------------
[[ $# -lt 1 ]] && usage

configure

if [[ "$1" == "all" ]]; then
    for p in "${ALL_PLUGINS[@]}"; do
        package_plugin "$p"
    done
else
    # Validate plugin name
    if [[ -z "${PLUGIN_TARGET[$1]+x}" ]]; then
        echo "Error: unknown plugin '$1'"
        usage
    fi
    package_plugin "$1"
fi

echo ""
echo "==> All done!"
