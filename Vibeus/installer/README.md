# Vibeus Installer

This folder contains the installer configuration for distributing Vibeus on Windows.

## Quick Start (Active Development)

Since your app is still in development, here's the fastest workflow:

### 1️⃣ One-Time Setup

**Install Inno Setup:**
```bash
# Download and install from:
https://jrsoftware.org/isdl.php
# Choose default installation path
```

### 2️⃣ Build After Each Code Change

```bash
# 1. Build your Release configuration in Visual Studio/CMake
cmake --build build --config Release

# 2. Run the build script (double-click or command line)
cd installer
build-installer.bat
# If ISCC.exe lives elsewhere: set INNO_PATH=C:\Path\to\ISCC.exe first

# Done! Installer is in: installer\output\Vibeus-0.1.0-dev-setup.exe
```

**Using Inno Setup Compiler (CLI):**
If `iscc` is on your PATH you can also run:

```bash
cd installer
iscc vibeus-installer.iss
```

Or point to a custom install by setting `INNO_PATH` before running `build-installer.bat`:

```bash
set INNO_PATH="D:\Tools\Inno Setup 6\ISCC.exe"
build-installer.bat
```

**Total time:** ~30 seconds after your Release build completes

---

## Files Included in Installer

### Core (Required)
- `Vibeus.exe` - Main application
- `SDL2.dll` - Windowing & input
- `projectM-4.dll` - Visualization engine
- `projectM-4-playlist.dll` - Preset management
- `vibeus_config.json` - Default settings

### Assets (~130 MB)
- `presets/` - 9,797 visualization presets
- `textures/` - 183 effect textures

### User Data (Preserved on Update)
- `favorites.txt` - User's favorite presets

---

## Customization

### Change Version Number
Edit `vibeus-installer.iss` line 10:
```iss
#define MyAppVersion "0.2.0-dev"
```

### Skip Presets (Minimal Installer)
Comment out line 49 in `vibeus-installer.iss`:
```iss
; Source: "{#BuildOutputDir}\presets\*"; DestDir: "{app}\presets"; ...
```

### Change Install Location
Edit line 19:
```iss
DefaultDirName={autopf}\MyCustomFolder
```

---

## Visual C++ Redistributable

**Important:** Vibeus requires VC++ 2022 Redistributable x64.

The installer will **check** if it's installed and warn users if missing.

### Option A: Bundle It (Recommended for Distribution)

1. Download: https://aka.ms/vs/17/release/vc_redist.x64.exe
2. Place in: `installer\redist\vc_redist.x64.exe`
3. Uncomment lines 55-59 in `vibeus-installer.iss`

This increases installer size by ~14 MB but ensures it works everywhere.

### Option B: User Installs (Current - Dev Friendly)

Users get a warning message with download link if missing.
Good for active development since rebuilds are faster.

---

## Advanced: Portable ZIP Distribution

If you prefer portable distribution alongside the installer:

```bash
# Create portable package
cd build\Release
powershell Compress-Archive -Path Vibeus.exe,*.dll,vibeus_config.json,presets,textures -DestinationPath ..\..\Vibeus-portable.zip -Force
```

Users just extract and run - no installation needed!

---

## Installer Features

✅ **Updates preserve user settings** - `vibeus_config.json` and `favorites.txt` won't be overwritten  
✅ **Clean uninstall** - Removes all files including presets/textures  
✅ **Desktop shortcut** (optional, user choice)  
✅ **Start Menu entry** (optional, user choice)  
✅ **64-bit only** (x64 architecture)  
✅ **Modern UI** with Inno Setup 6 styling  
✅ **Compressed** (~120 MB → ~35 MB installer with LZMA2/ultra64)

---

## Distribution Checklist

Before sharing your installer publicly:

- [ ] Update version in `vibeus-installer.iss`
- [ ] Test install on clean Windows 10/11 VM
- [ ] Test update over previous version (settings preserved?)
- [ ] Test uninstall (no leftover files?)
- [ ] Consider code signing (optional - improves trust)
- [ ] Bundle VC++ Redistributable for production releases

---

## Troubleshooting

**"ISCC.exe not found"**
→ Install Inno Setup or update path in `build-installer.bat`

**"Build output not found"**
→ Run `cmake --build build --config Release` first

**"Installer too large"**
→ Presets are 115 MB. Consider separate preset pack download.

**"Missing DLL errors when running installed app"**
→ User needs VC++ 2022 Redistributable. Bundle it in installer.

---

## Questions?

The installer script has comments explaining each section.  
For Inno Setup documentation: https://jrsoftware.org/ishelp/
