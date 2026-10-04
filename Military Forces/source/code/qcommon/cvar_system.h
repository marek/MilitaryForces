#ifndef CVAR_SYSTEM_H
#define CVAR_SYSTEM_H

class CvarSystem
{
public:
	static cvar_t *get( const char *var_name, const char *value, int flags );
	static void register_( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	static void update( vmCvar_t *vmCvar );
	static void set( const char *var_name, const char *value );
	static cvar_t *set2(const char *var_name, const char *value, bool force);
	static void setSafe( const char *var_name, const char *value );
	static void setLatched( const char *var_name, const char *value);
	static void setValue( const char *var_name, float value );
	static void setValueSafe( const char *var_name, float value );
	static float variableValue( const char *var_name );
	static int variableIntegerValue( const char *var_name );
	static char *variableString( const char *var_name );
	static void variableStringBuffer( const char *var_name, char *buffer, int bufsize );
	static int flags(const char *var_name);
	static void commandCompletion( void(*callback)(const char *s) );
	static void reset( const char *var_name );
	static void forceReset(const char *var_name);
	static void setCheatState( );
	static bool command( );
	static void writeVariables( fileHandle_t f );
	static void init( );
	static char *infoString( int bit );
	static char *infoString_Big( int bit );
	static void infoStringBuffer( int bit, char *buff, int buffsize );
	static void checkRange( cvar_t *cv, float minVal, float maxVal, bool shouldBeIntegral );
	static void setDescription( cvar_t *var, const char *var_description );
	static void restart(bool unsetVM);
	static void restart_f( );
	static void completeCvarName( char *args, int argNum );
	static void print( cvar_t *v );
	static void print_f();
	static void toggle_f( );
	static void set_f( );
	static void reset_f( );
	static void list_f( );
	static void listModified_f( );
	static cvar_t *unset(cvar_t *cv);
	static void unset_f();
};

#endif
