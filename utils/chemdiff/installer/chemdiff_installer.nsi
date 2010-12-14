Name "Chemdiff"

OutFile "chemdiff.exe"

InstallDir $PROGRAMFILES\Chemdiff

InstallDirRegKey HKLM "GGA\Chemdiff" "Install_Dir"

RequestExecutionLevel admin

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Chemdiff"

  SectionIn RO

  SetOutPath "$INSTDIR\lib\"
  File /r "..\lib\"
  File /r "..\dist\lib\"
  SetOutPath "$INSTDIR\tests\"
  File /r "..\tests\"
  SetOutPath $INSTDIR
  File "..\dist\chemdiff.jar"
  File "..\dist\launch.bat"

  WriteRegStr HKLM SOFTWARE\Chemdiff "Install_Dir" "$INSTDIR"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chemdiff" "DisplayName" "Chemdiff"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chemdiff" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chemdiff" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chemdiff" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Chemdiff"
  CreateShortCut "$SMPROGRAMS\Chemdiff\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Chemdiff\Chemdiff.lnk" "$INSTDIR\launch.bat" "" "$INSTDIR\launch.bat" 0

SectionEnd


Section "Uninstall"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chemdiff"
  DeleteRegKey HKLM SOFTWARE\Chemdiff

  RMDir /r $INSTDIR\lib
  RMDir /r $INSTDIR\tests
  Delete $INSTDIR\chemdiff.jar
  Delete $INSTDIR\launch.bat
  Delete $INSTDIR\uninstall.exe

  Delete "$SMPROGRAMS\Chemdiff\*.*"

  RMDir "$SMPROGRAMS\Chemdiff"
  RMDir "$INSTDIR"

SectionEnd
