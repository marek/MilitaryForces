mkdir vm
cd vm

call ..\compile ../ui_main.c
@if errorlevel 1 goto quit
call ..\compile ../ui_syscalls.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_misc.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_lib.c
@if errorlevel 1 goto quit
call ..\compile ../../game/q_math.c
@if errorlevel 1 goto quit
call ..\compile ../../game/q_shared.c
@if errorlevel 1 goto quit
call ..\compile ../ui_atoms.c
@if errorlevel 1 goto quit
call ..\compile ../ui_util.c
@if errorlevel 1 goto quit
call ..\compile ../ui_shared.c
@if errorlevel 1 goto quit
call ..\compile ../ui_gameinfo.c
@if errorlevel 1 goto quit

call ..\compile ../../game/bg_vehicledata.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_weapondata.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_mfq3util.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_md3util.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_loadouts.c
@if errorlevel 1 goto quit

q3asm -f ../ui
:quit
cd ..
x
