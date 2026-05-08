; Vibeus Installer Script
; For Inno Setup 6.x - Download from: https://jrsoftware.org/isdl.php
; 
; QUICK BUILD: Right-click this file → "Compile" in Inno Setup
; OR: Run build-installer.bat from command line

#define MyAppName "Vibeus"
#define MyAppVersion "0.3.1"
#define MyAppPublisher "Vibeus Project"
#define MyAppURL "https://github.com/fourthdensity/vibeusvisualizer"
#define MyAppExeName "Vibeus.exe"
#define BuildOutputDir "..\build\Release"
#define PresetsDir "..\..\presets-cream-of-the-crop"

[Setup]
; Basic Info
AppId={{E8F3A2C9-4B5D-4E8F-9A3C-2D1E5F6B7C8D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
; Output
OutputDir=.\output
OutputBaseFilename=Vibeus-{#MyAppVersion}-setup
; Compression
Compression=lzma2/ultra64
SolidCompression=yes
; Modern UI
WizardStyle=modern
DisableProgramGroupPage=yes
; Icons
#if FileExists("..\\assets\\favicon.ico")
SetupIconFile="..\\assets\\favicon.ico"
#endif
UninstallDisplayIcon={app}\{#MyAppExeName}
; Architecture (x64compatible = 64-bit Windows, can install in 32-bit mode if needed)
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; Core Application Files
Source: "{#BuildOutputDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\projectM-4.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\projectM-4-playlist.dll"; DestDir: "{app}"; Flags: ignoreversion
; vibeus_config.json is now stored per-user (AppData) at runtime; ship no config to avoid Program Files writes.
; Source: "{#BuildOutputDir}\vibeus_config.json"; DestDir: "{app}"; Flags: onlyifdoesntexist

; VC++ Runtime (bundled to avoid install issues)
Source: "{#BuildOutputDir}\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Assets (Presets & Textures)
; Option 1: Use symlink target directly (recommended)
Source: "{#PresetsDir}\*"; DestDir: "{app}\presets"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
; Option 2: Or follow symlink with external flag
; Source: "{#BuildOutputDir}\presets\*"; DestDir: "{app}\presets"; Flags: ignoreversion recursesubdirs createallsubdirs external skipifsourcedoesntexist
Source: "{#BuildOutputDir}\textures\*"; DestDir: "{app}\textures"; Flags: ignoreversion recursesubdirs createallsubdirs external skipifsourcedoesntexist

; Optional: User-specific files (favorites now stored per-user in AppData)
; Source: "{#BuildOutputDir}\favorites.txt"; DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall

; === OPTIONAL: Bundle VC++ Redistributable (uncomment for production releases) ===
; Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
; Place in: installer\redist\vc_redist.x64.exe
; Source: "redist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall
; [Run]
; Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Runtime..."; Flags: waituntilterminated

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}\presets"
Type: filesandordirs; Name: "{app}\textures"

[Code]
// Check if Visual C++ 2022 Redistributable is installed
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  // Check if VC++ 2022 x64 is installed (version 14.30 or higher)
  Result := not RegQueryStringValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version);
  if not Result then
  begin
    // Also check HKCU in case it's user-installed
    Result := not RegQueryStringValue(HKCU64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version);
  end;
end;

function InitializeSetup: Boolean;
begin
  Result := True;
  
  // Warn user if VC++ Redistributable is missing
  if VCRedistNeedsInstall then
  begin
    if MsgBox('Vibeus requires Visual C++ 2022 Redistributable (x64).' + #13#10 + #13#10 +
              'It was not detected on your system. The application may not run without it.' + #13#10 + #13#10 +
              'Would you like to continue anyway? You can download it from:' + #13#10 +
              'https://aka.ms/vs/17/release/vc_redist.x64.exe', 
              mbConfirmation, MB_YESNO) = IDNO then
    begin
      Result := False;
    end;
  end;
end;
