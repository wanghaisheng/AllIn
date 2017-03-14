; master_controller.nsi
;
; It will install master_controller.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "master_controller"

; The file to write
OutFile "master_controller.exe"

; The default installation directory
InstallDir $PROGRAMFILES\master_controller

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_master_controller" "Install_Dir"

; Request application privileges for Windows Vista
; RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles
  
UninstPage uninstConfirm
UninstPage instfiles 

;--------------------------------

; The stuff to install
Section "master_controller (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put files there
  ; File "master_controller.nsi"
  File /r "C:\\pj\\bin\\master_output\\lib\\w32d\\*"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_master_controller "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\master_controller" "DisplayName" "NSIS master_controller"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\master_controller" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\master_controller" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\master_controller" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\master_controller"
  CreateShortcut "$SMPROGRAMS\master_controller\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\master_controller\master_controller (MakeNSISW).lnk" "$INSTDIR\master_controller.nsi" "" "$INSTDIR\master_controller.nsi" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\master_controller"
  DeleteRegKey HKLM SOFTWARE\NSIS_master_controller

  ; Remove files and uninstaller
  Delete $INSTDIR\uninstall.exe  
  Delete $INSTDIR\*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\master_controller\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\master_controller"
  RMDir /r "$INSTDIR\imageformats"
  RMDir /r "$INSTDIR\Log"
  RMDir /r "$INSTDIR\pic"
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\Model_Image"
  RMDir "$INSTDIR"
  

SectionEnd
