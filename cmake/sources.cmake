set(VEHICLELIB_SOURCES
    bg_datamanager.cpp bg_md3utils.cpp bg_vehicleextra.cpp bg_vehicleinfo.cpp
    bg_vehicleinfo_boats.cpp bg_vehicleinfo_ground.cpp bg_vehicleinfo_helos.cpp
    bg_vehicleinfo_infantry.cpp bg_vehicleinfo_planes.cpp bg_vehicleinfo_vehicle.cpp
    bg_weaponinfo.cpp bg_weaponinfo_bombs.cpp bg_weaponinfo_guns.cpp
    bg_weaponinfo_missiles.cpp bg_weaponinfo_other.cpp)
list(TRANSFORM VEHICLELIB_SOURCES PREPEND "${SOURCE_DIR}/vehiclelib/")

set(GAME_SOURCES
    bg_boatmove.cpp bg_groundmove.cpp bg_helomove.cpp bg_loadouts.cpp bg_lqmmove.cpp bg_md3util.cpp
    bg_mfq3util.cpp bg_misc.cpp bg_planemove.cpp bg_pmove.cpp bg_slidemove.cpp bg_vehicledata.cpp
    bg_weapondata.cpp g_active.cpp g_arenas.cpp g_boat.cpp g_client.cpp g_cmds.cpp
    g_combat.cpp g_entity.cpp g_groundinstallation.cpp g_groundvehicle.cpp g_helo.cpp g_items.cpp
    g_level.cpp g_lqm.cpp g_main.cpp g_mem.cpp g_mfq3ents.cpp g_mfq3util.cpp g_misc.cpp
    g_miscvehicle.cpp g_missile.cpp g_missions.cpp g_mover.cpp g_plane.cpp g_scripts.cpp g_session.cpp
    g_spawn.cpp g_svcmds.cpp g_target.cpp g_team.cpp g_trigger.cpp g_utils.cpp g_vehicle.cpp
    g_waypoint.cpp g_weapon.cpp g_world.cpp game.cpp mf_client.cpp mf_vehiclespawn.cpp)
list(TRANSFORM GAME_SOURCES PREPEND "${SOURCE_DIR}/game/")

set(CGAME_SOURCES
    cg.cpp cg_boat.cpp cg_consolecmds.cpp cg_draw.cpp cg_drawnewhud.cpp cg_drawoldhud.cpp
    cg_drawtools.cpp cg_effects.cpp cg_ents.cpp cg_event.cpp cg_groundvehicle.cpp cg_helo.cpp cg_info.cpp
    cg_localents.cpp cg_lqm.cpp cg_main.cpp cg_marks.cpp cg_miscvehicle.cpp cg_missioneditor.cpp
    cg_particles.cpp cg_plane.cpp cg_players.cpp cg_playerstate.cpp cg_predict.cpp cg_scoreboard.cpp
    cg_servercmds.cpp cg_snapshot.cpp cg_testmodel.cpp cg_util.cpp cg_vehicle.cpp cg_vehicledraw.cpp
    cg_view.cpp cg_weapons.cpp ui_displaycontext_cg.cpp ui_utils_cg.cpp)
list(TRANSFORM CGAME_SOURCES PREPEND "${SOURCE_DIR}/cgame/")

set(UI_SOURCES
    ui.cpp ui_atoms.cpp ui_commanddef.cpp ui_controls.cpp ui_displaycontext.cpp
    ui_displaycontext_ui.cpp ui_gameinfo.cpp ui_hashutils.cpp ui_itemcapture.cpp
    ui_keywordhash.cpp ui_main.cpp ui_memtools.cpp ui_precomp.cpp ui_util.cpp ui_utils.cpp
    ui_utils_ui.cpp)
list(TRANSFORM UI_SOURCES PREPEND "${SOURCE_DIR}/ui/")

set(RENDERER_SOURCES
    tr_animation.cpp tr_backend.cpp tr_bsp.cpp tr_cmds.cpp tr_curve.cpp tr_flares.cpp tr_font.cpp
    tr_image.cpp tr_image_jpg.cpp tr_init.cpp tr_light.cpp tr_main.cpp tr_marks.cpp tr_mesh.cpp tr_model.cpp tr_noise.cpp
    tr_scene.cpp tr_shade.cpp tr_shade_calc.cpp tr_shader.cpp tr_shadows.cpp tr_sky.cpp tr_surface.cpp
    tr_world.cpp)
list(TRANSFORM RENDERER_SOURCES PREPEND "${SOURCE_DIR}/renderer/")

set(QCOMMON_SOURCES
    qcommon/cm_load.cpp qcommon/cm_patch.cpp qcommon/cm_polylib.cpp qcommon/cm_test.cpp
    qcommon/cm_trace.cpp qcommon/cmd.cpp qcommon/common.cpp qcommon/cvar.cpp qcommon/files.cpp
    qcommon/huffman.cpp qcommon/md4.cpp qcommon/msg.cpp qcommon/net_chan.cpp qcommon/unzip.cpp
    game/q_math.cpp game/q_shared.cpp)
list(TRANSFORM QCOMMON_SOURCES PREPEND "${SOURCE_DIR}/")

set(SERVER_SOURCES
    sv_bot.cpp sv_ccmds.cpp sv_client.cpp sv_entity.cpp sv_game.cpp sv_init.cpp
    sv_main.cpp sv_net_chan.cpp sv_server.cpp sv_snapshot.cpp sv_world.cpp)
list(TRANSFORM SERVER_SOURCES PREPEND "${SOURCE_DIR}/server/")

set(CLIENT_SOURCES
    cl_cgame.cpp cl_cin.cpp cl_console.cpp cl_input.cpp cl_keys.cpp cl_main.cpp cl_net_chan.cpp
    cl_parse.cpp cl_scrn.cpp cl_ui.cpp qal.cpp snd_adpcm.cpp snd_codec.cpp snd_codec_wav.cpp snd_dma.cpp snd_main.cpp snd_mem.cpp snd_mix.cpp snd_openal.cpp snd_wavelet.cpp cl_avi.cpp)
list(TRANSFORM CLIENT_SOURCES PREPEND "${SOURCE_DIR}/client/")

set(SYSTEM_SOURCES
    ${SOURCE_DIR}/sys/sys_main.cpp
    ${SOURCE_DIR}/sys/con_log.cpp
    ${SOURCE_DIR}/qcommon/net_ip.cpp)

set(SDL_SOURCES
    ${SOURCE_DIR}/sdl/sdl_input.cpp
    ${SOURCE_DIR}/sdl/sdl_snd.cpp)

set(SDL_RENDERER_SOURCES
    ${SOURCE_DIR}/sdl/sdl_glimp.cpp
    ${SOURCE_DIR}/sdl/sdl_gamma.cpp)
