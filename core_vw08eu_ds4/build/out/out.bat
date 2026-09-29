@echo off

rem --- S'assure que Cygwin n'affiche pas l'avertissement de chemin d'acc�s DOS
set CYGWIN=nodosfilewarning

rem --- Ajoute le chemin de Bash en d�but de path
SET BASH_EXE="c:\cygwin\bin\bash.exe"

SET REL_PATH=..\
SET ABS_PATH=%~dp1
SET FILENAME=%1

SET PLATFORM=%2

if "%PLATFORM%"=="" (
  for /f "tokens=1,2 delims=," %%A in ("%FILENAME%") do (
    SET FILENAME=%%A
    SET PLATFORM=%%B
  )
)

if "%3" == "" (  
  SET DEVICE=0
)else (
  SET DEVICE=%3
)


cd %REL_PATH%

SET OUT_PATH=%CD%\out

IF not exist %OUT_PATH%\web (mkdir %OUT_PATH%\web)
IF not exist %OUT_PATH%\dbg (mkdir %OUT_PATH%\dbg)
IF not exist %OUT_PATH%\403 (mkdir %OUT_PATH%\403)
IF not exist %OUT_PATH%\933 (mkdir %OUT_PATH%\933)


if "%2" == "DBALL" ( 
  call :SUB_402
) else if "%2" == "DBALL3" ( 
  call :SUB_403
) else if "%2" == "DBALL3_DBG" ( 
  call :SUB_403_DBG  
) else if "%2" == "CM800" ( 
  call :SUB_800  
) else if "%2" == "CM800_DBG" ( 
  call :SUB_800_DBG  
) else if "%2" == "NGMM" ( 
  call :SUB_933  
) else if "%2" == "NGMM_DBG" ( 
  call :SUB_933_DBG  
) else if "%2" == "HY_CANADA" (
  call :SUB_412
)  else (
  echo "!!! ERROR !!! Please Set Target(DBALL,CM800...)"
  echo "1- in option for target clic on user tab"
  echo "2- under run user program after build/rebuid add target after #H like:"
  echo "For DBALL  ->..\out\out.bat #H,DBALL"
  echo "For DBALL3 ->..\out\out.bat #H,DBALL3"
  echo "For CM800  ->..\out\out.bat #H,CM800"  
  echo "For NGMM   ->..\out\out.bat #H,NGMM"
  goto end
)

rem --- copy config prog
if exist %CD%\common\com.inc.config.fw\config_prog.h (
  copy %CD%\common\com.inc.config.fw\config_prog.h          %OUT_PATH%\web\config_prog.h
)
goto end


::***************************************************************
:SUB_402
    :ECHO In SUB_402
    
    rem %BASH_EXE% -l hex2enc_401 %FILENAME% 
    %BASH_EXE% -l hex2enc_402 %FILENAME% 
    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\402\ & call :SUB_CHECK_ERROR
    1>nul copy %ABS_PATH%\*.enc                                       %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\402\ & call :SUB_CHECK_ERROR
    
    1>nul copy %CD%\dball\inc.config.fw\config_rsr_feature.h          %OUT_PATH%\web\config_rsr_feature.h
    1>nul copy %CD%\dball\inc.config.d2d\config_d2d.h                 %OUT_PATH%\web\dball_config_d2d.h
    
    exit /b
::***************************************************************
:SUB_403
    :ECHO In SUB_403
       
    %BASH_EXE% -l hex2enc_403 %FILENAME% 

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\403\ & call :SUB_CHECK_ERROR
    if exist %ABS_PATH%\*.enc (
      1>nul copy %ABS_PATH%\*.enc                                     %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
      1>nul move %ABS_PATH%\*.enc                                     %OUT_PATH%\403\ & call :SUB_CHECK_ERROR
    ) else (
      if exist *.enc (
        1>nul copy *.enc                                               %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
        1>nul move *.enc                                               %OUT_PATH%\403\ & call :SUB_CHECK_ERROR
      )
    )

    if exist %CD%\dball\inc.config.fw\config_rsr_feature.h (
      1>nul copy %CD%\dball\inc.config.fw\config_rsr_feature.h      %OUT_PATH%\web\config_rsr_feature.h
    )
    if exist %CD%\dball\inc.config.d2d\config_d2d.h (
      1>nul copy %CD%\dball\inc.config.d2d\config_d2d.h             %OUT_PATH%\web\dball_config_d2d.h
    )
    rem --- DB3 split: config_prog.h lives in CORE (web-config file) ---
    if exist %CD%\core_db3\external\can.sdk.db3\INC\dball\inc.config.fw\config_prog.h (
      1>nul copy %CD%\core_db3\external\can.sdk.db3\INC\dball\inc.config.fw\config_prog.h   %OUT_PATH%\web\config_prog.h
    )

    exit /b
