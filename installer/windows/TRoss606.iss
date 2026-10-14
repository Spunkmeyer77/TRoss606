; Inno Setup script for the Windows installer.
; Build:  ISCC /DAppVersion=0.1.0 installer\windows\TRoss606.iss
; (paths are relative to this file; expects a Release build in ..\..\build)

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif

#define Artefacts "..\..\build\TRoss606_artefacts\Release"

[Setup]
AppId={{A3C1F5E2-6D04-4B7A-9E3F-606A2C7D1B58}
AppName=TRoss 606
AppVersion={#AppVersion}
AppVerName=TRoss 606 {#AppVersion}
AppPublisher=Ticco Ross
DefaultDirName={autopf}\Ticco Ross\TRoss 606
DefaultGroupName=Ticco Ross
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=..\..\dist
OutputBaseFilename=TRoss606-{#AppVersion}-Windows-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=TRoss 606

[Types]
Name: "full";   Description: "VST3 plug-in and standalone app"
Name: "custom"; Description: "Choose components"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plug-in (Ableton Live, Bitwig, Reaper, Cubase...)"; Types: full custom
Name: "app";  Description: "Standalone app";                                         Types: full custom

[Files]
Source: "{#Artefacts}\VST3\TRoss 606.vst3\*"; DestDir: "{commoncf64}\VST3\TRoss 606.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\TRoss 606.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\TRoss 606"; Filename: "{app}\TRoss 606.exe"; Components: app

[Messages]
FinishedLabel=TRoss 606 is installed.%n%nRescan plug-ins in your DAW (Ableton Live: Settings > Plug-Ins > Rescan).
