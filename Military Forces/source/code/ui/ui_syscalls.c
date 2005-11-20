/*
 * $Id: ui_syscalls.c,v 1.7 2005-11-20 11:21:38 thebjoern Exp $
*/
#include "ui_local.h"
#include "..\cgame\cg_public.h"
#include "..\qcommon\qcommon.h"
#include "..\renderer\tr_public.h"
#include "..\server\server.h"
#include "..\client\client.h"

//decls
void Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void Key_GetBindingBuf( int keynum, char *buf, int buflen );
int Key_GetCatcher( void );
void GetClipboardData( char *buf, int buflen );
void GetClientState( uiClientState_t *state );
void trap_GetGlconfig( glconfig_t *glconfig );
void CL_GetGlconfig( glconfig_t *config );
void CL_AddCgameCommand( const char *cmdName );
void CL_CM_LoadMap( const char *mapname );
void CL_GetGameState( gameState_t *gs );
void CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );
bool CL_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );
bool CL_GetServerCommand( int serverCommandNumber );
int CL_GetCurrentCmdNumber();
bool CL_GetUserCmd( int cmdNumber, usercmd_t *ucmd );
void CL_SetUserCmdValue( int userCmdValue, float sensitivityScale );
int GetConfigString(int index, char *buf, int size);
int LAN_GetServerCount( int source );
void LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int LAN_GetServerPing( int source, int n );
int LAN_GetServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void LAN_ResetPings(int source);
void LAN_ClearPing( int n );
void LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void LAN_GetPingInfo( int n, char *buf, int buflen );
void LAN_MarkServerVisible(int source, int n, bool visible );
int LAN_ServerIsVisible(int source, int n );
bool LAN_UpdateVisiblePings(int source );
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen );
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen );
int LAN_AddServer(int source, const char *name, const char *address);
void LAN_RemoveServer(int source, const char *addr);
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen );
int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void CLUI_SetCDKey( char *buf );
void CLUI_GetCDKey( char *buf, int buflen );


struct gentity_t;





extern refexport_t	re;




//int trap_Milliseconds( void ) 
//{
//	//return syscall( UI_MILLISECONDS ); 
//	return Sys_Milliseconds();
//}

void trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) 
{
	//syscall( UI_CVAR_REGISTER, cvar, var_name, value, flags );
	Cvar_Register( cvar, var_name, value, flags ); 
}

