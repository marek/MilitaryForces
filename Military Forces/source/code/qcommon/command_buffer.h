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
	// hold the rest of the buffer back for this many executions
	static void wait( int count );

private:
	static constexpr int maxBuffer = 128*1024;
	struct Text {
		byte	*data;
		int		maxsize;
		int		cursize;
	};

	static int	wait_;
	static Text	text_;
	static byte	textBuf_[maxBuffer];
};

#endif
