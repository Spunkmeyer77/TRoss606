#!/bin/bash
# Packs a Release build into dist/TRoss606-<version>-Linux-x86_64.tar.gz (with an install script)
set -euo pipefail
cd "$(dirname "$0")/.."

VERSION=$(sed -nE 's/^project\(TRoss606 VERSION ([0-9.]+)\).*/\1/p' CMakeLists.txt)
ART="build/TRoss606_artefacts/Release"
NAME="TRoss606-$VERSION-Linux-$(uname -m)"
STAGE="dist/$NAME"

rm -rf "$STAGE" && mkdir -p "$STAGE"
cp -R "$ART/VST3/TRoss 606.vst3"   "$STAGE/"
cp -R "$ART/LV2/TRoss 606.lv2"     "$STAGE/"
cp    "$ART/Standalone/TRoss 606"  "$STAGE/"

cat > "$STAGE/install.sh" <<'EOS'
#!/bin/bash
# Installs TRoss 606 for the current user (no root needed).
set -e
cd "$(dirname "$0")"
mkdir -p ~/.vst3 ~/.lv2 ~/.local/bin
rm -rf ~/.vst3/"TRoss 606.vst3" ~/.lv2/"TRoss 606.lv2"
cp -R "TRoss 606.vst3" ~/.vst3/
cp -R "TRoss 606.lv2"  ~/.lv2/
cp "TRoss 606" ~/.local/bin/TRoss606 && chmod +x ~/.local/bin/TRoss606
echo "Installed: ~/.vst3/TRoss 606.vst3, ~/.lv2/TRoss 606.lv2, ~/.local/bin/TRoss606"
echo "Rescan plug-ins in your DAW (Bitwig, Reaper, Ardour...)."
EOS

cat > "$STAGE/uninstall.sh" <<'EOS'
#!/bin/bash
rm -rf ~/.vst3/"TRoss 606.vst3" ~/.lv2/"TRoss 606.lv2" ~/.local/bin/TRoss606
echo "TRoss 606 removed."
EOS
chmod +x "$STAGE/install.sh" "$STAGE/uninstall.sh"

cat > "$STAGE/README.txt" <<EOS
TRoss 606 $VERSION - 606-style drum machine with step sequencer - Ticco Ross 2026

Install:    ./install.sh      (VST3 -> ~/.vst3, LV2 -> ~/.lv2, app -> ~/.local/bin/TRoss606)
Uninstall:  ./uninstall.sh
Needs: ALSA or JACK, X11, FreeType (standard on desktop distributions).
EOS

tar -C dist -czf "dist/$NAME.tar.gz" "$NAME"
rm -rf "$STAGE"
echo "==> Done: dist/$NAME.tar.gz"
