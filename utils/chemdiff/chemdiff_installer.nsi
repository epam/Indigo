!define WEB_SITE "http://epam.com/opensource/indigo/chemdiff"
!define APP_NAME "ChemDiff"
!define COMP_NAME "EPAM Systems"
!define COPYRIGHT "EPAM Systems © 2010"
!define DESCRIPTION "ChemDiff is an Indigo-based utility for visual comparison of two files containing multiple structures."

SetCompressor /SOLID lzma
 
  !define MULTIUSER_EXECUTIONLEVEL Highest
  !define MULTIUSER_MUI
  !define MULTIUSER_INSTALLMODE_COMMANDLINE
  !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "Software\EPAM Systems\${APP_NAME}"
  !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME ""
  !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "Software\EPAM Systems\${APP_NAME}"
  !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME ""
  !define MULTIUSER_INSTALLMODE_INSTDIR "EPAM Systems\${APP_NAME}"
  !include "MultiUser.nsh"
  !include "MUI2.nsh"
 
;--------------------------------
;General
 
  ;Name and file
  Name "${APP_NAME}"
  OutFile "chemdiff-${VERSION}-installer.exe"
 
;--------------------------------
;Variables
 
  Var StartMenuFolder
 
;--------------------------------
;Interface Settings
 
  !define MUI_ABORTWARNING
 
;--------------------------------
;Language Selection Dialog Settings
 
  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "SHCTX" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\EPAM Systems\${APP_NAME}" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
 
;--------------------------------
;Pages
 
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "LICENSE.GPL"
  !insertmacro MULTIUSER_PAGE_INSTALLMODE
  !insertmacro MUI_PAGE_DIRECTORY
 
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "EPAM Systems\${APP_NAME}"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\EPAM Systems\${APP_NAME}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
 
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
 
  !insertmacro MUI_PAGE_INSTFILES

  
	Function finishpageaction
		CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\launch.bat"
	FunctionEnd
 
	!define MUI_FINISHPAGE_RUN $INSTDIR\launch.bat
	!define MUI_FINISHPAGE_SHOWREADME ""
	!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
	!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"
	!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction
  !insertmacro MUI_PAGE_FINISH
 
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
 
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
 
;--------------------------------
;Installer Sections
 
Section "${APP_NAME}"
 
  SetOutPath "$INSTDIR\lib\"
  File /r "lib\"
  SetOutPath $INSTDIR
  File "chemdiff.jar"
  File "launch.bat"
  SetOutPath "$INSTDIR\tests\"
  File /r "tests\"
 
  ;%NSIS_INSTALL_FILES
 
  ;Store installation folder
  WriteRegStr SHCTX "Software\EPAM Systems\${APP_NAME}" "" $INSTDIR
 
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
 
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
 
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APP_NAME}.lnk" "$INSTDIR\launch.bat" ""
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	!ifdef WEB_SITE
		WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
		CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Website.lnk" "$INSTDIR\${APP_NAME} website.url"
	!endif
 
  !insertmacro MUI_STARTMENU_WRITE_END
 
SectionEnd
 
;--------------------------------
;Installer Functions
 
Function .onInit
 
  !insertmacro MULTIUSER_INIT
 
FunctionEnd
 
;--------------------------------
;This function and example function call can be used to recursively delete empty parent folders of a given folder.
Function un.RMDirUP
	!define RMDirUP "!insertmacro RMDirUPCall"

	!macro RMDirUPCall _PATH
		push '${_PATH}'
		Call un.RMDirUP
	!macroend

	; $0 - current folder
	ClearErrors

	Exch $0
	;DetailPrint "ASDF - $0\.."
	RMDir "$0\.."

	IfErrors Skip
	${RMDirUP} "$0\.."
	Skip:

	Pop $0
FunctionEnd

;--------------------------------
;Uninstaller Section
 
Section "Uninstall"
  RMDir /r $INSTDIR
 
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
 
  Delete "$SMPROGRAMS\$StartMenuFolder\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	!ifdef WEB_SITE
		Delete "$SMPROGRAMS\$StartMenuFolder\Website.lnk"
	!endif
  
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  ${RMDirUP} "$SMPROGRAMS\$StartMenuFolder"
 
  Delete "$DESKTOP\${APP_NAME}.lnk"
 
  DeleteRegKey SHCTX "Software\EPAM Systems\${APP_NAME}"
  DeleteRegKey /ifempty SHCTX "Software\EPAM Systems"
 
SectionEnd
 
;--------------------------------
;Uninstaller Functions
 
Function un.onInit
 
  !insertmacro MULTIUSER_UNINIT
 
FunctionEnd