void trap_Cvar_Update( vmCvar_t *cvar ) 
{
	//syscall( UI_CVAR_UPDATE, cvar );
	Cvar_Update( cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) 
{
	//syscall( UI_CVAR_SET, var_name, value );
	Cvar_Set( var_name, value );
}

float trap_Cvar_VariableValue( const char *var_name ) 
{
//	int temp;
//	temp = syscall( UI_CVAR_VARIABLEVALUE, var_name );
//	return (*(float*)&temp);
	return Cvar_VariableValue( var_name );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) 
{
	return Cvar_VariableIntegerValue( var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) 
{
	//syscall( UI_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize );
	Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( const char *var_name, float value ) 
{
	//syscall( UI_CVAR_SETVALUE, var_name, PASSFLOAT( value ) );
	Cvar_SetValue( var_name, value );
}

void trap_Cvar_Reset( const char *name ) 
{
	//syscall( UI_CVAR_RESET, name ); 
	Cvar_Reset( name );
}

void trap_Cvar_Create( const char *var_name, const char *var_value, int flags ) 
{
	//syscall( UI_CVAR_CREATE, var_name, var_value, flags );
	Cvar_Get( var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize ) 
{
	//syscall( UI_CVAR_INFOSTRINGBUFFER, bit, buffer, bufsize );
	Cvar_InfoStringBuffer( bit, buffer, bufsize );
}

int trap_Argc( void ) 
{
	//return syscall( UI_ARGC );
	return Cmd_Argc();
}

void trap_Argv( int n, char *buffer, int bufferLength ) 
{
	//syscall( UI_ARGV, n, buffer, bufferLength );
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void trap_Args( char *buffer, int bufferLength ) 
{
	Cmd_ArgsBuffer( buffer, bufferLength );
}

void trap_Cmd_ExecuteText( int exec_when, const char *text ) 
{
	//syscall( UI_CMD_EXECUTETEXT, exec_when, text );
	Cbuf_ExecuteText( exec_when, text );
}

void trap_SendConsoleCommand( const char *text ) 
{
	Cbuf_AddText( text );
}

void trap_AddCommand( const char *cmdName ) 
{
	CL_AddCgameCommand( cmdName );
}

void trap_RemoveCommand( const char *cmdName ) 
{
	Cmd_RemoveCommand( cmdName );
}

void trap_SendClientCommand( const char *s ) 
{
	CL_AddReliableCommand( s );
}

int trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) 
{
	//return syscall( UI_FS_FOPENFILE, qpath, f, mode );
	return FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void *buffer, int len, fileHandle_t f ) 
{
	//syscall( UI_FS_READ, buffer, len, f );
	FS_Read2( buffer, len, f );
}

void trap_FS_Write( const void *buffer, int len, fileHandle_t f ) 
{
	FS_Write( buffer, len, f );
	//syscall( UI_FS_WRITE, buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f ) 
{
	//syscall( UI_FS_FCLOSEFILE, f );
	FS_FCloseFile( f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) 
{
	//return syscall( UI_FS_GETFILELIST, path, extension, listbuf, bufsize );
	return FS_GetFileList( path, extension, listbuf, bufsize );
}

int trap_FS_Seek( fileHandle_t f, long offset, int origin ) 
{
	//return syscall( UI_FS_SEEK, f, offset, origin );
	return FS_Seek( f, offset, origin );
}

void trap_R_LoadWorldMap( const char *mapname ) 
{
	re.LoadWorld( mapname );
}

qhandle_t trap_R_RegisterModel( const char *name ) 
{
	//return syscall( UI_R_REGISTERMODEL, name );
	return re.RegisterModel( name );
}

qhandle_t trap_R_RegisterSkin( const char *name ) 
{
	//return syscall( UI_R_REGISTERSKIN, name );
	return re.RegisterSkin( name );
}

void trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) 
{
	//syscall( UI_R_REGISTERFONT, fontName, pointSize, font );
	re.RegisterFont( fontName, pointSize, font );
}

qhandle_t trap_R_RegisterShader( const char *name ) 
{
	return re.RegisterShader( name );
}

qhandle_t trap_R_RegisterShaderNoMip( const char *name ) 
{
	//return syscall( UI_R_REGISTERSHADERNOMIP, name );
	return re.RegisterShaderNoMip( name );
}

void trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset ) 
{
	//syscall( UI_R_REMAP_SHADER, oldShader, newShader, timeOffset );
	re.RemapShader( oldShader, newShader, timeOffset );
}

void trap_R_ClearScene( void ) 
{
	//syscall( UI_R_CLEARSCENE );
	re.ClearScene();
}

void trap_R_AddRefEntityToScene( const refEntity_t *refEnt ) 
{
	//syscall( UI_R_ADDREFENTITYTOSCENE, re );
	re.AddRefEntityToScene( refEnt );
}

void trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) 
{
	//syscall( UI_R_ADDPOLYTOSCENE, hShader, numVerts, verts );
	re.AddPolyToScene( hShader, numVerts, verts, 1 );
}

void trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) 
{
	//syscall( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
	re.AddLightToScene( org, intensity, r, g, b );
}

void trap_R_RenderScene( const refdef_t *fd ) 
{
	//syscall( UI_R_RENDERSCENE, fd );
	re.RenderScene( fd );
}

void trap_R_SetColor( const float *rgba ) 
{
	//syscall( UI_R_SETCOLOR, rgba );
	re.SetColor( rgba );
}

void trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader )
{
	//syscall( UI_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader );
	re.DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) 
{
	//syscall( UI_R_MODELBOUNDS, model, mins, maxs );
	re.ModelBounds( model, mins, maxs );
}

int	trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) 
{
	return re.LightForPoint( point, ambientLight, directedLight, lightDir );
}

