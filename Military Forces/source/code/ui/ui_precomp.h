#ifndef __UI_PRECOMP_H__
#define __UI_PRECOMP_H__


#include <string>
#include <vector>


#ifndef PATH_SEPERATORSTR
	#if defined(WIN32)|defined(_WIN32)|defined(__NT__)|defined(__WINDOWS__)|defined(__WINDOWS_386__)
		#define PATHSEPERATOR_STR		"\\"
	#else
		#define PATHSEPERATOR_STR		"/"
	#endif
#endif
#ifndef PATH_SEPERATORCHAR
	#if defined(WIN32)|defined(_WIN32)|defined(__NT__)|defined(__WINDOWS__)|defined(__WINDOWS_386__)
		#define PATHSEPERATOR_CHAR		'\\'
	#else
		#define PATHSEPERATOR_CHAR		'/'
	#endif
#endif


struct UI_PrecompilerTools
{
	//punctuation sub type
	//--------------------
	enum
	{
		P_RSHIFT_ASSIGN					= 1,
		P_LSHIFT_ASSIGN					= 2,
		P_PARMS							= 3,
		P_PRECOMPMERGE					= 4,

		P_LOGIC_AND						= 5,
		P_LOGIC_OR						= 6,
		P_LOGIC_GEQ						= 7,
		P_LOGIC_LEQ						= 8,
		P_LOGIC_EQ						= 9,
		P_LOGIC_UNEQ					= 10,

		P_MUL_ASSIGN					= 11,
		P_DIV_ASSIGN					= 12,
		P_MOD_ASSIGN					= 13,
		P_ADD_ASSIGN					= 14,
		P_SUB_ASSIGN					= 15,
		P_INC							= 16,
		P_DEC							= 17,

		P_BIN_AND_ASSIGN				= 18,
		P_BIN_OR_ASSIGN					= 19,
		P_BIN_XOR_ASSIGN				= 20,
		P_RSHIFT						= 21,
		P_LSHIFT						= 22,

		P_POINTERREF					= 23,
		P_CPP1							= 24,
		P_CPP2							= 25,
		P_MUL							= 26,
		P_DIV							= 27,
		P_MOD							= 28,
		P_ADD							= 29,
		P_SUB							= 30,
		P_ASSIGN						= 31,

		P_BIN_AND						= 32,
		P_BIN_OR						= 33,
		P_BIN_XOR						= 34,
		P_BIN_NOT						= 35,

		P_LOGIC_NOT						= 36,
		P_LOGIC_GREATER					= 37,
		P_LOGIC_LESS					= 38,

		P_REF							= 39,
		P_COMMA							= 40,
		P_SEMICOLON						= 41,
		P_COLON							= 42,
		P_QUESTIONMARK					= 43,

		P_PARENTHESESOPEN				= 44,
		P_PARENTHESESCLOSE				= 45,
		P_BRACEOPEN						= 46,
		P_BRACECLOSE					= 47,
		P_SQBRACKETOPEN					= 48,
		P_SQBRACKETCLOSE				= 49,
		P_BACKSLASH						= 50,

		P_PRECOMP						= 51,
		P_DOLLAR						= 52
	};

	enum
	{
		SCFL_NOERRORS					= 0x0001,
		SCFL_NOWARNINGS					= 0x0002,
		SCFL_NOSTRINGWHITESPACES		= 0x0004,
		SCFL_NOSTRINGESCAPECHARS		= 0x0008,
		SCFL_PRIMITIVE					= 0x0010,
		SCFL_NOBINARYNUMBERS			= 0x0020,
		SCFL_NONUMBERVALUES				= 0x0040
	};

