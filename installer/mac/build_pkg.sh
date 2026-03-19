#!/bin/bash
# WavCult Strip — macOS .pkg Installer Builder
# Run from the project root after building with CMake:
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --config Release
#   ./installer/mac/build_pkg.sh

set -euo pipefail

PLUGIN_NAME="WavCult Strip"
BUNDLE_ID="com.wavcult.strip"
VERSION="1.0.0"
PKG_NAME="WavCultStrip-${VERSION}-macOS.pkg"

BUILD_DIR="build/WavCultStrip_artefacts/Release"
STAGING="/tmp/wavcult-strip-pkg"
OUTPUT_DIR="installer/output"

echo "=== Building WavCult Strip macOS Installer ==="

# Clean staging
rm -rf "$STAGING"
mkdir -p "$STAGING/vst3" "$STAGING/au" "$STAGING/scripts" "$OUTPUT_DIR"

# Copy VST3
if [ -d "${BUILD_DIR}/VST3/${PLUGIN_NAME}.vst3" ]; then
    echo "  Packaging VST3..."
    cp -R "${BUILD_DIR}/VST3/${PLUGIN_NAME}.vst3" "$STAGING/vst3/"
else
    echo "ERROR: VST3 not found at ${BUILD_DIR}/VST3/${PLUGIN_NAME}.vst3"
    exit 1
fi

# Copy AU (optional)
if [ -d "${BUILD_DIR}/AU/${PLUGIN_NAME}.component" ]; then
    echo "  Packaging AU..."
    cp -R "${BUILD_DIR}/AU/${PLUGIN_NAME}.component" "$STAGING/au/"
fi

# Build the VST3 component package
echo "  Building VST3 package..."
pkgbuild \
    --identifier "${BUNDLE_ID}.vst3" \
    --version "$VERSION" \
    --root "$STAGING/vst3" \
    --install-location "/Library/Audio/Plug-Ins/VST3" \
    "$STAGING/vst3-pkg.pkg"

# Build the AU component package (if exists)
if [ -d "$STAGING/au/${PLUGIN_NAME}.component" ]; then
    echo "  Building AU package..."
    pkgbuild \
        --identifier "${BUNDLE_ID}.au" \
        --version "$VERSION" \
        --root "$STAGING/au" \
        --install-location "/Library/Audio/Plug-Ins/Components" \
        "$STAGING/au-pkg.pkg"
fi

# Create distribution XML for combined installer
cat > "$STAGING/distribution.xml" << EOF
<?xml version="1.0" encoding="utf-8" standalone="no"?>
<installer-gui-script minSpecVersion="2">
    <title>${PLUGIN_NAME}</title>
    <organization>${BUNDLE_ID}</organization>
    <welcome file="welcome.txt"/>
    <options customize="allow" require-scripts="false"/>
    <choices-outline>
        <line choice="vst3"/>
EOF

if [ -f "$STAGING/au-pkg.pkg" ]; then
    cat >> "$STAGING/distribution.xml" << EOF
        <line choice="au"/>
EOF
fi

cat >> "$STAGING/distribution.xml" << EOF
    </choices-outline>
    <choice id="vst3" title="VST3 Plugin" description="Install ${PLUGIN_NAME} VST3 to /Library/Audio/Plug-Ins/VST3">
        <pkg-ref id="${BUNDLE_ID}.vst3"/>
    </choice>
EOF

if [ -f "$STAGING/au-pkg.pkg" ]; then
    cat >> "$STAGING/distribution.xml" << EOF
    <choice id="au" title="Audio Unit Plugin" description="Install ${PLUGIN_NAME} AU to /Library/Audio/Plug-Ins/Components">
        <pkg-ref id="${BUNDLE_ID}.au"/>
    </choice>
EOF
fi

cat >> "$STAGING/distribution.xml" << EOF
    <pkg-ref id="${BUNDLE_ID}.vst3" version="${VERSION}">vst3-pkg.pkg</pkg-ref>
EOF

if [ -f "$STAGING/au-pkg.pkg" ]; then
    cat >> "$STAGING/distribution.xml" << EOF
    <pkg-ref id="${BUNDLE_ID}.au" version="${VERSION}">au-pkg.pkg</pkg-ref>
EOF
fi

cat >> "$STAGING/distribution.xml" << EOF
</installer-gui-script>
EOF

# Create welcome text
cat > "$STAGING/welcome.txt" << EOF
Welcome to the ${PLUGIN_NAME} installer.

This will install:
  - VST3 plugin to /Library/Audio/Plug-Ins/VST3/
  - Audio Unit plugin to /Library/Audio/Plug-Ins/Components/

Version: ${VERSION}
By WavCult
EOF

# Build final product
echo "  Building combined installer..."
productbuild \
    --distribution "$STAGING/distribution.xml" \
    --resources "$STAGING" \
    --package-path "$STAGING" \
    "$OUTPUT_DIR/$PKG_NAME"

echo "=== Done! Installer: $OUTPUT_DIR/$PKG_NAME ==="
