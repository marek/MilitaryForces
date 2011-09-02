# Microsoft Developer Studio Project File - Name="NewAirQ2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NewAirQ2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NewAirQ2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NewAirQ2.mak" CFG="NewAirQ2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NewAirQ2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "NewAirQ2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NewAirQ2 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NEWAIRQ2_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NEWAIRQ2_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"Release/gamex86.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy c:\spiele\quake2\newairq2\newairq2\release\gamex86.dll c:\spiele\quake2\newairq2
# End Special Build Tool

!ELSEIF  "$(CFG)" == "NewAirQ2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NEWAIRQ2_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NEWAIRQ2_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "NewAirQ2 - Win32 Release"
# Name "NewAirQ2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\aq2_bots.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_cam.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_cmds.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_data.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_ents.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_helo.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_lqm.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_plane.c
# End Source File
# Begin Source File

SOURCE=.\source\aq2_veh.c
# End Source File
# Begin Source File

SOURCE=.\source\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\source\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\source\g_func.c
# End Source File
# Begin Source File

SOURCE=.\source\g_items.c
# End Source File
# Begin Source File

SOURCE=.\source\g_main.c
# End Source File
# Begin Source File

SOURCE=.\source\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\source\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\source\g_save.c
# End Source File
# Begin Source File

SOURCE=.\source\g_spawn.c
# End Source File
# Begin Source File

SOURCE=.\source\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=.\source\g_target.c
# End Source File
# Begin Source File

SOURCE=.\source\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\source\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\source\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\source\game.def
# End Source File
# Begin Source File

SOURCE=.\source\p_client.c
# End Source File
# Begin Source File

SOURCE=.\source\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\source\p_view.c
# End Source File
# Begin Source File

SOURCE=.\source\p_weapon.c
# End Source File
# Begin Source File

SOURCE=.\source\q_shared.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\g_local.h
# End Source File
# Begin Source File

SOURCE=.\source\game.h
# End Source File
# Begin Source File

SOURCE=.\source\q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\source\game.dsp
# End Source File
# Begin Source File

SOURCE=.\source\game.plg
# End Source File
# Begin Source File

SOURCE=.\source\Makefile
# End Source File
# End Target
# End Project