	enum
	{
		TT_STRING						= 1,			// string
		TT_LITERAL						= 2,			// literal
		TT_NUMBER						= 3,			// number
		TT_NAME							= 4,			// name
		TT_PUNCTUATION					= 5,			// punctuation
		TT_DECIMAL						= 0x0008,		// decimal number
		TT_HEX							= 0x0100,		// hexadecimal number
		TT_OCTAL						= 0x0200,		// octal number
		TT_BINARY						= 0x0400,		// binary number
		TT_FLOAT						= 0x0800,		// floating point number
		TT_INTEGER						= 0x1000,		// integer number
		TT_LONG							= 0x2000,		// long number
		TT_UNSIGNED						= 0x4000		// unsigned number
	};

	enum
	{
		BUILTIN_LINE					= 1,
		BUILTIN_FILE					= 2,
		BUILTIN_DATE					= 3,
		BUILTIN_TIME					= 4,
		BUILTIN_STDC					= 5
	};

	struct SourceFile;
	struct Script;
	struct Punctuation;
	struct Token;
	struct Define;
	struct Indent;
	struct PC_Token;
	struct Operator;
	struct Value;
	struct Directive;
	struct Directive_if;
	struct Directive_ifdef;
	struct Directive_ifndef;
	struct Directive_elif;
	struct Directive_else;
	struct Directive_endif;
	struct Directive_include;
	struct Directive_define;
	struct Directive_undef;
	struct Directive_line;
	struct Directive_error;
	struct Directive_pragma;
	struct Directive_eval;
	struct Directive_evalfloat;
	struct DollarDirective_evalint;
	struct DollarDirective_evalfloat;

						UI_PrecompilerTools();
						~UI_PrecompilerTools();

	int					loadSourceHandle( const char *filename );
	int					freeSourceHandle( int handle );
	int					readTokenHandle( int handle, PC_Token* pc_token );
	int					sourceFileAndLine( int handle, char *filename, int *line );

protected:

	void				setBaseFolder( std::string const& folder ) { basefolder_ = folder; }

	SourceFile*			loadSourceFile( const char *filename );
	void				freeSourceFile( SourceFile* source );

	int					readLine( SourceFile* source, Token* token );

	Script*				loadScriptFile( const char *filename );
	void				freeScript( Script* script );
	int					endOfScript( Script* script );
	void				pushScript( SourceFile* source, Script* script );

	void				setScriptPunctuations( Script* script, Punctuation* p );
	void				createPunctuationTable( Script* script, Punctuation* punctuations );

	void				addGlobalDefinesToSource( SourceFile* source );
	Define*				copyDefine( SourceFile* source, Define* define );
	void				addDefineToHash( Define* define, Define** definehash );
	void				freeDefine( Define* define );
	Define*				findHashedDefine( Define** definehash, char *name );
	int					expandDefineIntoSource( SourceFile* source, Token* deftoken, Define* define );
	int					expandDefine( SourceFile* source, Token* deftoken, Define* define,
									  Token** firsttoken, Token** lasttoken );
	int					expandBuiltinDefine( SourceFile* source, Token* deftoken, Define* define,
											 Token** firsttoken, Token** lasttoken );
	int					readDefineParms( SourceFile* source, Define* define, Token** parms, int maxparms );
	int					findDefineParm( Define *define, char *name );
	
	int					scriptReadToken( Script* script, Token* token );
	int					scriptReadWhiteSpace( Script* script );
	int					scriptReadString( Script* script, Token* token, int quote );
	int					scriptReadNumber( Script* script, Token* token );
	int					scriptReadPrimitive( Script* script, Token* token );
	int					scriptReadName( Script* script, Token* token );
	int					scriptReadPunctuation( Script* script, Token* token );
	int					scriptReadEscapeCharacter( Script* script, char *ch );

	int					readSourceToken( SourceFile* source, Token* token );
	int					unreadSourceToken( SourceFile* source, Token* token );

	int					readDirective( SourceFile* source );
	int					readDollarDirective( SourceFile* source );

	int					readToken( SourceFile* source, Token* token );
	Token*				copyToken( Token* token );
	void				freeToken( Token* token );
	int					stringizeTokens( Token* tokens, Token* token );
	int					mergeTokens( Token* t1, Token* t2 );

