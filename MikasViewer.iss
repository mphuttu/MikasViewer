; =============================================================================
; MikasViewer — Inno Setup 6 installer script
; Build:  compile Release x64 in Visual Studio first, then run this script.
; Output: setup_output\MikasViewerSetup-1.0-x64.exe
; =============================================================================

#define MyAppName      "MikasViewer"
#define MyAppVersion   "1.0"
#define MyAppPublisher "Mika Huttunen"
#define MyAppURL       "https://github.com/mphuttu/MikasViewer"
#define MyAppExeName   "MikasViewer.exe"

[Setup]
; Unique application identifier — do NOT reuse this GUID in other projects
AppId={{B4E9C1A3-7F2D-4B8E-9C0A-3E6D5F1A2B4C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases

; Installation defaults
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
DisableProgramGroupPage=yes

; Output
OutputDir=setup_output
OutputBaseFilename=MikasViewerSetup-{#MyAppVersion}-x64
SetupIconFile=res\MikasViewer.ico

; Compression
Compression=lzma
SolidCompression=yes

; Appearance
WizardStyle=modern
WizardResizable=yes

; 64-bit Windows 10+ only
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
MinVersion=10.0.17763

; Uninstaller
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; \
    Description: "{cm:CreateDesktopIcon}"; \
    GroupDescription: "{cm:AdditionalIcons}"; \
    Flags: unchecked

[Files]
; Main executable — build Release x64 before running this script
Source: "x64\Release\{#MyAppExeName}"; \
    DestDir: "{app}"; \
    Flags: ignoreversion

; CHM help file
Source: "help\MikasViewer.chm"; \
    DestDir: "{app}"; \
    Flags: ignoreversion

[Icons]
; Start Menu
Name: "{group}\{#MyAppName}"; \
    Filename: "{app}\{#MyAppExeName}"; \
    Comment: "View images with MikasViewer"

; Start Menu uninstall entry
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; \
    Filename: "{uninstallexe}"

; Optional desktop icon (user must tick the checkbox during install)
Name: "{autodesktop}\{#MyAppName}"; \
    Filename: "{app}\{#MyAppExeName}"; \
    Comment: "View images with MikasViewer"; \
    Tasks: desktopicon

[Run]
; Offer to launch the application after installation
Filename: "{app}\{#MyAppExeName}"; \
    Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; \
    Flags: nowait postinstall skipifsilent

[Code]
// ---------------------------------------------------------------------------
// Check for Visual C++ 2015-2022 Redistributable (x64).
// MikasViewer requires this runtime — it ships with Visual Studio but must
// be installed separately on end-user machines.
// Download: https://aka.ms/vs/17/release/vc_redist.x64.exe
// ---------------------------------------------------------------------------
function VCRedist2022x64Installed: Boolean;
var
  Installed: Cardinal;
begin
  Result :=
    RegQueryDWordValue(
      HKEY_LOCAL_MACHINE,
      'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
      'Installed',
      Installed) and (Installed = 1);
end;

function InitializeSetup: Boolean;
begin
  Result := True;

  if not VCRedist2022x64Installed then
  begin
    if MsgBox(
        'The Microsoft Visual C++ 2015-2022 Redistributable (x64) was not found '
        + 'on this computer.' + #13#10#13#10
        + 'MikasViewer requires this runtime. Please download and install it:'
        + #13#10
        + '  https://aka.ms/vs/17/release/vc_redist.x64.exe'
        + #13#10#13#10
        + 'Click Yes to continue the installation anyway, or No to cancel.',
        mbConfirmation, MB_YESNO) = IDNO then
    begin
      Result := False;
    end;
  end;
end;
