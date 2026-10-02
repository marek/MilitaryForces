set(VEHICLELIB_SOURCES
    bg_datamanager.cpp bg_md3utils.cpp bg_vehicleextra.cpp bg_vehicleinfo.cpp
    bg_vehicleinfo_boats.cpp bg_vehicleinfo_ground.cpp bg_vehicleinfo_helos.cpp
    bg_vehicleinfo_infantry.cpp bg_vehicleinfo_planes.cpp bg_vehicleinfo_vehicle.cpp
    bg_weaponinfo.cpp bg_weaponinfo_bombs.cpp bg_weaponinfo_guns.cpp
    bg_weaponinfo_missiles.cpp bg_weaponinfo_other.cpp)
list(TRANSFORM VEHICLELIB_SOURCES PREPEND "${SOURCE_DIR}/vehiclelib/")

set(GAME_SOURCES
    bg_boatmove.c bg_groundmove.c bg_helomove.c bg_loadouts.c bg_lqmmove.c bg_md3util.c
    bg_mfq3util.c bg_misc.c bg_planemove.c bg_pmove.c bg_slidemove.c bg_vehicledata.c
    bg_weapondata.c g_active.c g_arenas.c g_boat.cpp g_client.c g_client.cpp g_cmds.c
    g_combat.c g_entity.cpp g_groundinstallation.c g_groundvehicle.cpp g_helo.cpp g_items.c
    g_level.cpp g_lqm.cpp g_main.c g_mem.c g_mfq3ents.c g_mfq3util.c g_misc.c
    g_miscvehicle.c g_missile.c g_missions.c g_mover.c g_plane.cpp g_scripts.c g_session.c
    g_spawn.c g_svcmds.c g_target.c g_team.c g_trigger.c g_utils.c g_vehicle.cpp
    g_waypoint.c g_weapon.c g_world.cpp game.cpp mf_client.c mf_vehiclespawn.c)
list(TRANSFORM GAME_SOURCES PREPEND "${SOURCE_DIR}/game/")

set(CGAME_SOURCES
    cg.cpp cg_boat.c cg_consolecmds.c cg_draw.c cg_drawnewhud.c cg_drawoldhud.c
    cg_drawtools.c cg_effects.c cg_ents.c cg_event.c cg_groundvehicle.c cg_helo.c cg_info.c
    cg_localents.c cg_lqm.c cg_main.c cg_marks.c cg_miscvehicle.c cg_missioneditor.c
    cg_particles.c cg_plane.c cg_players.c cg_playerstate.c cg_predict.c cg_scoreboard.c
    cg_servercmds.c cg_snapshot.c cg_testmodel.c cg_util.c cg_vehicle.c cg_vehicledraw.c
    cg_view.c cg_weapons.c ui_displaycontext_cg.cpp ui_utils_cg.cpp)
list(TRANSFORM CGAME_SOURCES PREPEND "${SOURCE_DIR}/cgame/")

set(UI_SOURCES
    ui.cpp ui_atoms.c ui_commanddef.cpp ui_controls.cpp ui_displaycontext.cpp
    ui_displaycontext_ui.cpp ui_gameinfo.c ui_hashutils.cpp ui_itemcapture.cpp
    ui_keywordhash.cpp ui_main.c ui_memtools.cpp ui_precomp.cpp ui_util.c ui_utils.cpp
    ui_utils_ui.cpp)
list(TRANSFORM UI_SOURCES PREPEND "${SOURCE_DIR}/ui/")

set(RENDERER_SOURCES
    tr_animation.c tr_backend.c tr_bsp.c tr_cmds.c tr_curve.c tr_flares.c tr_font.c
    tr_image.c tr_image_jpg.c tr_init.c tr_light.c tr_main.c tr_marks.c tr_mesh.c tr_model.c tr_noise.c
    tr_scene.c tr_shade.c tr_shade_calc.c tr_shader.c tr_shadows.c tr_sky.c tr_surface.c
    tr_world.c)
list(TRANSFORM RENDERER_SOURCES PREPEND "${SOURCE_DIR}/renderer/")

set(QCOMMON_SOURCES
    qcommon/cm_load.c qcommon/cm_patch.c qcommon/cm_polylib.c qcommon/cm_test.c
    qcommon/cm_trace.c qcommon/cmd.c qcommon/common.c qcommon/cvar.c qcommon/files.c
    qcommon/huffman.c qcommon/md4.c qcommon/msg.c qcommon/net_chan.c qcommon/unzip.c
    game/q_math.c game/q_shared.c)
list(TRANSFORM QCOMMON_SOURCES PREPEND "${SOURCE_DIR}/")

set(SERVER_SOURCES
    sv_bot.c sv_ccmds.c sv_client.c sv_client.cpp sv_entity.cpp sv_game.c sv_init.c
    sv_main.c sv_net_chan.c sv_server.cpp sv_snapshot.c sv_world.c)
list(TRANSFORM SERVER_SOURCES PREPEND "${SOURCE_DIR}/server/")

set(CLIENT_SOURCES
    cl_cgame.c cl_cin.c cl_console.c cl_input.c cl_keys.c cl_main.c cl_net_chan.c
    cl_parse.c cl_scrn.c cl_ui.c snd_adpcm.c snd_dma.c snd_mem.c snd_mix.c snd_wavelet.c)
list(TRANSFORM CLIENT_SOURCES PREPEND "${SOURCE_DIR}/client/")

set(SYSTEM_SOURCES
    ${SOURCE_DIR}/sys/sys_main.c
    ${SOURCE_DIR}/sys/con_log.c
    ${SOURCE_DIR}/qcommon/net_ip.c)

set(SDL_SOURCES
    ${SOURCE_DIR}/sdl/sdl_input.c
    ${SOURCE_DIR}/sdl/sdl_snd.c)

set(SDL_RENDERER_SOURCES
    ${SOURCE_DIR}/sdl/sdl_glimp.c
    ${SOURCE_DIR}/sdl/sdl_gamma.c)