	void				popIndent( SourceFile* source, int *type, int *skip );
	void				pushIndent( SourceFile* source, int type, int skip );

	int					evaluate( SourceFile *source, signed long int *intvalue,
								  double *floatvalue, int integer );
	int					evaluateTokens( SourceFile* source, Token* tokens, signed long int *intvalue,
										double *floatvalue, int integer );

	int					nameHash( char *name );
	void				stripDoubleQuotes( char *string );
	void				numberValue( char *string, int subtype, unsigned long int *intvalue, long double *floatvalue );
	void				convertPath( char *path );
	int					operatorPriority( int op );
	int					whiteSpaceBeforeToken( Token* token );
	void				unreadSignToken( SourceFile* source );
	int					dollarEvaluate( SourceFile* source, signed long int *intvalue,
										double *floatvalue, int integer );
	int					directive_undef( SourceFile* source );
	int					checkTokenString( SourceFile* source, char *string );
	void				clearTokenWhiteSpace( Token* token );

private:
	static const int	k_MAX_SOURCEFILES = 64;
	static const int	k_DEFINEHASHSIZE = 1024;
	static const int	k_MAX_TOKEN = 1024;
	static const int	k_DEFINE_FIXED = 0x0001;
	static const int	k_MAX_DEFINEPARMS = 128;
	static const int	k_MAX_VALUES = 64;
	static const int	k_MAX_OPERATORS	= 64;

	std::string			basefolder_;
	SourceFile*			sourceFiles_[k_MAX_SOURCEFILES];
	
	Define*				globalDefines_;

	int					numTokens_;

	static Punctuation	default_punctuations[];
	std::vector<Directive*> directives_;
	std::vector<Directive*> dollarDirectives_;
};

struct UI_PrecompilerTools::Punctuation
{
	char*			p_;							//punctuation character(s)
	int				n_;							//punctuation indication
	Punctuation*	next_;						//next punctuation
};

struct UI_PrecompilerTools::Token
{
	static const int	MAX_TOKEN = 1024;

	char				string_[MAX_TOKEN];		//available token
	int					type_;					//last read token type
	int					subtype_;				//last read token sub type
	unsigned long int	intvalue_;				//integer value
	long double			floatvalue_;			//floating point value
	char*				whitespace_p_;			//start of white space before token
	char*				endwhitespace_p_;		//start of white space before token
	int					line_;					//line the token was on
	int					linescrossed_;			//lines crossed in white space
	Token*				next_;					//next token in chain

};

struct UI_PrecompilerTools::PC_Token
{
	static const int	k_MAX_TOKENLENGTH = 1024;

	int					type_;
	int					subtype_;
	int					intvalue_;
	float				floatvalue_;
	char				string_[k_MAX_TOKENLENGTH];
};

struct UI_PrecompilerTools::Define 
{
	char*		name_;							//define name
	int			flags_;							//define flags
	int			builtin_;						// > 0 if builtin define
	int			numparms_;						//number of define parameters
	Token*		parms_;							//define parameters
	Token*		tokens_;						//macro tokens (possibly containing parm tokens)
	Define*		next_;							//next defined macro in a list
	Define*		hashnext_;						//next define in the hash chain
};

struct UI_PrecompilerTools::Indent
{
	int			type_;							//indent type
	int			skip_;							//true if skipping current indent
	Script*		script_;						//script the indent was in
	Indent*		next_;							//next indent on the indent stack
};

