; SRL (Serial Run Language) v0.3.0 Inno Setup Installer Script
#define MyAppName "SRL Language Toolchain"
#define MyAppVersion "0.3.0"
#define MyAppPublisher "SRL Language Team"
#define MyAppURL "https://github.com/srl-lang/srl-lang"
#define MyAppExeName "srl.exe"

[Setup]
AppId={{D37E6F89-4B52-47C8-912A-1845C9B387F3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\SRL
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=LICENSE
OutputDir=build_installer
OutputBaseFilename=SRL_v0.3.0_Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ChangesEnvironment=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "addtopath"; Description: "Add SRL to system PATH environment variable"; Flags: unchecked

[Files]
Source: "build\Debug\srl.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "srlc.exe"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "std\*"; DestDir: "{app}\std"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "include\*"; DestDir: "{app}\include"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
var
  PathVar: String;
  AppBinDir: String;
begin
  if (CurStep = ssPostInstall) and IsTaskSelected('addtopath') then
  begin
    AppBinDir := ExpandConstant('{app}\bin');
    if RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', PathVar) then
    begin
      if Pos(AppBinDir, PathVar) = 0 then
      begin
        PathVar := PathVar + ';' + AppBinDir;
        RegWriteStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', PathVar);
      end;
    end;
  end;
end;