int	trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName ) 
{
	return re.LerpTag( tag, mod, startFrame, endFrame, frac, tagName );
}

void trap_UpdateScreen( void ) 
{
	//syscall( UI_UPDATESCREEN );
	SCR_UpdateScreen();
}

int trap_CM_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName ) 
{
	//return syscall( UI_CM_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName );
	re.LerpTag( tag, mod, startFrame, endFrame, frac, tagName );
	return 0;
}

void trap_CM_LoadMap( const char *mapname ) 
{
	CL_CM_LoadMap( mapname );
}

int	trap_CM_NumInlineModels() 
{
	return CM_NumInlineModels();
}

clipHandle_t trap_CM_InlineModel( int index ) 
{
	return CM_InlineModel( index );
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs ) 
{
	return CM_TempBoxModel( mins, maxs, false );
}

int	trap_CM_PointContents( const vec3_t p, clipHandle_t model ) 
{
	return CM_PointContents( p, model );
}

int	trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) 
{
	return CM_TransformedPointContents( p, model, origin, angles );
}

void trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					   const vec3_t mins, const vec3_t maxs,
					   clipHandle_t model, int brushmask ) 
{
	CM_BoxTrace( results, start, end, mins, maxs, model, brushmask, false );
}

void trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs,
						  clipHandle_t model, int brushmask,
						  const vec3_t origin, const vec3_t angles ) 
{
	CM_TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask, origin, angles, false );
}

int	trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
				const vec3_t projection,
				int maxPoints, vec3_t pointBuffer,
				int maxFragments, markFragment_t *fragmentBuffer ) 
{
	return re.MarkFragments( numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum ) 
{
	//syscall( UI_S_STARTLOCALSOUND, sfx, channelNum );
	S_StartLocalSound( sfx, channelNum );
}

void trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) 
{
	S_StartSound( origin, entityNum, entchannel, sfx );
}

sfxHandle_t	trap_S_RegisterSound( const char *sample, bool compressed ) 
{
	//return syscall( UI_S_REGISTERSOUND, sample, compressed );
	return S_RegisterSound( sample, compressed );
}

void trap_S_ClearLoopingSounds( bool killall ) 
{
	S_ClearLoopingSounds( killall );
}

void trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) 
{
	S_AddLoopingSound( entityNum, origin, velocity, sfx );
}

void trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) 
{
	S_AddRealLoopingSound( entityNum, origin, velocity, sfx );
}

void trap_S_StopLoopingSound( int entityNum ) 
{
	S_StopLoopingSound( entityNum );
}

void trap_S_StopBackgroundTrack( void ) 
{
	//syscall( UI_S_STOPBACKGROUNDTRACK );
	S_StopBackgroundTrack();
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop) 
{
	//syscall( UI_S_STARTBACKGROUNDTRACK, intro, loop );
	S_StartBackgroundTrack( intro, loop );
}

void trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin ) 
{
	S_UpdateEntityPosition( entityNum, origin );
}

void trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater ) 
{
	S_Respatialize( entityNum, origin, axis, inwater );
}

void trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) 
{
	//syscall( UI_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen );
	Key_KeynumToStringBuf( keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( int keynum, char *buf, int buflen ) 
{
	//syscall( UI_KEY_GETBINDINGBUF, keynum, buf, buflen );
	Key_GetBindingBuf( keynum, buf, buflen );
}

void trap_Key_SetBinding( int keynum, const char *binding ) 
{
	//syscall( UI_KEY_SETBINDING, keynum, binding );
	Key_SetBinding( keynum, binding );
}

int trap_Key_GetKey( const char *binding )
{
	return Key_GetKey( binding );
}

int trap_Key_IsDown( int keynum ) 
{
	//return syscall( UI_KEY_ISDOWN, keynum );
	return Key_IsDown( keynum );
}

bool trap_Key_GetOverstrikeMode( void ) 
{
	//return syscall( UI_KEY_GETOVERSTRIKEMODE );
	return Key_GetOverstrikeMode();
}

void trap_Key_SetOverstrikeMode( bool state ) 
{
	//syscall( UI_KEY_SETOVERSTRIKEMODE, state );
	Key_SetOverstrikeMode( state );
}

void trap_Key_ClearStates( void ) 
{
	//syscall( UI_KEY_CLEARSTATES );
	Key_ClearStates();
}

int trap_Key_GetCatcher( void ) 
{
	//return syscall( UI_KEY_GETCATCHER );
	return Key_GetCatcher();
}

void trap_Key_SetCatcher( int catcher ) 
{
	//syscall( UI_KEY_SETCATCHER, catcher );
	Key_SetCatcher( catcher );
}

void trap_GetClipboardData( char *buf, int bufsize ) 
{
	//syscall( UI_GETCLIPBOARDDATA, buf, bufsize );
	GetClipboardData( buf, bufsize );
}

void trap_GetClientState( uiClientState_t *state ) 
{
	//syscall( UI_GETCLIENTSTATE, state );
	GetClientState( state );
}

void trap_GetGlconfig( glconfig_t *glconfig ) 
{
	//syscall( UI_GETGLCONFIG, glconfig );
	CL_GetGlconfig( glconfig );
}

int trap_GetConfigString( int index, char* buff, int buffsize ) 
{
	//return syscall( UI_GETCONFIGSTRING, index, buff, buffsize );
	return GetConfigString( index, buff, buffsize );
}

int	trap_LAN_GetServerCount( int source ) 
{
	//return syscall( UI_LAN_GETSERVERCOUNT, source );
	return LAN_GetServerCount( source );
}

void trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) 
{
	//syscall( UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen );
	LAN_GetServerAddressString( source, n, buf, buflen );
}

void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen ) 
{
	//syscall( UI_LAN_GETSERVERINFO, source, n, buf, buflen );
	LAN_GetServerInfo( source, n, buf, buflen );
}

int trap_LAN_GetServerPing( int source, int n ) 
{
	//return syscall( UI_LAN_GETSERVERPING, source, n );
	return LAN_GetServerPing( source, n );
}

//int trap_LAN_GetPingQueueCount( void ) {
//	return syscall( UI_LAN_GETPINGQUEUECOUNT );
//}

int trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) 
{
	//return syscall( UI_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen );
	return LAN_GetServerStatus( serverAddress, serverStatus, maxLen );
}

void trap_LAN_SaveCachedServers() 
{
	//syscall( UI_LAN_SAVECACHEDSERVERS );
	LAN_SaveServersToCache();
}

void trap_LAN_LoadCachedServers() 
{
	//syscall( UI_LAN_LOADCACHEDSERVERS );
	LAN_LoadCachedServers();
}

void trap_LAN_ResetPings(int n) 
{
	//syscall( UI_LAN_RESETPINGS, n );
	LAN_ResetPings( n );
}

void trap_LAN_ClearPing( int n ) 
{
	//syscall( UI_LAN_CLEARPING, n );
	LAN_ClearPing( n );
}

void trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) 
{
	//syscall( UI_LAN_GETPING, n, buf, buflen, pingtime );
	LAN_GetPing( n, buf, buflen, pingtime );
}

void trap_LAN_GetPingInfo( int n, char *buf, int buflen ) 
{
	//syscall( UI_LAN_GETPINGINFO, n, buf, buflen );
	LAN_GetPingInfo( n, buf, buflen );
}

void trap_LAN_MarkServerVisible( int source, int n, bool visible ) 
{
	//syscall( UI_LAN_MARKSERVERVISIBLE, source, n, visible );
	LAN_MarkServerVisible( source, n, visible );
}

