# Microsoft Developer Studio Project File - Name="ht_meterProduct" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ht_meterProduct - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ht_meterProduct.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ht_meterProduct.mak" CFG="ht_meterProduct - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ht_meterProduct - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ht_meterProduct - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D WINVER=0x0501 /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 xlibs.lib /nologo /subsystem:windows /machine:I386 /out:"ht_meterProduct.exe"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "treeprop" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D WINVER=0x0501 /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 xlibsd.lib /nologo /subsystem:windows /debug /machine:I386 /out:"ht_meterProductd.exe" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "ht_meterProduct - Win32 Release"
# Name "ht_meterProduct - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "trans"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\comm.cpp
# ADD CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ht_cmn.cpp
# ADD CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ht_pms.cpp
# ADD CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ht_show.cpp
# ADD CPP /YX
# End Source File
# Begin Source File

SOURCE=.\ht_trans.cpp
# ADD CPP /YX
# End Source File
# End Group
# Begin Group "TreeProp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\treeprop\hookwnd.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\PropPageFrame.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\PropPageFrameBordered.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\PropPageFrameDefault.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\PropPageFrameEx.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\PropPageFrameOffice2003.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableDialog.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableGrip.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableLayout.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableMinMax.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizablePage.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableSheet.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ResizableState.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\ThemeLibEx.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheet.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetBase.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetBordered.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetEx.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetOffice2003.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetResizableLibHook.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetSplitter.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\treeprop\TreePropSheetTreeCtrl.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ht_meterProduct.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MeterProduct.cpp

!IF  "$(CFG)" == "ht_meterProduct - Win32 Release"

!ELSEIF  "$(CFG)" == "ht_meterProduct - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\xwork_trans.cpp
# ADD CPP /YX
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ht_32_gvars.h
# End Source File
# Begin Source File

SOURCE=.\ht_cmn.h
# End Source File
# Begin Source File

SOURCE=.\ht_meterProduct.h
# End Source File
# Begin Source File

SOURCE=.\ht_meterProductDlg.h
# End Source File
# Begin Source File

SOURCE=.\ht_pms.h
# End Source File
# Begin Source File

SOURCE=.\ht_trans.h
# End Source File
# Begin Source File

SOURCE=.\MeterProduct.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StaticEx.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\xwork_trans.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\date-time.ico"
# End Source File
# Begin Source File

SOURCE=.\res\ht_meterProduct.ico
# End Source File
# Begin Source File

SOURCE=.\ht_meterProduct.rc
# End Source File
# Begin Source File

SOURCE=.\res\ht_meterProduct.rc2
# End Source File
# Begin Source File

SOURCE=.\res\incoming.ico
# End Source File
# Begin Source File

SOURCE=.\res\outgoing.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
