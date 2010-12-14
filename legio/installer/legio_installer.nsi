Name "Legio"

OutFile "legio.exe"

InstallDir $PROGRAMFILES\Legio

InstallDirRegKey HKLM "GGA\Legio" "Install_Dir"

RequestExecutionLevel admin

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Legio"

  SectionIn RO

  SetOutPath "$INSTDIR\lib\"
  File /r "..\lib\"
  File /r "..\dist\lib\"
  SetOutPath "$INSTDIR\tests\"
  File /r "..\tests\"
  SetOutPath $INSTDIR
  File "..\dist\legio.jar"
  File "..\dist\launch.bat"

  WriteRegStr HKLM SOFTWARE\Legio "Install_Dir" "$INSTDIR"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Legio" "DisplayName" "Legio"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Legio" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Legio" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Legio" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Legio"
  CreateShortCut "$SMPROGRAMS\Legio\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Legio\Legio.lnk" "$INSTDIR\launch.bat" "" "$INSTDIR\launch.bat" 0

SectionEnd


Section "Uninstall"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Legio"
  DeleteRegKey HKLM SOFTWARE\Legio

  RMDir /r $INSTDIR\lib
  RMDir /r $INSTDIR\tests
  Delete $INSTDIR\legio.jar
  Delete $INSTDIR\launch.bat
  Delete $INSTDIR\uninstall.exe

  Delete "$SMPROGRAMS\Legio\*.*"

  RMDir "$SMPROGRAMS\Legio"
  RMDir "$INSTDIR"

SectionEnd