int trap_LAN_ServerIsVisible( int source, int n) 
{
	//return syscall( UI_LAN_SERVERISVISIBLE, source, n );
	return LAN_ServerIsVisible( source, n );
}

int trap_LAN_UpdateVisiblePings( int source ) 
{
	//return syscall( UI_LAN_UPDATEVISIBLEPINGS, source );
	return LAN_UpdateVisiblePings( source );
}

int trap_LAN_AddServer(int source, const char *name, const char *addr)
{
	//return syscall( UI_LAN_ADDSERVER, source, name, addr );
	return LAN_AddServer( source, name, addr );
}

void trap_LAN_RemoveServer(int source, const char *addr) 
{
	//syscall( UI_LAN_REMOVESERVER, source, addr );
	LAN_RemoveServer( source, addr );
}

int trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) 
{
	//return syscall( UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2 );
	return LAN_CompareServers( source, sortKey, sortDir, s1, s2 );
}

int trap_MemoryRemaining( void ) 
{
	//return syscall( UI_MEMORY_REMAINING );
	return Hunk_MemoryRemaining();
}

void trap_GetCDKey( char *buf, int buflen ) 
{
	//syscall( UI_GET_CDKEY, buf, buflen );
	CLUI_GetCDKey( buf, buflen );
}

void trap_SetCDKey( char *buf ) 
{
	//syscall( UI_SET_CDKEY, buf );
	CLUI_SetCDKey( buf );
}

//int trap_PC_AddGlobalDefine( char *define ) {
//	return syscall( UI_PC_ADD_GLOBAL_DEFINE, define );
//}

//int trap_PC_LoadSource( const char *filename ) 
//{
//	//return syscall( UI_PC_LOAD_SOURCE, filename );
//	return botlib_export->PC_LoadSourceHandle( filename );
//}

//int trap_PC_FreeSource( int handle ) 
//{
//	//return syscall( UI_PC_FREE_SOURCE, handle );
//	return botlib_export->PC_FreeSourceHandle( handle );
//}

//int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) 
//{
//	//return syscall( UI_PC_READ_TOKEN, handle, pc_token );
//	return botlib_export->PC_ReadTokenHandle( handle, pc_token );
//}

//int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) 
//{
//	//return syscall( UI_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
//	return botlib_export->PC_SourceFileAndLine( handle, filename, line );
//}

int trap_RealTime(qtime_t *qtime) 
{
	//return syscall( UI_REAL_TIME, qtime );
	return Com_RealTime(qtime);
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to false (do not alter gamestate)
int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits) 
{
  //return syscall(UI_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
	return CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}
 
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic(int handle) 
{
	//return static_cast<e_status>(syscall(UI_CIN_STOPCINEMATIC, handle));
	return CIN_StopCinematic(handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic (int handle) 
{
	//return static_cast<e_status>(syscall(UI_CIN_RUNCINEMATIC, handle));
	return CIN_RunCinematic(handle);
}
 

// draws the current frame
void trap_CIN_DrawCinematic (int handle) 
{
  //syscall(UI_CIN_DRAWCINEMATIC, handle);
	CIN_DrawCinematic(handle);
}
 

// allows you to resize the animation dynamically
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h) 
{
  //syscall(UI_CIN_SETEXTENTS, handle, x, y, w, h);
	CIN_SetExtents(handle, x, y, w, h);
}

int trap_VerifyCDKey( const char *key, const char *chksum) 
{
	//return syscall( UI_VERIFY_CDKEY, key, chksum);
	return CL_CDKeyValidate( key, chksum );
}

void trap_GetGameState( gameState_t *gamestate ) 
{
	CL_GetGameState( gamestate );
}

void trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) 
{
	CL_GetCurrentSnapshotNumber( snapshotNumber, serverTime );
}

int	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot ) 
{
	return CL_GetSnapshot( snapshotNumber, snapshot );
}

