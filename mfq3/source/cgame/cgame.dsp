# Microsoft Developer Studio Project File - Name="cgame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CGAME - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cgame.mak" CFG="CGAME - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cgame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cgame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cgame - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /machine:I386 /out:"../Release/cgamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "cgame - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MENU_SCOREBOARD" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /base:"0x30000000" /subsystem:windows /dll /map /debug /machine:I386 /out:"..\..\cgamex86.dll"
# SUBTRACT LINK32 /profile /nodefaultlib

!ENDIF 

# Begin Target

# Name "cgame - Win32 Release"
# Name "cgame - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=..\game\bg_boatmove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_groundmove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_helomove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_lib.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\game\bg_lqmmove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_mfq3util.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_misc.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_planemove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_pmove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_slidemove.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_vehicledata.c
# End Source File
# Begin Source File

SOURCE=..\game\bg_weapondata.c
# End Source File
# Begin Source File

SOURCE=.\cg_boat.c
# End Source File
# Begin Source File

SOURCE=.\cg_consolecmds.c
# End Source File
# Begin Source File

SOURCE=.\cg_draw.c
# End Source File
# Begin Source File

SOURCE=.\cg_drawnewhud.c
# End Source File
# Begin Source File

SOURCE=.\cg_drawoldhud.c
# End Source File
# Begin Source File

SOURCE=.\cg_drawtools.c
# End Source File
# Begin Source File

SOURCE=.\cg_effects.c
# End Source File
# Begin Source File

SOURCE=.\cg_ents.c
# End Source File
# Begin Source File

SOURCE=.\cg_event.c
# End Source File
# Begin Source File

SOURCE=.\cg_groundvehicle.c
# End Source File
# Begin Source File

SOURCE=.\cg_helo.c
# End Source File
# Begin Source File

SOURCE=.\cg_info.c
# End Source File
# Begin Source File

SOURCE=.\cg_localents.c
# End Source File
# Begin Source File

SOURCE=.\cg_lqm.c
# End Source File
# Begin Source File

SOURCE=.\cg_main.c
# End Source File
# Begin Source File

SOURCE=.\cg_marks.c
# End Source File
# Begin Source File

SOURCE=.\cg_miscvehicle.c
# End Source File
# Begin Source File

SOURCE=.\cg_plane.c
# End Source File
# Begin Source File

SOURCE=.\cg_players.c
# End Source File
# Begin Source File

SOURCE=.\cg_playerstate.c
# End Source File
# Begin Source File

SOURCE=.\cg_predict.c
# End Source File
# Begin Source File

SOURCE=.\cg_scoreboard.c
# End Source File
# Begin Source File

SOURCE=.\cg_servercmds.c
# End Source File
# Begin Source File

SOURCE=.\cg_snapshot.c
# End Source File
# Begin Source File

SOURCE=.\cg_syscalls.c
# End Source File
# Begin Source File

SOURCE=.\cg_testmodel.c
# End Source File
# Begin Source File

SOURCE=.\cg_util.c
# End Source File
# Begin Source File

SOURCE=.\cg_vehicle.c
# End Source File
# Begin Source File

SOURCE=.\cg_view.c
# End Source File
# Begin Source File

SOURCE=.\cg_weapons.c
# End Source File
# Begin Source File

SOURCE=..\game\q_math.c
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.c
# End Source File
# Begin Source File

SOURCE=..\ui\ui_shared.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\game\bg_public.h
# End Source File
# Begin Source File

SOURCE=.\cg_local.h
# End Source File
# Begin Source File

SOURCE=.\cg_public.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\game\surfaceflags.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cgame.def
# End Source File
# End Target
# End Project