::***************************************************************
:SUB_403_DBG
    :ECHO In SUB_403_DBG
       
    %BASH_EXE% -l hex2enc_403 %FILENAME% 

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\403\ & call :SUB_CHECK_ERROR   
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\dbg\ & call :SUB_CHECK_ERROR   
    
    exit /b    
::***************************************************************
:SUB_933
    :ECHO In SUB_933

    %BASH_EXE% -l hex2enc_933 %FILENAME%

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\933\ & call :SUB_CHECK_ERROR
    if exist %ABS_PATH%\*.enc (
      1>nul copy %ABS_PATH%\*.enc                                     %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
      1>nul move %ABS_PATH%\*.enc                                     %OUT_PATH%\933\ & call :SUB_CHECK_ERROR
    ) else (
      if exist *.enc (
        1>nul copy *.enc                                               %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
        1>nul move *.enc                                               %OUT_PATH%\933\ & call :SUB_CHECK_ERROR
      )
    )

    rem --- DS4 split: fisierele de configurat pe web stau in VEHICLE_SPECIFIC,
    rem     comun celor doua platforme. Caile de monolit (cmall\...) nu exista aici.
    if exist %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config_fw\config_ngmm_feature.h (
      1>nul copy %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config_fw\config_ngmm_feature.h  %OUT_PATH%\web\config_ngmm_feature.h
      1>nul copy %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config_fw\config_ngmm_feature.h  %OUT_PATH%\web\config_933_feature.h
    )
    if exist %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config.d2d\config_d2d.h (
      1>nul copy %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config.d2d\config_d2d.h          %OUT_PATH%\web\ngmm_config_d2d.h
      1>nul copy %CD%\VEHICLE_SPECIFIC\lib.config.ds4\inc.config.d2d\config_d2d.h          %OUT_PATH%\web\933_config_d2d.h
    )

    exit /b
::***************************************************************
:SUB_933_DBG
    :ECHO In SUB_933_DBG

    %BASH_EXE% -l hex2enc_933 %FILENAME% 

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\933\ & call :SUB_CHECK_ERROR
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\dbg\ & call :SUB_CHECK_ERROR

    exit /b 
::***************************************************************
:SUB_800
    :ECHO In SUB_800

    IF exist %CD%\cmall\ (  
      SET FOLDER1=cmall
    )else ( SET FOLDER1=cm800
    SET FOLDER2=\common)
    
    %BASH_EXE% -l hex2enc_800 %FILENAME% 

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\800\ & call :SUB_CHECK_ERROR
    1>nul copy %ABS_PATH%\*.enc                                       %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\800\ & call :SUB_CHECK_ERROR
    
    1>nul copy %CD%\%FOLDER1%\inc.config_fw\config_dei_feature.h                %OUT_PATH%\web\config_dei_feature.h
    1>nul copy %CD%\%FOLDER1%\core%FOLDER2%\inc.config.d2d\config_d2d.h     	%OUT_PATH%\web\cm800_config_d2d.h


    exit /b
::***************************************************************
:SUB_800_DBG
    :ECHO In SUB_800_DBG

    IF exist %CD%\cmall\ (  
      SET FOLDER1=cmall
    )else ( SET FOLDER1=cm800
    SET FOLDER2=\common)
    
    %BASH_EXE% -l hex2enc_800 %FILENAME% 

    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\800\ & call :SUB_CHECK_ERROR   
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\dbg\ & call :SUB_CHECK_ERROR     

    exit /b    
::***************************************************************
:SUB_412
    :ECHO In SUB_412

    %BASH_EXE% -l hex2enc_412 %FILENAME%
    
    1>nul copy %ABS_PATH%\*.hex                                       %OUT_PATH%\412\ & call :SUB_CHECK_ERROR
    1>nul copy %ABS_PATH%\*.enc                                       %OUT_PATH%\web\ & call :SUB_CHECK_ERROR
    1>nul move %ABS_PATH%\*.enc                                       %OUT_PATH%\412\ & call :SUB_CHECK_ERROR
    
    1>nul copy %CD%\dball\inc.config.fw\config_rsr_feature.h          %OUT_PATH%\web\config_rsr_feature.h
    1>nul copy %CD%\dball\inc.config.d2d\config_d2d.h                 %OUT_PATH%\web\dball_config_d2d.h
    
    exit /b
::***************************************************************
:SUB_CHECK_ERROR
    if ERRORLEVEL 1 ( ECHO ERROR: FILE COPY FAILED )
::***************************************************************


:End
