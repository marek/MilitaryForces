#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

class CommandBuffer
{
public:
	static void init ();
	static void addText( const char *text );
	static void executeText( int exec_when, const char *text );
	static void execute ();
	static void insertText( const char *text );
};

#endif
