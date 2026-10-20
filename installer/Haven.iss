#ifndef StageDir
  #error StageDir must point to the prepared runtime folder
#endif
#ifndef ReleaseDir
  #define ReleaseDir "."
#endif
[Setup]
AppId={{38E53CEF-DA70-4A66-BEF5-0B27BA7F6D41}
AppName=Haven
AppVersion=0.6.2
AppPublisher=heybuilder
AppPublisherURL=https://github.com/heybuilder/haven-wallpaper
DefaultDirName={localappdata}\Programs\Haven
DefaultGroupName=Haven
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#ReleaseDir}
OutputBaseFilename=Haven-Setup-v0.6.2
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\app\Haven.exe
AppMutex=Local\HavenDesktopApp
CloseApplications=yes
[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\Haven"; Filename: "{app}\app\Haven.exe"; WorkingDir: "{app}\app"
Name: "{group}\Uninstall Haven"; Filename: "{uninstallexe}"
[Run]
Filename: "{app}\app\Haven.exe"; Description: "Open Haven"; Flags: nowait postinstall skipifsilent

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var Command: String;
begin
 if CurUninstallStep = usUninstall then
  if RegQueryStringValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', 'Haven', Command) then
   if Command = '"' + ExpandConstant('{app}\app\Haven.exe') + '"' then
    RegDeleteValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', 'Haven');
end;
