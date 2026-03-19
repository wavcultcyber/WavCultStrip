; WavCult Strip — Inno Setup Installer Script
; Compile with Inno Setup 6+ (https://jrsoftware.org/isinfo.php)
;
; Prerequisites: Build the plugin with CMake first:
;   cmake -B build -G "Visual Studio 17 2022" -A x64
;   cmake --build build --config Release

#define MyAppName "WavCult Strip"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "WavCult"
#define MyAppURL "https://github.com/wavcultcyber"

[Setup]
AppId={{B2C3D4E5-F6A7-8901-BCDE-F12345678901}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={commonpf}\WavCult\Strip
DefaultGroupName={#MyAppPublisher}
AllowNoIcons=yes
OutputDir=..\..\installer\output
OutputBaseFilename=WavCultStrip-{#MyAppVersion}-Windows
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
DisableDirPage=yes
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\uninstall.ico
LicenseFile=
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation"
Name: "vst3only"; Description: "VST3 only"

[Components]
Name: "vst3"; Description: "VST3 Plugin"; Types: full vst3only; Flags: fixed
Name: "standalone"; Description: "Standalone Application"; Types: full

[Files]
; VST3 Plugin — install to system VST3 folder
Source: "..\..\build\WavCultStrip_artefacts\Release\VST3\WavCult Strip.vst3\*"; \
    DestDir: "{commoncf}\VST3\WavCult Strip.vst3"; \
    Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs

; Standalone app (optional)
Source: "..\..\build\WavCultStrip_artefacts\Release\Standalone\WavCult Strip.exe"; \
    DestDir: "{app}"; Components: standalone; Flags: ignoreversion; \
    Check: FileExists(ExpandConstant('{#SetupSetting("SourceDir")}\..\..\build\WavCultStrip_artefacts\Release\Standalone\WavCult Strip.exe'))

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\WavCult Strip.exe"; Components: standalone
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "{cmd}"; Parameters: "/c echo Plugin installed to {commoncf}\VST3\"; \
    Description: "VST3 installed"; Flags: runhidden

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\WavCult Strip.vst3"

[Messages]
WelcomeLabel2=This will install {#MyAppName} {#MyAppVersion} on your computer.%n%nThe VST3 plugin will be installed to:%n  C:\Program Files\Common Files\VST3\%n%nPlease close any DAWs before continuing.
FinishedLabel=Setup has finished installing {#MyAppName}.%n%nOpen your DAW and rescan for plugins to find "{#MyAppName}".
