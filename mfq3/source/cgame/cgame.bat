mkdir vm
cd vm

call ..\compile ../../game/bg_misc.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_pmove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_slidemove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_lib.c
@if errorlevel 1 goto quit
call ..\compile ../../game/q_math.c
@if errorlevel 1 goto quit
call ..\compile ../../game/q_shared.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_vehicledata.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_weapondata.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_planemove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_groundmove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_helomove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_lqmmove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_boatmove.c
@if errorlevel 1 goto quit
call ..\compile ../../game/bg_mfq3util.c
@if errorlevel 1 goto quit
call ..\compile ../../ui/ui_shared.c
@if errorlevel 1 goto quit

call ..\compile ../cg_consolecmds.c
@if errorlevel 1 goto quit
call ..\compile ../cg_draw.c
@if errorlevel 1 goto quit
call ..\compile ../cg_drawtools.c
@if errorlevel 1 goto quit
call ..\compile ../cg_effects.c
@if errorlevel 1 goto quit
call ..\compile ../cg_ents.c
@if errorlevel 1 goto quit
call ..\compile ../cg_event.c
@if errorlevel 1 goto quit
call ..\compile ../cg_info.c
@if errorlevel 1 goto quit
call ..\compile ../cg_localents.c
@if errorlevel 1 goto quit
call ..\compile ../cg_main.c
@if errorlevel 1 goto quit
call ..\compile ../cg_marks.c
@if errorlevel 1 goto quit
call ..\compile ../cg_players.c
@if errorlevel 1 goto quit
call ..\compile ../cg_playerstate.c
@if errorlevel 1 goto quit
call ..\compile ../cg_predict.c
@if errorlevel 1 goto quit
call ..\compile ../cg_scoreboard.c
@if errorlevel 1 goto quit
call ..\compile ../cg_servercmds.c
@if errorlevel 1 goto quit
call ..\compile ../cg_snapshot.c
@if errorlevel 1 goto quit
call ..\compile ../cg_view.c
@if errorlevel 1 goto quit
call ..\compile ../cg_weapons.c
@if errorlevel 1 goto quit
call ..\compile ../cg_testmodel.c
@if errorlevel 1 goto quit

call ..\compile ../cg_drawnewhud.c
@if errorlevel 1 goto quit
call ..\compile ../cg_drawoldhud.c
@if errorlevel 1 goto quit
call ..\compile ../cg_vehicle.c
@if errorlevel 1 goto quit
call ..\compile ../cg_miscvehicle.c
@if errorlevel 1 goto quit
call ..\compile ../cg_plane.c
@if errorlevel 1 goto quit
call ..\compile ../cg_helo.c
@if errorlevel 1 goto quit
call ..\compile ../cg_boat.c
@if errorlevel 1 goto quit
call ..\compile ../cg_lqm.c
@if errorlevel 1 goto quit
call ..\compile ../cg_groundvehicle.c
@if errorlevel 1 goto quit

q3asm -f ../cgame
:quit
cd ..
