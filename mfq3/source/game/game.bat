mkdir vm
cd vm

call ..\compile  ../g_main.c
@if errorlevel 1 goto quit

call ..\compile  ../g_syscalls.c
@if errorlevel 1 goto quit

call ..\compile  ../bg_misc.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_lib.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_pmove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_slidemove.c
@if errorlevel 1 goto quit
call ..\compile  ../q_math.c
@if errorlevel 1 goto quit
call ..\compile  ../q_shared.c
@if errorlevel 1 goto quit

call ..\compile  ../g_active.c
@if errorlevel 1 goto quit
call ..\compile  ../g_arenas.c
@if errorlevel 1 goto quit
call ..\compile  ../g_client.c
@if errorlevel 1 goto quit
call ..\compile  ../g_cmds.c
@if errorlevel 1 goto quit
call ..\compile  ../g_combat.c
@if errorlevel 1 goto quit
call ..\compile  ../g_items.c
@if errorlevel 1 goto quit
call ..\compile  ../g_mem.c
@if errorlevel 1 goto quit
call ..\compile  ../g_misc.c
@if errorlevel 1 goto quit
call ..\compile  ../g_missile.c
@if errorlevel 1 goto quit
call ..\compile  ../g_mover.c
@if errorlevel 1 goto quit
call ..\compile  ../g_session.c
@if errorlevel 1 goto quit
call ..\compile  ../g_spawn.c
@if errorlevel 1 goto quit
call ..\compile  ../g_svcmds.c
@if errorlevel 1 goto quit
call ..\compile  ../g_target.c
@if errorlevel 1 goto quit
call ..\compile  ../g_team.c
@if errorlevel 1 goto quit
call ..\compile  ../g_trigger.c
@if errorlevel 1 goto quit
call ..\compile  ../g_utils.c
@if errorlevel 1 goto quit
call ..\compile  ../g_weapon.c
@if errorlevel 1 goto quit

call ..\compile  ../g_droneutil.c
@if errorlevel 1 goto quit
call ..\compile  ../g_droneground.c
@if errorlevel 1 goto quit
call ..\compile  ../g_droneplane.c
@if errorlevel 1 goto quit
call ..\compile  ../g_waypoint.c
@if errorlevel 1 goto quit
call ..\compile  ../g_mfq3ents.c
@if errorlevel 1 goto quit
call ..\compile  ../g_miscvehicle.c
@if errorlevel 1 goto quit
call ..\compile  ../g_mfq3util.c
@if errorlevel 1 goto quit
call ..\compile  ../g_plane.c
@if errorlevel 1 goto quit
call ..\compile  ../g_helo.c
@if errorlevel 1 goto quit
call ..\compile  ../g_lqm.c
@if errorlevel 1 goto quit
call ..\compile  ../g_boat.c
@if errorlevel 1 goto quit
call ..\compile  ../g_scripts.c
@if errorlevel 1 goto quit
call ..\compile  ../g_groundvehicle.c
@if errorlevel 1 goto quit
call ..\compile  ../mf_client.c
@if errorlevel 1 goto quit
call ..\compile  ../mf_vehiclespawn.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_vehicledata.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_weapondata.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_planemove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_groundmove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_helomove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_lqmmove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_boatmove.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_mfq3util.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_md3util.c
@if errorlevel 1 goto quit
call ..\compile  ../bg_loadouts.c
@if errorlevel 1 goto quit

q3asm -f ../game
:quit
cd ..