struct UI_PrecompilerTools::Script
{
	char			filename_[1024];			//file name of the script
	char*			buffer_;					//buffer containing the script
	char*			script_p_;					//current pointer in the script
	char*			end_p_;						//pointer to the end of the script
	char*			lastscript_p_;				//script pointer before reading token
	char*			whitespace_p_;				//begin of the white space
	char*			endwhitespace_p_;			//end of the white space
	int				length_;					//length of the script in bytes
	int				line_;						//current line in script
	int				lastline_;					//line before reading token
	int				tokenavailable_;			//set by UnreadLastToken
	int				flags_;						//several script flags
	Punctuation*	punctuations_;				//the punctuations used in the script
	Punctuation**	punctuationtable_;
	Token			token_;						//available token
	Script*			next_;						//next script in a chain
};

struct UI_PrecompilerTools::SourceFile
{
	char			filename_[1024];				//file name of the script
	char			includePath_[1024];				//path to include files
	Punctuation*	punctuations_;					//punctuations to use
	Script*			scriptstack_;					//stack with scripts of the source
	Token*			tokens_;						//tokens to read first
	Define*			defines_;						//list with macro definitions
	Define**		definehash_;					//hash chain with defines
	Indent*			indentstack_;					//stack with indents
	int				skip_;							// > 0 if skipping conditional code
	Token			token_;							//last read token
};

struct UI_PrecompilerTools::Operator
{
	int			operator_;
	int			priority_;
	int			parentheses_;
	Operator*	prev_;
	Operator*	next_;
};

struct UI_PrecompilerTools::Value
{
	signed long int	intvalue_;
	double			floatvalue_;
	int				parentheses_;
	Value			*prev_;
	Value			*next_;
};

struct UI_PrecompilerTools::Directive
{
	explicit	Directive( std::string const& name ) : name_(name) {}
	virtual		~Directive() {}

	virtual int	func( SourceFile* source ) = 0;

	static UI_PrecompilerTools* tools_;

	std::string name_;

	enum
	{
		INDENT_IF				= 0x0001,
		INDENT_ELSE				= 0x0002,
		INDENT_ELIF				= 0x0004,
		INDENT_IFDEF			= 0x0008,
		INDENT_IFNDEF			= 0x0010
	};
	static int  if_def( SourceFile* source, int type );

};

struct UI_PrecompilerTools::Directive_if : public UI_PrecompilerTools::Directive
{
				Directive_if() : Directive("if") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_ifdef : public UI_PrecompilerTools::Directive
{
				Directive_ifdef() : Directive("ifdef") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_ifndef : public UI_PrecompilerTools::Directive
{
				Directive_ifndef() : Directive("ifndef") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_elif : public UI_PrecompilerTools::Directive
{
				Directive_elif() : Directive("elif") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_else : public UI_PrecompilerTools::Directive
{
				Directive_else() : Directive("else") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_endif : public UI_PrecompilerTools::Directive
{
				Directive_endif() : Directive("endif") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_include : public UI_PrecompilerTools::Directive
{
				Directive_include() : Directive("include") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_define : public UI_PrecompilerTools::Directive
{
				Directive_define() : Directive("define") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_undef : public UI_PrecompilerTools::Directive
{
				Directive_undef() : Directive("undef") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_line : public UI_PrecompilerTools::Directive
{
				Directive_line() : Directive("line") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_error : public UI_PrecompilerTools::Directive
{
				Directive_error() : Directive("error") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_pragma : public UI_PrecompilerTools::Directive
{
				Directive_pragma() : Directive("pragma") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_eval : public UI_PrecompilerTools::Directive
{
				Directive_eval() : Directive("eval") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::Directive_evalfloat : public UI_PrecompilerTools::Directive
{
				Directive_evalfloat() : Directive("evalfloat") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::DollarDirective_evalint : public UI_PrecompilerTools::Directive
{
				DollarDirective_evalint() : Directive("evalint") {}
	virtual int	func( SourceFile* source );
};

struct UI_PrecompilerTools::DollarDirective_evalfloat : public UI_PrecompilerTools::Directive
{
				DollarDirective_evalfloat() : Directive("evalfloat") {}
	virtual int	func( SourceFile* source );
};


#endif // __UI_PRECOMP_H__
