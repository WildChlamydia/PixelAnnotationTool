!define APPNAME "PixelAnnotation"
!define DESCRIPTION "PixelAnnotation - Semantic segmenation annotation tool for AI solutions"
# These three must be integers
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0
!define SYSTEMFOLDER $WINDIR\System32

RequestExecutionLevel admin ;

InstallDir "$PROGRAMFILES\${APPNAME}"

# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
LicenseData "MIT.lic"
# This will be in the installer/uninstaller's title bar
Name "${APPNAME}"
Icon "icon.ico"
outFile ".\pixel_annotation.exe"

!include LogicLib.nsh
!include x64.nsh
#!include "MUI2.nsh"
#!include "FileFunc.nsh" # To use GetParameter
!include LogicLib.nsh

# Just three pages - license agreement, install location, and installation
Page license
Page directory
Page instfiles

#!macro VerifyUserIsAdmin

#UserInfo::GetAccountType
#pop $0
#${If} $0 != "admin" ;Require admin rights on NT4+
#        messageBox mb_iconstop "Administrator rights required!"
#        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
#        quit
#${EndIf}
#!macroend

SetCompressor /SOLID lzma

section "install" install_section_id

    # For 64-bit machines
    ${If} ${RunningX64}
        SetRegView 64
    ${EndIf}
        
	# Files for the install directory - to build the installer, these should be in the same directory as the install script (this file)
	setOutPath $INSTDIR
	
	# Add all files in script dir except files after /x
    file /r /x pixel_annotation.exe /x MIT.lic /x setup_script.nsi ""
	ExecWait '"$INSTDIR\vcredist_x86.exe" /q /norestart'
	ExecWait '"$INSTDIR\vcredist_x86_10.exe" /q /norestart'
	delete $INSTDIR\vcredist_x86.exe
	delete $INSTDIR\vcredist_x86_10.exe

	# nsExec::Exec 'SCHTASKS /Create /TN PixelAnnotation /SC ONLOGON /RL HIGHEST /RU пользователи /TR "\"C:\Program Files\ProximityAuth\ProximityAuth.exe"\"'
	# Add any other files for the install directory (license files, app data, etc) here

	# Uninstaller - See function un.onInit and section "uninstall" for configuration
	writeUninstaller "$INSTDIR\uninstall.exe"
	CreateShortcut "$DESKTOP\PixelAnnotation.lnk" "$INSTDIR\PixelAnnotationTool.exe"
	Exec $INSTDIR\PixelAnnotationTool.exe

	# ExecShell open "$INSTDIR"
	quit
sectionEnd

# Uninstaller

function un.onInit
	SetShellVarContext all

	#Verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Permanantly remove ${APPNAME}?" IDOK next
		Abort
	next:
	#!insertmacro VerifyUserIsAdmin
functionEnd

section "uninstall"
        ${If} ${RunningX64}
              SetRegView 64
        ${EndIf}

	# Always delete uninstaller as the last action
	#delete $INSTDIR\uninstall.exe

    # Remove interception
    #ExecWait "$INSTDIR\install-interception.exe \uninstall"
    #delete $INSTDIR\install-interception.exe
    SetShellVarContext all
 	Delete "$DESKTOP\PixelAnnotation.lnk"

    # Try to remove the install directory - this will only happen if it is empty
    RMDir /r $INSTDIR
sectionEnd

function .onInit
	setShellVarContext all
	#!insertmacro VerifyUserIsAdmin

  	SectionSetFlags ${install_section_id} ${SF_SELECTED}

functionEnd