int	trap_GetServerCommand( int serverCommandNumber ) 
{
	return CL_GetServerCommand( serverCommandNumber );
}

int	trap_GetCurrentCmdNumber() 
{
	return CL_GetCurrentCmdNumber();
}

int	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd ) 
{
	return CL_GetUserCmd( cmdNumber, ucmd );
}

void trap_GetClientUsercmd( int clientNum, usercmd_t *cmd ) 
{
	SV_GetUsercmd( clientNum, cmd );
}

void trap_SetUserCmdValue( int stateValue, float sensitivityScale ) 
{
	CL_SetUserCmdValue( stateValue, sensitivityScale );
}

void trap_SnapVector( float *v ) 
{
	Sys_SnapVector( v );
}

bool trap_GetEntityToken( char *buffer, int bufferSize ) 
{
	return SV_GetEntityToken( buffer, bufferSize );
}

int trap_EntityContact( const vec3_t mins, const vec3_t maxs, const entityState_t* s, const entityShared_t* r ) 
{
	//return syscall( G_ENTITY_CONTACT, mins, maxs, ent );
	return SV_EntityContact( mins, maxs, s, r, false );
}

int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) 
{
	//return syscall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
	return SV_AreaEntities( mins, maxs, list, maxcount );
}

void trap_LinkEntity( entityState_t* s, entityShared_t* r ) 
{
	//syscall( G_LINKENTITY, ent );
	SV_LinkEntity( s, r );
}

void trap_UnlinkEntity( entityState_t* s, entityShared_t* r ) 
{
	//syscall( G_UNLINKENTITY, ent );
	SV_UnlinkEntity( s, r );
}

int trap_AreasConnected( int area1, int area2 ) 
{
	//return syscall( G_AREAS_CONNECTED, area1, area2 );
	return CM_AreasConnected( area1, area2 );
}

int trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) 
{
	//return syscall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
	return SV_inPVSIgnorePortals( p1, p2 );
}

int trap_InPVS( const vec3_t p1, const vec3_t p2 ) 
{
	//return syscall( G_IN_PVS, p1, p2 );
	return SV_inPVS( p1, p2 );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) 
{
	//return syscall( G_POINT_CONTENTS, point, passEntityNum );
	return SV_PointContents( point, passEntityNum );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) 
{
	//syscall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
	SV_Trace( results, start, mins, maxs, end, passEntityNum, contentmask, false );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) 
{
	//syscall( G_GET_SERVERINFO, buffer, bufferSize );
	SV_GetServerinfo( buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) 
{
	//syscall( G_SET_USERINFO, num, buffer );
	SV_SetUserinfo( num, buffer );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) 
{
	//syscall( G_GET_USERINFO, num, buffer, bufferSize );
	SV_GetUserinfo( num, buffer, bufferSize );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) 
{
	//syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
	SV_GetConfigstring( num, buffer, bufferSize );
}

void trap_SetConfigstring( int num, const char *string ) 
{
	//syscall( G_SET_CONFIGSTRING, num, string );
	SV_SetConfigstring( num, string );
}

void trap_SendServerCommand( int clientNum, const char *text ) 
{
	//syscall( G_SEND_SERVER_COMMAND, clientNum, text );
	SV_GameSendServerCommand( clientNum, text );
}

void trap_DropClient( int clientNum, const char *reason ) 
{
	//syscall( G_DROP_CLIENT, clientNum, reason );
	SV_GameDropClient( clientNum, reason );
}

void trap_AdjustAreaPortalState( const entityState_t* s, const entityShared_t* r, bool open ) 
{
	//syscall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
	SV_AdjustAreaPortalState( s, r, open );
}

void trap_SetBrushModel( entityState_t* s, entityShared_t* r, const char *name ) 
{
	//syscall( G_SET_BRUSH_MODEL, ent, name );
	SV_SetBrushModel( s, r, name );
}

void trap_LocateGameData( void* gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) 
{
	//syscall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
	SV_LocateGameData( gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

