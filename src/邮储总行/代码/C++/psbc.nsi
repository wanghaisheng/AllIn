; psbc.nsi
;
; It will install psbc.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "psbc"

; The file to write
OutFile "ÓÊ´¢×ÜÐÐÓ¡¿ØÒÇÇý¶¯.exe"

; The default installation directory
InstallDir $PROGRAMFILES\ÈýÌ©Ó¡¿ØÒÇ

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_psbc" "Install_Dir"

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
Section "psbc (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put files there
  ; File "psbc.nsi"
  ; File /r "$SRCDIR\*"
  File "C:\\pj\\bin\\w32r\\ABC.STDZ.Device.STAMP.RZCamera.dll"
  File "C:\\pj\\bin\\w32r\\ABC.STDZ.Device.STAMP.SealLog.dll"
  File "C:\\pj\\bin\\w32r\\ABC.STDZ.Device.STAMP.USBCamAPI.dll"
  File "C:\\pj\\bin\\w32r\\ABC.STDZ.Device.STAMP.USBF60API.dll"
  File "C:\\pj\\bin\\w32r\\cximagecrt.dll"
  File "C:\\pj\\bin\\w32r\\configure.xml"
  File "C:\\pj\\bin\\w32r\\ImageProcess.dll"
  File "C:\\pj\\bin\\w32r\\liblept168.dll"
  File "C:\\pj\\bin\\w32r\\libtesseract302.dll"
  File "C:\\pj\\bin\\w32r\\STDLL.dll"
  File "C:\\pj\\bin\\w32r\\TestPSBC.exe"
  
  File "C:\\pj\\bin\\w32r\\amcap.exe"
  File "C:\\pj\\bin\\w32r\\amcap.ini"
  
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_psbc "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\psbc" "DisplayName" "NSIS psbc"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\psbc" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\psbc" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\psbc" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\psbc"
  CreateShortcut "$SMPROGRAMS\psbc\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\psbc\psbc (MakeNSISW).lnk" "$INSTDIR\psbc.nsi" "" "$INSTDIR\psbc.nsi" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\psbc"
  DeleteRegKey HKLM SOFTWARE\NSIS_psbc

  ; Remove files and uninstaller
  Delete $INSTDIR\uninstall.exe  
  Delete $INSTDIR\*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\psbc\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\psbc"
  RMDir "$INSTDIR"
  

SectionEnd
