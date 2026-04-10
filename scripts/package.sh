#!/opt/homebrew/bin/bash
set -euo pipefail

# ------------------------------------------------------------------
# Corvid Audio: build, sign, notarize, and package plugins as DMG+ZIP
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
#   ./scripts/package.sh all              # package all plugins + suite bundle
#
#   Plugins: 2-OP, Dist308, Life, Loc-Box
# ------------------------------------------------------------------

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${REPO_ROOT}"

# ---- Suite version ----
SUITE_VERSION="1.3.0"

# ---- Signing identity ----
SIGN_ID="Developer ID Application"

# ---- Notarization profile ----
NOTARY_PROFILE="notarytool-profile"

BUILD_DIR="build"
DIST_DIR="${BUILD_DIR}/dist"

# ------------------------------------------------------------------
# Plugin registry
# ------------------------------------------------------------------
declare -A PLUGIN_TARGET
declare -A PLUGIN_ARTEFACT
declare -A PLUGIN_VERSION
declare -A PLUGIN_DESC

PLUGIN_TARGET[2-OP]="TwoOpFM"
PLUGIN_ARTEFACT[2-OP]="TwoOpFM"
PLUGIN_VERSION[2-OP]="1.2.0"
PLUGIN_DESC[2-OP]="2-operator FM synthesizer"

PLUGIN_TARGET[Broken]="Broken"
PLUGIN_ARTEFACT[Broken]="Broken"
PLUGIN_VERSION[Broken]="0.4.0"
PLUGIN_DESC[Broken]="Circuit failure emulator"

PLUGIN_TARGET[Dist308]="Dist308"
PLUGIN_ARTEFACT[Dist308]="Dist308"
PLUGIN_VERSION[Dist308]="1.3.0"
PLUGIN_DESC[Dist308]="ProCo Rat-inspired distortion"

PLUGIN_TARGET[Life]="Life"
PLUGIN_ARTEFACT[Life]="Life"
PLUGIN_VERSION[Life]="0.6.0"
PLUGIN_DESC[Life]="Analog character and warmth"

PLUGIN_TARGET[Headroom]="Headroom"
PLUGIN_ARTEFACT[Headroom]="Headroom"
PLUGIN_VERSION[Headroom]="0.3.0"
PLUGIN_DESC[Headroom]="Hard clipper with threshold control"

PLUGIN_TARGET[Loc-Box]="LocBox"
PLUGIN_ARTEFACT[Loc-Box]="LocBox"
PLUGIN_VERSION[Loc-Box]="0.4.0"
PLUGIN_DESC[Loc-Box]="Shure Level Loc brickwall limiter emulation"

ALL_PLUGINS=("2-OP" "Broken" "Dist308" "Headroom" "Life" "Loc-Box")

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

# build_all: compile every plugin in one cmake invocation
build_all() {
    echo ""
    echo "==> Building all plugins..."
    cmake --build "${BUILD_DIR}" --config Release
}

# build_plugin NAME: compile AU + VST3 for a single plugin
build_plugin() {
    local NAME="$1"
    local T="${PLUGIN_TARGET[$NAME]}"
    echo ""
    echo "==> Building ${NAME}..."
    cmake --build "${BUILD_DIR}" --config Release \
        --target "${T}_AU" --target "${T}_VST3"
}

# sign_plugin NAME: codesign AU + VST3 for a single plugin
sign_plugin() {
    local NAME="$1"
    local ARTEFACT="${PLUGIN_ARTEFACT[$NAME]}"
    local ARTEFACTS="${BUILD_DIR}/plugins/${NAME}/${ARTEFACT}_artefacts/Release"
    local AU_SRC="${ARTEFACTS}/AU/${NAME}.component"
    local VST3_SRC="${ARTEFACTS}/VST3/${NAME}.vst3"

    echo "==> Signing ${NAME}..."
    codesign --force --sign "${SIGN_ID}" \
        --options runtime --timestamp --deep "${AU_SRC}"
    codesign --force --sign "${SIGN_ID}" \
        --options runtime --timestamp --deep "${VST3_SRC}"

    echo "==> Verifying signatures..."
    codesign --verify --strict "${AU_SRC}"
    codesign --verify --strict "${VST3_SRC}"
}

# stage_plugin NAME STAGING_DIR: copy AU + VST3 into a staging folder
stage_plugin() {
    local NAME="$1"
    local STAGING="$2"
    local ARTEFACT="${PLUGIN_ARTEFACT[$NAME]}"
    local ARTEFACTS="${BUILD_DIR}/plugins/${NAME}/${ARTEFACT}_artefacts/Release"

    cp -R "${ARTEFACTS}/AU/${NAME}.component" "${STAGING}/"
    cp -R "${ARTEFACTS}/VST3/${NAME}.vst3"    "${STAGING}/"
}

# write_readme NAME VERSION DESC STAGING_DIR
write_readme() {
    local NAME="$1"
    local VERSION="$2"
    local DESC="$3"
    local STAGING="$4"

    cat > "${STAGING}/README.txt" << README
${NAME} v${VERSION}
by Corvid Audio

${DESC}.

Install by copying the plugins to:
  AU:   /Library/Audio/Plug-Ins/Components/
  VST3: /Library/Audio/Plug-Ins/VST3/

Restart your DAW after installation.
README
}

