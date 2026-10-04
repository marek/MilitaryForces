#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

typedef void (*xcommand_t) ();
typedef void (*completionFunc_t)( char *args, int argNum );

struct cmd_function_s;
using cmd_function_t = cmd_function_s;

class CommandSystem
{
public:
	static void init ();
	static void addCommand( const char *cmd_name, xcommand_t function );
	static void removeCommand( const char *cmd_name );
	static void removeCommandSafe( const char *cmd_name );
	static void commandCompletion( void(*callback)(const char *s) );
	static void setCommandCompletionFunc( const char *command, completionFunc_t complete );
	static void completeArgument( const char *command, char *args, int argNum );
	static void completeCfgName( char *args, int argNum );
	static int argc ();
	static char *argv (int arg);
	static void argvBuffer( int arg, char *buffer, int bufferLength );
	static char *args ();
	static char *argsFrom( int arg );
	static void argsBuffer( char *buffer, int bufferLength );
	static char *cmd ();
	static void args_Sanitize( );
	static void tokenizeString( const char *text );
	static void tokenizeStringIgnoreQuotes( const char *text_in );
	static void executeString( const char *text );
	static void wait_f( );
	static void exec_f( );
	static void vstr_f( );
	static void echo_f ();
	static cmd_function_t *findCommand( const char *cmd_name );
	static void list_f ();
};

#endif