# notarize_dmg PATH: notarize and staple a DMG
notarize_dmg() {
    local TARGET="$1"
    echo "==> Submitting for notarization: $(basename "${TARGET}")..."
    xcrun notarytool submit "${TARGET}" \
        --keychain-profile "${NOTARY_PROFILE}" \
        --wait
    echo "==> Stapling..."
    xcrun stapler staple "${TARGET}"
    spctl --assess --type open --context context:primary-signature "${TARGET}"
}

# notarize_zip PATH: notarize a ZIP (stapler does not support ZIP)
notarize_zip() {
    local TARGET="$1"
    echo "==> Submitting for notarization: $(basename "${TARGET}")..."
    xcrun notarytool submit "${TARGET}" \
        --keychain-profile "${NOTARY_PROFILE}" \
        --wait
    echo "==> (ZIP: staple not applicable, notarization ticket embedded in signed binaries)"
}

# make_dmg_and_zip STAGING_DIR VOLNAME OUT_BASE
# Produces OUT_BASE.dmg and OUT_BASE.zip
make_dmg_and_zip() {
    local STAGING="$1"
    local VOLNAME="$2"
    local OUT_BASE="$3"

    local DMG="${OUT_BASE}.dmg"
    local ZIP="${OUT_BASE}.zip"

    rm -f "${DMG}" "${ZIP}"

    echo "==> Creating DMG..."
    hdiutil create -volname "${VOLNAME}" \
        -srcfolder "${STAGING}" \
        -ov -format UDZO \
        "${DMG}"
    codesign --force --sign "${SIGN_ID}" --timestamp "${DMG}"
    notarize_dmg "${DMG}"

    echo "==> Creating ZIP..."
    # ZIP from inside staging so paths are relative
    (cd "${STAGING}" && zip -r --symlinks "${OLDPWD}/${ZIP}" .)
    notarize_zip "${ZIP}"

    echo "==> Packages ready:"
    echo "    ${DMG}"
    echo "    ${ZIP}"
}

package_plugin() {
    local NAME="$1"
    local VERSION="${PLUGIN_VERSION[$NAME]}"
    local DESC="${PLUGIN_DESC[$NAME]}"

    local STAGING="${BUILD_DIR}/staging-${NAME}"
    local OUT_BASE="${DIST_DIR}/${NAME}-${VERSION}-Mac"

    sign_plugin "${NAME}"

    echo "==> Staging ${NAME}..."
    rm -rf "${STAGING}"
    mkdir -p "${STAGING}"
    stage_plugin "${NAME}" "${STAGING}"
    write_readme "${NAME}" "${VERSION}" "${DESC}" "${STAGING}"
    mkdir -p "${DIST_DIR}"
    make_dmg_and_zip "${STAGING}" "${NAME} ${VERSION}" "${OUT_BASE}"

    rm -rf "${STAGING}"
    echo "==> Done: ${NAME}"
}

package_suite() {
    echo ""
    echo "==> Building suite bundle (CorvidAudio v${SUITE_VERSION})..."

    local STAGING="${BUILD_DIR}/staging-suite"
    local OUT_BASE="${DIST_DIR}/CorvidAudio-${SUITE_VERSION}-Mac"

    rm -rf "${STAGING}"
    mkdir -p "${STAGING}"

    for NAME in "${ALL_PLUGINS[@]}"; do
        stage_plugin "${NAME}" "${STAGING}"
    done

    cat > "${STAGING}/README.txt" << README
Corvid Audio Suite v${SUITE_VERSION}

Includes:
  2-OP      - 2-operator FM synthesizer
  Broken    - Dying battery fuzz
  Dist308   - ProCo Rat-inspired distortion
  Headroom  - Hard clipper with threshold control
  Life      - Analog character and warmth
  Loc-Box   - Shure Level Loc brickwall limiter emulation

Install by copying the plugins to:
  AU:   /Library/Audio/Plug-Ins/Components/
  VST3: /Library/Audio/Plug-Ins/VST3/

Restart your DAW after installation.
README

    mkdir -p "${DIST_DIR}"
    make_dmg_and_zip "${STAGING}" "Corvid Audio Suite ${SUITE_VERSION}" "${OUT_BASE}"

    rm -rf "${STAGING}"
    echo "==> Suite done: ${OUT_BASE}.dmg / .zip"
}

# ------------------------------------------------------------------
# Main
# ------------------------------------------------------------------
[[ $# -lt 1 ]] && usage

configure

if [[ "$1" == "all" ]]; then
    build_all
    for p in "${ALL_PLUGINS[@]}"; do
        package_plugin "$p"
    done
    package_suite
else
    if [[ -z "${PLUGIN_TARGET[$1]+x}" ]]; then
        echo "Error: unknown plugin '$1'"
        usage
    fi
    build_plugin "$1"
    package_plugin "$1"
fi

echo ""
echo "==> All done! Output: ${DIST_DIR}/"
