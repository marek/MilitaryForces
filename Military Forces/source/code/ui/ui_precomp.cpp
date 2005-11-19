#include "ui_precomp.h"
#include "ui_memtools.h"
#include "../game/q_shared.h"



int	FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
int FS_Read2( void *buffer, int len, fileHandle_t f );
void FS_FCloseFile( fileHandle_t f );


UI_PrecompilerTools::Punctuation UI_PrecompilerTools::default_punctuations[] =
{
	//binary operators
	{">>=",P_RSHIFT_ASSIGN, NULL},
	{"<<=",P_LSHIFT_ASSIGN, NULL},
	//
	{"...",P_PARMS, NULL},
	//define merge operator
	{"##",P_PRECOMPMERGE, NULL},
	//logic operators
	{"&&",P_LOGIC_AND, NULL},
	{"||",P_LOGIC_OR, NULL},
	{">=",P_LOGIC_GEQ, NULL},
	{"<=",P_LOGIC_LEQ, NULL},
	{"==",P_LOGIC_EQ, NULL},
	{"!=",P_LOGIC_UNEQ, NULL},
	//arithmatic operators
	{"*=",P_MUL_ASSIGN, NULL},
	{"/=",P_DIV_ASSIGN, NULL},
	{"%=",P_MOD_ASSIGN, NULL},
	{"+=",P_ADD_ASSIGN, NULL},
	{"-=",P_SUB_ASSIGN, NULL},
	{"++",P_INC, NULL},
	{"--",P_DEC, NULL},
	//binary operators
	{"&=",P_BIN_AND_ASSIGN, NULL},
	{"|=",P_BIN_OR_ASSIGN, NULL},
	{"^=",P_BIN_XOR_ASSIGN, NULL},
	{">>",P_RSHIFT, NULL},
	{"<<",P_LSHIFT, NULL},
	//reference operators
	{"->",P_POINTERREF, NULL},
	//C++
	{"::",P_CPP1, NULL},
	{".*",P_CPP2, NULL},
	//arithmatic operators
	{"*",P_MUL, NULL},
	{"/",P_DIV, NULL},
	{"%",P_MOD, NULL},
	{"+",P_ADD, NULL},
	{"-",P_SUB, NULL},
	{"=",P_ASSIGN, NULL},
	//binary operators
	{"&",P_BIN_AND, NULL},
	{"|",P_BIN_OR, NULL},
	{"^",P_BIN_XOR, NULL},
	{"~",P_BIN_NOT, NULL},
	//logic operators
	{"!",P_LOGIC_NOT, NULL},
	{">",P_LOGIC_GREATER, NULL},
	{"<",P_LOGIC_LESS, NULL},
	//reference operator
	{".",P_REF, NULL},
	//seperators
	{",",P_COMMA, NULL},
	{";",P_SEMICOLON, NULL},
	//label indication
	{":",P_COLON, NULL},
	//if statement
	{"?",P_QUESTIONMARK, NULL},
	//embracements
	{"(",P_PARENTHESESOPEN, NULL},
	{")",P_PARENTHESESCLOSE, NULL},
	{"{",P_BRACEOPEN, NULL},
	{"}",P_BRACECLOSE, NULL},
	{"[",P_SQBRACKETOPEN, NULL},
	{"]",P_SQBRACKETCLOSE, NULL},
	//
	{"\\",P_BACKSLASH, NULL},
	//precompiler operator
	{"#",P_PRECOMP, NULL},
#ifdef DOLLAR
	{"$",P_DOLLAR, NULL},
#endif //DOLLAR
	{NULL, 0}
};

UI_PrecompilerTools::UI_PrecompilerTools() :
	basefolder_(""),
	globalDefines_(0),
	numTokens_(0)
{
	Directive::tools_ = this;
	directives_.push_back(new Directive_if);
	directives_.push_back(new Directive_ifdef);
	directives_.push_back(new Directive_ifndef);
	directives_.push_back(new Directive_elif);
	directives_.push_back(new Directive_else);
	directives_.push_back(new Directive_endif);
	directives_.push_back(new Directive_include);
	directives_.push_back(new Directive_define);
	directives_.push_back(new Directive_undef);
	directives_.push_back(new Directive_line);
	directives_.push_back(new Directive_error);
	directives_.push_back(new Directive_pragma);
	directives_.push_back(new Directive_eval);
	directives_.push_back(new Directive_evalfloat);

	dollarDirectives_.push_back(new DollarDirective_evalint);
	dollarDirectives_.push_back(new DollarDirective_evalfloat);

	for( size_t i = 0; i < k_MAX_SOURCEFILES; ++i )
	{
		sourceFiles_[i] = 0;
	}
}

UI_PrecompilerTools::~UI_PrecompilerTools()
{
	for( size_t i = 0; i < directives_.size(); ++i )
	{
		delete directives_[i];
		directives_[i] = 0;
	}
	directives_.clear();

	for( size_t i = 0; i < dollarDirectives_.size(); ++i )
	{
		delete dollarDirectives_[i];
		dollarDirectives_[i] = 0;
	}
	dollarDirectives_.clear();
}

int
UI_PrecompilerTools::loadSourceHandle( const char *filename )
{
	SourceFile* source;
	int i;

	for (i = 1; i < k_MAX_SOURCEFILES; i++)
	{
		if (!sourceFiles_[i])
			break;
	} //end for
	if (i >= k_MAX_SOURCEFILES)
		return 0;
	setBaseFolder("");
	source = loadSourceFile(filename);
	if (!source)
		return 0;
	sourceFiles_[i] = source;
	return i;
}

int 
UI_PrecompilerTools::freeSourceHandle( int handle )
{
	if (handle < 1 || handle >= k_MAX_SOURCEFILES)
		return false;
	if (!sourceFiles_[handle])
		return false;

	freeSourceFile(sourceFiles_[handle]);
	sourceFiles_[handle] = 0;
	return true;
}

UI_PrecompilerTools::SourceFile* 
UI_PrecompilerTools::loadSourceFile( const char *filename )
{
	SourceFile*	source;
	Script*		script;

	script = loadScriptFile(filename);
	if (!script) 
		return NULL;

	script->next_ = NULL;

	source = reinterpret_cast<SourceFile*>(UI_Memtools::getMemory(sizeof(SourceFile)));
	memset(source, 0, sizeof(SourceFile));

	strncpy(source->filename_, filename, MAX_QPATH);
	source->scriptstack_ = script;
	source->tokens_ = NULL;
	source->defines_ = NULL;
	source->indentstack_ = NULL;
	source->skip_ = 0;
	source->definehash_ = reinterpret_cast<Define**>(UI_Memtools::getClearedMemory(k_DEFINEHASHSIZE * sizeof(Define*)));

	addGlobalDefinesToSource(source);
	return source;
}

void 
UI_PrecompilerTools::freeSourceFile( SourceFile* source )
{
	Script* script;
	Token* token;
	Define* define;
	Indent* indent;
	int i;

	//PC_PrintDefineHashTable(source->definehash);
	//free all the scripts
	while(source->scriptstack_)
	{
		script = source->scriptstack_;
		source->scriptstack_ = source->scriptstack_->next_;
		freeScript(script);
	} //end for
	//free all the tokens
	while(source->tokens_)
	{
		token = source->tokens_;
		source->tokens_ = source->tokens_->next_;
		freeToken(token);
	} //end for
	for (i = 0; i < k_DEFINEHASHSIZE; i++)
	{
		while(source->definehash_[i])
		{
			define = source->definehash_[i];
			source->definehash_[i] = source->definehash_[i]->hashnext_;
			freeDefine(define);
		} //end while
	} //end for

	//free all indents
	while(source->indentstack_)
	{
		indent = source->indentstack_;
		source->indentstack_ = source->indentstack_->next_;
		UI_Memtools::freeMemory(indent);
	} //end for
	//
	if (source->definehash_) 
		UI_Memtools::freeMemory(source->definehash_);
	//free the source itself
	UI_Memtools::freeMemory(source);
} 

int
UI_PrecompilerTools::readLine( SourceFile* source, Token* token )
{
	int crossline;

	crossline = 0;
	do
	{
		if (!readSourceToken(source, token)) return false;
		
		if (token->linescrossed_ > crossline)
		{
			unreadSourceToken(source, token);
			return false;
		} //end if
		crossline = 1;
	} while(!strcmp(token->string_, "\\"));

	return true;
}

UI_PrecompilerTools::Script* 
UI_PrecompilerTools::loadScriptFile( const char *filename )
{
	fileHandle_t fp;
	char pathname[MAX_QPATH];
	int length;
	void* buffer;
	Script* script;

	if (basefolder_.size())
		Com_sprintf(pathname, sizeof(pathname), "%s/%s", basefolder_.c_str(), filename);
	else
		Com_sprintf(pathname, sizeof(pathname), "%s", filename);
	length = FS_FOpenFileByMode( pathname, &fp, FS_READ );
	if (!fp) return NULL;

	buffer = UI_Memtools::getClearedMemory(sizeof(Script) + length + 1);
	script = (Script *) buffer;
	Com_Memset(script, 0, sizeof(Script));
	strcpy(script->filename_, filename);
	script->buffer_ = (char *) buffer + sizeof(Script);
	script->buffer_[length] = 0;
	script->length_ = length;
	//pointer in script buffer
	script->script_p_ = script->buffer_;
	//pointer in script buffer before reading token
	script->lastscript_p_ = script->buffer_;
	//pointer to end of script buffer
	script->end_p_ = &script->buffer_[length];
	//set if there's a token available in script->token
	script->tokenavailable_ = 0;
	//
	script->line_ = 1;
	script->lastline_ = 1;
	//
	setScriptPunctuations(script, NULL);
	//
	FS_Read2(script->buffer_, length, fp);
	FS_FCloseFile(fp);
	//
	script->length_ = COM_Compress(script->buffer_);

	return script;
}

void 
UI_PrecompilerTools::freeScript( Script* script )
{
	if (script->punctuationtable_) 
		UI_Memtools::freeMemory( script->punctuationtable_ );
	UI_Memtools::freeMemory( script );
}

int 
UI_PrecompilerTools::endOfScript( Script* script )
{
	return script->script_p_ >= script->end_p_;
}

void
UI_PrecompilerTools::pushScript( SourceFile* source, Script* script )
{
	Script *s;

	for (s = source->scriptstack_; s; s = s->next_)
	{
		if (!Q_stricmp(s->filename_, script->filename_))
		{
			Com_Printf(S_COLOR_RED "ERROR: %s recursively included", script->filename_);
			return;
		} //end if
	} //end for
	//push the script on the script stack
	script->next_ = source->scriptstack_;
	source->scriptstack_ = script;
}

void 
UI_PrecompilerTools::setScriptPunctuations( Script* script, Punctuation* p )
{
	if( p ) 
		createPunctuationTable(script, p);
	else  
		createPunctuationTable(script, default_punctuations);

	if( p ) 
		script->punctuations_ = p;
	else 
		script->punctuations_ = default_punctuations;
}

void 
UI_PrecompilerTools::createPunctuationTable( Script* script, Punctuation* punctuations )
{
	int i;
	Punctuation *p, *lastp, *newp;

	//get memory for the table
	if (!script->punctuationtable_) 
		script->punctuationtable_ = reinterpret_cast<Punctuation**>(UI_Memtools::getMemory(256 * sizeof(Punctuation*)));

	Com_Memset( script->punctuationtable_, 0, 256 * sizeof(Punctuation *) );
	//add the punctuations in the list to the punctuation table
	for (i = 0; punctuations[i].p_; i++)
	{
		newp = &punctuations[i];
		lastp = NULL;
		//sort the punctuations in this table entry on length (longer punctuations first)
		for (p = script->punctuationtable_[(unsigned int) newp->p_[0]]; p; p = p->next_)
		{
			if (strlen(p->p_) < strlen(newp->p_))
			{
				newp->next_ = p;
				if (lastp) 
					lastp->next_ = newp;
				else 
					script->punctuationtable_[(unsigned int) newp->p_[0]] = newp;
				break;
			}
			lastp = p;
		}
		if (!p)
		{
			newp->next_ = NULL;
			if (lastp) 
				lastp->next_ = newp;
			else 
				script->punctuationtable_[(unsigned int) newp->p_[0]] = newp;
		}
	}
}

void 
UI_PrecompilerTools::addGlobalDefinesToSource( SourceFile* source )
{
	Define* define;
	Define* newdefine;

	for (define = globalDefines_; define; define = define->next_)
	{
		newdefine = copyDefine(source, define);

		addDefineToHash(newdefine, source->definehash_);
	}
}

UI_PrecompilerTools::Define* 
UI_PrecompilerTools::copyDefine( SourceFile* source, Define* define )
{
	Define* newdefine;
	Token *token;
	Token* newtoken;
	Token* lasttoken;

	newdefine = reinterpret_cast<Define*>(UI_Memtools::getMemory(sizeof(Define) + strlen(define->name_) + 1));
	//copy the define name
	newdefine->name_ = (char *) newdefine + sizeof(Define);
	strcpy(newdefine->name_, define->name_);
	newdefine->flags_ = define->flags_;
	newdefine->builtin_ = define->builtin_;
	newdefine->numparms_ = define->numparms_;
	//the define is not linked
	newdefine->next_ = 0;
	newdefine->hashnext_ = 0;
	//copy the define tokens
	newdefine->tokens_ = 0;
	for (lasttoken = 0, token = define->tokens_; token; token = token->next_)
	{
		newtoken = copyToken(token);
		newtoken->next_ = 0;
		if (lasttoken) 
			lasttoken->next_ = newtoken;
		else 
			newdefine->tokens_ = newtoken;
		lasttoken = newtoken;
	} //end for
	//copy the define parameters
	newdefine->parms_ = 0;
	for (lasttoken = 0, token = define->parms_; token; token = token->next_)
	{
		newtoken = copyToken(token);
		newtoken->next_ = 0;
		if (lasttoken) 
			lasttoken->next_ = newtoken;
		else 
			newdefine->parms_ = newtoken;
		lasttoken = newtoken;
	} //end for
	return newdefine;
}

void 
UI_PrecompilerTools::addDefineToHash( Define* define, Define** definehash )
{
	int hash;

	hash = nameHash(define->name_);
	define->hashnext_ = definehash[hash];
	definehash[hash] = define;
}

void 
UI_PrecompilerTools::freeDefine( Define* define )
{
	Token* t;
	Token* next;

	//free the define parameters
	for (t = define->parms_; t; t = next)
	{
		next = t->next_;
		freeToken(t);
	} 

	for (t = define->tokens_; t; t = next)
	{
		next = t->next_;
		freeToken(t);
	} 
	UI_Memtools::freeMemory( define );
}

UI_PrecompilerTools::Define* 
UI_PrecompilerTools::findHashedDefine( Define** definehash, char *name )
{
	Define* d;
	int hash;

	hash = nameHash(name);
	for( d = definehash[hash]; d; d = d->hashnext_ )
	{
		if (!strcmp(d->name_, name)) 
			return d;
	}
	return NULL;
}

int 
UI_PrecompilerTools::expandDefineIntoSource( SourceFile* source, Token* deftoken, Define* define )
{
	Token *firsttoken, *lasttoken;

	if (!expandDefine(source, deftoken, define, &firsttoken, &lasttoken)) 
		return false;

	if (firsttoken && lasttoken)
	{
		lasttoken->next_ = source->tokens_;
		source->tokens_ = firsttoken;
		return true;
	} //end if
	return false;
}

int 
UI_PrecompilerTools::expandDefine( SourceFile* source, Token* deftoken, Define* define,
								   Token** firsttoken, Token** lasttoken )
{
	Token *parms[k_MAX_DEFINEPARMS], *dt, *pt, *t;
	Token *t1, *t2, *first, *last, *nextpt, token;
	int parmnum, i;

	//if it is a builtin define
	if (define->builtin_)
	{
		return expandBuiltinDefine(source, deftoken, define, firsttoken, lasttoken);
	} //end if
	//if the define has parameters
	if (define->numparms_)
	{
		if (!readDefineParms(source, define, parms, k_MAX_DEFINEPARMS)) 
			return false;
	} //end if
	//empty list at first
	first = NULL;
	last = NULL;
	//create a list with tokens of the expanded define
	for (dt = define->tokens_; dt; dt = dt->next_)
	{
		parmnum = -1;
		//if the token is a name, it could be a define parameter
		if (dt->type_ == TT_NAME)
		{
			parmnum = findDefineParm(define, dt->string_);
		} //end if
		//if it is a define parameter
		if (parmnum >= 0)
		{
			for (pt = parms[parmnum]; pt; pt = pt->next_)
			{
				t = copyToken(pt);
				//add the token to the list
				t->next_ = NULL;
				if (last) 
					last->next_ = t;
				else 
					first = t;
				last = t;
			} //end for
		} //end if
		else
		{
			//if stringizing operator
			if (dt->string_[0] == '#' && dt->string_[1] == '\0')
			{
				//the stringizing operator must be followed by a define parameter
				if (dt->next_) 
					parmnum = findDefineParm(define, dt->next_->string_);
				else 
					parmnum = -1;
				//
				if (parmnum >= 0)
				{
					//step over the stringizing operator
					dt = dt->next_;
					//stringize the define parameter tokens
					if (!stringizeTokens(parms[parmnum], &token))
					{
						Com_Printf(S_COLOR_RED "ERROR: can't stringize tokens");
						return false;
					} //end if
					t = copyToken(&token);
				} //end if
				else
				{
					Com_Printf(S_COLOR_YELLOW "stringizing operator without define parameter");
					continue;
				} //end if
			} //end if
			else
			{
				t = copyToken(dt);
			} //end else
			//add the token to the list
			t->next_ = NULL;
			if (last) 
				last->next_ = t;
			else
				first = t;
			last = t;
		} //end else
	} //end for
	//check for the merging operator
	for (t = first; t; )
	{
		if (t->next_)
		{
			//if the merging operator
			if (t->next_->string_[0] == '#' && t->next_->string_[1] == '#')
			{
				t1 = t;
				t2 = t->next_->next_;
				if (t2)
				{
					if (!mergeTokens(t1, t2))
					{
						Com_Printf(S_COLOR_RED "ERROR: can't merge %s with %s", t1->string_, t2->string_);
						return false;
					} //end if
					freeToken(t1->next_);
					t1->next_ = t2->next_;
					if (t2 == last) 
						last = t1;
					freeToken(t2);
					continue;
				} //end if
			} //end if
		} //end if
		t = t->next_;
	} //end for
	//store the first and last token of the list
	*firsttoken = first;
	*lasttoken = last;
	//free all the parameter tokens
	for (i = 0; i < define->numparms_; i++)
	{
		for (pt = parms[i]; pt; pt = nextpt)
		{
			nextpt = pt->next_;
			freeToken(pt);
		} //end for
	} //end for
	//
	return true;
}

int 
UI_PrecompilerTools::expandBuiltinDefine( SourceFile* source, Token* deftoken, Define* define,
										  Token** firsttoken, Token** lasttoken )
{
	Token *token;
	time_t t;	
	char *curtime;

	token = copyToken(deftoken);
	switch(define->builtin_)
	{
		case BUILTIN_LINE:
		{
			sprintf(token->string_, "%d", deftoken->line_);
			token->intvalue_ = deftoken->line_;
			token->floatvalue_ = deftoken->line_;
			token->type_ = TT_NUMBER;
			token->subtype_ = TT_DECIMAL | TT_INTEGER;
			*firsttoken = token;
			*lasttoken = token;
			break;
		} //end case
		case BUILTIN_FILE:
		{
			strcpy(token->string_, source->scriptstack_->filename_);
			token->type_ = TT_NAME;
			token->subtype_ = strlen(token->string_);
			*firsttoken = token;
			*lasttoken = token;
			break;
		} //end case
		case BUILTIN_DATE:
		{
			t = time(NULL);
			curtime = ctime(&t);
			strcpy(token->string_, "\"");
			strncat(token->string_, curtime+4, 7);
			strncat(token->string_+7, curtime+20, 4);
			strcat(token->string_, "\"");
			free(curtime);
			token->type_ = TT_NAME;
			token->subtype_ = strlen(token->string_);
			*firsttoken = token;
			*lasttoken = token;
			break;
		} //end case
		case BUILTIN_TIME:
		{
			t = time(NULL);
			curtime = ctime(&t);
			strcpy(token->string_, "\"");
			strncat(token->string_, curtime+11, 8);
			strcat(token->string_, "\"");
			free(curtime);
			token->type_ = TT_NAME;
			token->subtype_ = strlen(token->string_);
			*firsttoken = token;
			*lasttoken = token;
			break;
		} //end case
		case BUILTIN_STDC:
		default:
		{
			*firsttoken = NULL;
			*lasttoken = NULL;
			break;
		} //end case
	} //end switch
	return true;
} 

int 
UI_PrecompilerTools::readDefineParms( SourceFile* source, Define* define, Token** parms, int maxparms )
{
	Token token, *t, *last;
	int i, done, lastcomma, numparms, indent;

	if (!readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: define %s missing parms", define->name_);
		return false;
	} //end if
	//
	if (define->numparms_ > maxparms)
	{
		Com_Printf(S_COLOR_RED "ERROR: define with more than %d parameters", maxparms);
		return false;
	} //end if
	//
	for (i = 0; i < define->numparms_; i++) parms[i] = NULL;
	//if no leading "("
	if (strcmp(token.string_, "("))
	{
		unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: define %s missing parms", define->name_);
		return false;
	} //end if
	//read the define parameters
	for (done = 0, numparms = 0, indent = 0; !done;)
	{
		if (numparms >= maxparms)
		{
			Com_Printf(S_COLOR_RED "ERROR: define %s with too many parms", define->name_);
			return false;
		} //end if
		if (numparms >= define->numparms_)
		{
			Com_Printf(S_COLOR_YELLOW "WARNING: define %s has too many parms", define->name_);
			return false;
		} //end if
		parms[numparms] = NULL;
		lastcomma = 1;
		last = NULL;
		while(!done)
		{
			//
			if (!readSourceToken(source, &token))
			{
				Com_Printf(S_COLOR_RED "ERROR: define %s incomplete", define->name_);
				return false;
			} //end if
			//
			if (!strcmp(token.string_, ","))
			{
				if (indent <= 0)
				{
					if (lastcomma) 
						Com_Printf(S_COLOR_YELLOW "WARNING: too many comma's");
					lastcomma = 1;
					break;
				} //end if
			} //end if
			lastcomma = 0;
			//
			if (!strcmp(token.string_, "("))
			{
				indent++;
				continue;
			} //end if
			else if (!strcmp(token.string_, ")"))
			{
				if (--indent <= 0)
				{
					if (!parms[define->numparms_-1])
					{
						Com_Printf(S_COLOR_YELLOW "WARNING: too few define parms");
					} //end if
					done = 1;
					break;
				} //end if
			} //end if
			//
			if (numparms < define->numparms_)
			{
				//
				t = copyToken(&token);
				t->next_ = NULL;
				if (last) 
					last->next_ = t;
				else 
					parms[numparms] = t;
				last = t;
			} //end if
		} //end while
		numparms++;
	} //end for
	return true;
}

int 
UI_PrecompilerTools::findDefineParm( Define *define, char *name )
{
	Token *p;
	int i;

	i = 0;
	for (p = define->parms_; p; p = p->next_)
	{
		if (!strcmp(p->string_, name)) 
			return i;
		i++;
	} //end for
	return -1;
}

int 
UI_PrecompilerTools::readTokenHandle( int handle, PC_Token* pc_token )
{
	Token token;
	int ret;

	if (handle < 1 || handle >= k_MAX_SOURCEFILES)
		return 0;
	if (!sourceFiles_[handle])
		return 0;

	ret = readToken(sourceFiles_[handle], &token);
	strcpy(pc_token->string_, token.string_);
	pc_token->type_ = token.type_;
	pc_token->subtype_ = token.subtype_;
	pc_token->intvalue_ = token.intvalue_;
	pc_token->floatvalue_ = token.floatvalue_;
	if (pc_token->type_ == TT_STRING)
		stripDoubleQuotes(pc_token->string_);
	return ret;
} 

int 
UI_PrecompilerTools::scriptReadToken( Script* script, Token* token )
{
	//if there is a token available (from UnreadToken)
	if (script->tokenavailable_)
	{
		script->tokenavailable_ = 0;
		Com_Memcpy(token, &script->token_, sizeof(Token));
		return 1;
	} //end if
	//save script pointer
	script->lastscript_p_ = script->script_p_;
	//save line counter
	script->lastline_ = script->line_;
	//clear the token stuff
	Com_Memset(token, 0, sizeof(Token));
	//start of the white space
	script->whitespace_p_ = script->script_p_;
	token->whitespace_p_ = script->script_p_;
	//read unusefull stuff
	if (!scriptReadWhiteSpace(script)) 
		return 0;
	//end of the white space
	script->endwhitespace_p_ = script->script_p_;
	token->endwhitespace_p_ = script->script_p_;
	//line the token is on
	token->line_ = script->line_;
	//number of lines crossed before token
	token->linescrossed_ = script->line_ - script->lastline_;
	//if there is a leading double quote
	if (*script->script_p_ == '\"')
	{
		if (!scriptReadString(script, token, '\"')) 
			return 0;
	} //end if
	//if an literal
	else if (*script->script_p_ == '\'')
	{
		//if (!PS_ReadLiteral(script, token)) return 0;
		if (!scriptReadString(script, token, '\'')) 
			return 0;
	} //end if
	//if there is a number
	else if ((*script->script_p_ >= '0' && *script->script_p_ <= '9') ||
				(*script->script_p_ == '.' &&
				(*(script->script_p_ + 1) >= '0' && *(script->script_p_ + 1) <= '9')))
	{
		if (!scriptReadNumber(script, token)) 
			return 0;
	} //end if
	//if this is a primitive script
	else if (script->flags_ & SCFL_PRIMITIVE)
	{
		return scriptReadPrimitive(script, token);
	} //end else if
	//if there is a name
	else if ((*script->script_p_ >= 'a' && *script->script_p_ <= 'z') ||
		(*script->script_p_ >= 'A' && *script->script_p_ <= 'Z') ||
		*script->script_p_ == '_')
	{
		if (!scriptReadName(script, token)) 
			return 0;
	} //end if
	//check for punctuations
	else if (!scriptReadPunctuation(script, token))
	{
		Com_Printf(S_COLOR_RED "ERROR: can't read token");
		return 0;
	} //end if
	//copy the token into the script structure
	Com_Memcpy(&script->token_, token, sizeof(Token));
	//succesfully read a token
	return 1;
}

int 
UI_PrecompilerTools::scriptReadWhiteSpace( Script* script )
{
	while(1)
	{
		//skip white space
		while(*script->script_p_ <= ' ')
		{
			if (!*script->script_p_) 
				return 0;
			if (*script->script_p_ == '\n') 
				script->line_++;
			script->script_p_++;
		} //end while
		//skip comments
		if (*script->script_p_ == '/')
		{
			//comments //
			if (*(script->script_p_+1) == '/')
			{
				script->script_p_++;
				do
				{
					script->script_p_++;
					if (!*script->script_p_) 
						return 0;
				} //end do
				while(*script->script_p_ != '\n');
				script->line_++;
				script->script_p_++;
				if (!*script->script_p_) 
					return 0;
				continue;
			} //end if
			//comments /* */
			else if (*(script->script_p_+1) == '*')
			{
				script->script_p_++;
				do
				{
					script->script_p_++;
					if (!*script->script_p_) 
						return 0;
					if (*script->script_p_ == '\n') 
						script->line_++;
				} //end do
				while(!(*script->script_p_ == '*' && *(script->script_p_+1) == '/'));
				script->script_p_++;
				if (!*script->script_p_) 
					return 0;
				script->script_p_++;
				if (!*script->script_p_) 
					return 0;
				continue;
			} //end if
		} //end if
		break;
	} //end while
	return 1;
}

int 
UI_PrecompilerTools::scriptReadString( Script* script, Token* token, int quote )
{
	int len, tmpline;
	char *tmpscript_p;

	if (quote == '\"') 
		token->type_ = TT_STRING;
	else 
		token->type_ = TT_LITERAL;

	len = 0;
	//leading quote
	token->string_[len++] = *script->script_p_++;
	//
	while(1)
	{
		//minus 2 because trailing double quote and zero have to be appended
		if (len >= k_MAX_TOKEN - 2)
		{
			Com_Printf(S_COLOR_RED "ERROR: string longer than MAX_TOKEN = %d", k_MAX_TOKEN);
			return 0;
		} //end if
		//if there is an escape character and
		//if escape characters inside a string are allowed
		if (*script->script_p_ == '\\' && !(script->flags_ & SCFL_NOSTRINGESCAPECHARS))
		{
			if (!scriptReadEscapeCharacter(script, &token->string_[len]))
			{
				token->string_[len] = 0;
				return 0;
			} //end if
			len++;
		} //end if
		//if a trailing quote
		else if (*script->script_p_ == quote)
		{
			//step over the double quote
			script->script_p_++;
			//if white spaces in a string are not allowed
			if (script->flags_ & SCFL_NOSTRINGWHITESPACES) 
				break;
			//
			tmpscript_p = script->script_p_;
			tmpline = script->line_;
			//read unusefull stuff between possible two following strings
			if (!scriptReadWhiteSpace(script))
			{
				script->script_p_ = tmpscript_p;
				script->line_ = tmpline;
				break;
			} //end if
			//if there's no leading double qoute
			if (*script->script_p_ != quote)
			{
				script->script_p_ = tmpscript_p;
				script->line_ = tmpline;
				break;
			} //end if
			//step over the New leading double quote
			script->script_p_++;
		} //end if
		else
		{
			if (*script->script_p_ == '\0')
			{
				token->string_[len] = 0;
				Com_Printf(S_COLOR_RED "ERROR: missing trailing quote");
				return 0;
			} //end if
	      if (*script->script_p_ == '\n')
			{
				token->string_[len] = 0;
				Com_Printf(S_COLOR_RED "ERROR: newline inside string %s", token->string_);
				return 0;
			} //end if
			token->string_[len++] = *script->script_p_++;
		} //end else
	} //end while
	//trailing quote
	token->string_[len++] = quote;
	//end string with a zero
	token->string_[len] = '\0';
	//the sub type is the length of the string
	token->subtype_ = len;
	return 1;
}

int 
UI_PrecompilerTools::scriptReadNumber( Script* script, Token* token )
{
	int len = 0, i;
	int octal, dot;
	char c;
//	unsigned long int intvalue = 0;
//	long double floatvalue = 0;

	token->type_ = TT_NUMBER;
	//check for a hexadecimal number
	if (*script->script_p_ == '0' &&
		(*(script->script_p_ + 1) == 'x' ||
		*(script->script_p_ + 1) == 'X'))
	{
		token->string_[len++] = *script->script_p_++;
		token->string_[len++] = *script->script_p_++;
		c = *script->script_p_;
		//hexadecimal
		while((c >= '0' && c <= '9') ||
					(c >= 'a' && c <= 'f') ||
					(c >= 'A' && c <= 'A'))
		{
			token->string_[len++] = *script->script_p_++;
			if (len >= k_MAX_TOKEN)
			{
				Com_Printf(S_COLOR_RED "ERROR: hexadecimal number longer than MAX_TOKEN = %d", k_MAX_TOKEN);
				return 0;
			} //end if
			c = *script->script_p_;
		} //end while
		token->subtype_ |= TT_HEX;
	} //end if
	//check for a binary number
	else if (*script->script_p_ == '0' &&
		(*(script->script_p_ + 1) == 'b' ||
		*(script->script_p_ + 1) == 'B'))
	{
		token->string_[len++] = *script->script_p_++;
		token->string_[len++] = *script->script_p_++;
		c = *script->script_p_;
		//binary
		while(c == '0' || c == '1')
		{
			token->string_[len++] = *script->script_p_++;
			if (len >= k_MAX_TOKEN)
			{
				Com_Printf(S_COLOR_RED "ERROR: binary number longer than MAX_TOKEN = %d", k_MAX_TOKEN);
				return 0;
			} //end if
			c = *script->script_p_;
		} //end while
		token->subtype_ |= TT_BINARY;
	} //end if
	else //decimal or octal integer or floating point number
	{
		octal = false;
		dot = false;
		if (*script->script_p_ == '0') octal = true;
		while(1)
		{
			c = *script->script_p_;
			if (c == '.') 
				dot = true;
			else if (c == '8' || c == '9') 
				octal = false;
			else if (c < '0' || c > '9') 
				break;
			token->string_[len++] = *script->script_p_++;
			if (len >= k_MAX_TOKEN - 1)
			{
				Com_Printf(S_COLOR_RED "ERROR: number longer than MAX_TOKEN = %d", k_MAX_TOKEN);
				return 0;
			} //end if
		} //end while
		if (octal) 
			token->subtype_ |= TT_OCTAL;
		else 
			token->subtype_ |= TT_DECIMAL;
		if (dot) 
			token->subtype_ |= TT_FLOAT;
	} //end else
	for (i = 0; i < 2; i++)
	{
		c = *script->script_p_;
		//check for a LONG number
		if ( (c == 'l' || c == 'L') // bk001204 - brackets 
		     && !(token->subtype_ & TT_LONG))
		{
			script->script_p_++;
			token->subtype_ |= TT_LONG;
		} //end if
		//check for an UNSIGNED number
		else if ( (c == 'u' || c == 'U') // bk001204 - brackets 
			  && !(token->subtype_ & (TT_UNSIGNED | TT_FLOAT)))
		{
			script->script_p_++;
			token->subtype_ |= TT_UNSIGNED;
		} //end if
	} //end for
	token->string_[len] = '\0';
	numberValue(token->string_, token->subtype_, &token->intvalue_, &token->floatvalue_);
	if (!(token->subtype_ & TT_FLOAT)) 
		token->subtype_ |= TT_INTEGER;
	return 1;
}

int 
UI_PrecompilerTools::scriptReadPrimitive( Script* script, Token* token )
{
	int len;

	len = 0;
	while(*script->script_p_ > ' ' && *script->script_p_ != ';')
	{
		if (len >= k_MAX_TOKEN)
		{
			Com_Printf(S_COLOR_RED "ERROR: primitive token longer than MAX_TOKEN = %d", k_MAX_TOKEN);
			return 0;
		} //end if
		token->string_[len++] = *script->script_p_++;
	} //end while
	token->string_[len] = 0;
	//copy the token into the script structure
	Com_Memcpy(&script->token_, token, sizeof(Token));
	//primitive reading successfull
	return 1;
}

int
UI_PrecompilerTools::scriptReadName( Script* script, Token* token )
{
	int len = 0;
	char c;

	token->type_ = TT_NAME;
	do
	{
		token->string_[len++] = *script->script_p_++;
		if (len >= k_MAX_TOKEN)
		{
			Com_Printf(S_COLOR_RED "ERROR: name longer than MAX_TOKEN = %d", k_MAX_TOKEN);
			return 0;
		} //end if
		c = *script->script_p_;
   } while ((c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') ||
				c == '_');
	token->string_[len] = '\0';
	//the sub type is the length of the name
	token->subtype_ = len;
	return 1;
}

int 
UI_PrecompilerTools::scriptReadPunctuation( Script* script, Token* token )
{
	int len;
	char *p;
	Punctuation* punc;

	for (punc = script->punctuationtable_[(unsigned int)*script->script_p_]; punc; punc = punc->next_)
	{
		p = punc->p_;
		len = strlen(p);
		//if the script contains at least as much characters as the punctuation
		if (script->script_p_ + len <= script->end_p_)
		{
			//if the script contains the punctuation
			if (!strncmp(script->script_p_, p, len))
			{
				strncpy(token->string_, p, k_MAX_TOKEN);
				script->script_p_ += len;
				token->type_ = TT_PUNCTUATION;
				//sub type is the number of the punctuation
				token->subtype_ = punc->n_;
				return 1;
			}
		}
	}
	return 0;
}

int 
UI_PrecompilerTools::scriptReadEscapeCharacter( Script* script, char *ch )
{
	int c, val, i;

	//step over the leading '\\'
	script->script_p_++;
	//determine the escape character
	switch(*script->script_p_)
	{
		case '\\': c = '\\'; 
			break;
		case 'n': c = '\n'; 
			break;
		case 'r': c = '\r'; 
			break;
		case 't': c = '\t'; 
			break;
		case 'v': c = '\v'; 
			break;
		case 'b': c = '\b'; 
			break;
		case 'f': c = '\f'; 
			break;
		case 'a': c = '\a'; 
			break;
		case '\'': c = '\''; 
			break;
		case '\"': c = '\"'; 
			break;
		case '\?': c = '\?'; 
			break;
		case 'x':
		{
			script->script_p_++;
			for (i = 0, val = 0; ; i++, script->script_p_++)
			{
				c = *script->script_p_;
				if (c >= '0' && c <= '9') 
					c = c - '0';
				else if (c >= 'A' && c <= 'Z') 
					
					c = c - 'A' + 10;
				else if (c >= 'a' && c <= 'z') 
					c = c - 'a' + 10;
				else 
					break;
				val = (val << 4) + c;
			} //end for
			script->script_p_--;
			if (val > 0xFF)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: too large value in escape character");
				val = 0xFF;
			} //end if
			c = val;
			break;
		} //end case
		default: //NOTE: decimal ASCII code, NOT octal
		{
			if (*script->script_p_ < '0' || *script->script_p_ > '9') 
				Com_Printf(S_COLOR_RED "ERROR: unknown escape char");

			for (i = 0, val = 0; ; i++, script->script_p_++)
			{
				c = *script->script_p_;
				if (c >= '0' && c <= '9') c = c - '0';
				else break;
				val = val * 10 + c;
			} //end for
			script->script_p_--;
			if (val > 0xFF)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: too large value in escape character");
				val = 0xFF;
			} //end if
			c = val;
			break;
		} //end default
	} //end switch
	//step over the escape character or the last digit of the number
	script->script_p_++;
	//store the escape character
	*ch = c;
	//succesfully read escape character
	return 1;
}

int 
UI_PrecompilerTools::readSourceToken( SourceFile* source, Token* token )
{
	Token* t;
	Script* script;
	int type;
	int skip;

	//if there's no token already available
	while(!source->tokens_)
	{
		//if there's a token to read from the script
		if (scriptReadToken(source->scriptstack_, token)) 
			return true;
		//if at the end of the script
		if (endOfScript(source->scriptstack_))
		{
			//remove all indents of the script
			while(source->indentstack_ &&
					source->indentstack_->script_ == source->scriptstack_)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: missing #endif");
				popIndent(source, &type, &skip);
			} //end if
		} //end if
		//if this was the initial script
		if (!source->scriptstack_->next_) 
			return false;
		//remove the script and return to the last one
		script = source->scriptstack_;
		source->scriptstack_ = source->scriptstack_->next_;
		freeScript(script);
	} //end while
	//copy the already available token
	Com_Memcpy(token, source->tokens_, sizeof(Token));
	//free the read token
	t = source->tokens_;
	source->tokens_ = source->tokens_->next_;
	freeToken(t);
	return true;
}

int 
UI_PrecompilerTools::unreadSourceToken( SourceFile* source, Token* token )
{
	Token *t;

	t = copyToken(token);
	t->next_ = source->tokens_;
	source->tokens_ = t;
	return true;
} 

int 
UI_PrecompilerTools::readDirective( SourceFile* source )
{
	Token token;

	//read the directive name
	if (!readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: found # without name");
		return false;
	}
	//directive name must be on the same line
	if (token.linescrossed_ > 0)
	{
		unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: found # at end of line");
		return false;
	}
	//if if is a name
	if (token.type_ == TT_NAME)
	{
		//find the precompiler directive
		for (size_t i = 0; directives_.size(); ++i)
		{
			if (!strcmp(directives_[i]->name_.c_str(), token.string_))
			{
				return directives_[i]->func(source);
			}
		}
	}
	Com_Printf(S_COLOR_RED "ERROR: unknown precompiler directive %s", token.string_);
	return false;
} 

int 
UI_PrecompilerTools::readDollarDirective( SourceFile* source )
{
	Token token;
	int i;

	//read the directive name
	if (!readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: found $ without name");
		return false;
	} //end if
	//directive name must be on the same line
	if (token.linescrossed_ > 0)
	{
		unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: found $ at end of line");
		return false;
	} //end if
	//if if is a name
	if (token.type_ == TT_NAME)
	{
		//find the precompiler directive
		for (i = 0; i < dollarDirectives_.size(); i++)
		{
			if (!strcmp(dollarDirectives_[i]->name_.c_str(), token.string_))
			{
				return dollarDirectives_[i]->func(source);
			} //end if
		} //end for
	} //end if
	unreadSourceToken(source, &token);
	Com_Printf(S_COLOR_RED "ERROR: unknown precompiler directive %s", token.string_);
	return false;
}

int 
UI_PrecompilerTools::readToken( SourceFile* source, Token* token )
{
	Define* define;

	while(1)
	{
		if (!readSourceToken(source, token)) 
			return false;
		//check for precompiler directives
		if (token->type_ == TT_PUNCTUATION && *token->string_ == '#')
		{
			{
				//read the precompiler directive
				if (!readDirective(source)) 
					return false;
				continue;
			} //end if
		} //end if
		if (token->type_ == TT_PUNCTUATION && *token->string_ == '$')
		{
			{
				//read the precompiler directive
				if (!readDollarDirective(source)) 
					return false;
				continue;
			} //end if
		} //end if
		// recursively concatenate strings that are behind each other still resolving defines
		if (token->type_ == TT_STRING)
		{
			Token newtoken;
			if (readToken(source, &newtoken))
			{
				if (newtoken.type_ == TT_STRING)
				{
					token->string_[strlen(token->string_)-1] = '\0';
					if (strlen(token->string_) + strlen(newtoken.string_+1) + 1 >= k_MAX_TOKEN)
					{
						Com_Printf(S_COLOR_RED "ERROR: string longer than MAX_TOKEN = %d", k_MAX_TOKEN);
						return false;
					}
					strcat(token->string_, newtoken.string_+1);
				}
				else
				{
					unreadSourceToken(source, &newtoken);
				}
			}
		} //end if
		//if skipping source because of conditional compilation
		if (source->skip_) continue;
		//if the token is a name
		if (token->type_ == TT_NAME)
		{
			//check if the name is a define macro
			define = findHashedDefine(source->definehash_, token->string_);
			//if it is a define macro
			if (define)
			{
				//expand the defined macro
				if (!expandDefineIntoSource(source, token, define)) 
					return false;
				continue;
			} //end if
		}
		//copy token for unreading
		Com_Memcpy(&source->token_, token, sizeof(Token));
		//found a token
		return true;
	}
}

UI_PrecompilerTools::Token* 
UI_PrecompilerTools::copyToken( Token* token )
{
	Token *t;

	t = reinterpret_cast<Token*>(UI_Memtools::getMemory(sizeof(Token)));
	if (!t)
	{
		Com_Error(ERR_FATAL, "out of token space\n");
		return NULL;
	}
	Com_Memcpy(t, token, sizeof(Token));
	t->next_ = 0;
	numTokens_++;
	return t;
}

void
UI_PrecompilerTools::freeToken( Token* token )
{
	UI_Memtools::freeMemory(token);
	numTokens_--;
}

int 
UI_PrecompilerTools::stringizeTokens( Token* tokens, Token* token )
{
	Token *t;

	token->type_ = TT_STRING;
	token->whitespace_p_ = NULL;
	token->endwhitespace_p_ = NULL;
	token->string_[0] = '\0';
	strcat(token->string_, "\"");
	for (t = tokens; t; t = t->next_)
	{
		strncat(token->string_, t->string_, k_MAX_TOKEN - strlen(token->string_));
	} //end for
	strncat(token->string_, "\"", k_MAX_TOKEN - strlen(token->string_));
	return true;
}

int 
UI_PrecompilerTools::mergeTokens( Token* t1, Token* t2 )
{
	//merging of a name with a name or number
	if (t1->type_ == TT_NAME && (t2->type_ == TT_NAME || t2->type_ == TT_NUMBER))
	{
		strcat(t1->string_, t2->string_);
		return true;
	} //end if
	//merging of two strings
	if (t1->type_ == TT_STRING && t2->type_ == TT_STRING)
	{
		//remove trailing double quote
		t1->string_[strlen(t1->string_)-1] = '\0';
		//concat without leading double quote
		strcat(t1->string_, &t2->string_[1]);
		return true;
	} //end if
	//FIXME: merging of two number of the same sub type
	return false;
}

void 
UI_PrecompilerTools::popIndent( SourceFile* source, int *type, int *skip )
{
	Indent* indent;

	*type = 0;
	*skip = 0;

	indent = source->indentstack_;
	if (!indent) 
		return;

	//must be an indent from the current script
	if (source->indentstack_->script_ != source->scriptstack_) 
		return;

	*type = indent->type_;
	*skip = indent->skip_;
	source->indentstack_ = source->indentstack_->next_;
	source->skip_ -= indent->skip_;
	UI_Memtools::freeMemory(indent);
}

void 
UI_PrecompilerTools::pushIndent( SourceFile* source, int type, int skip )
{
	Indent* indent;

	indent = reinterpret_cast<Indent*>(UI_Memtools::getMemory(sizeof(Indent)));
	indent->type_ = type;
	indent->script_ = source->scriptstack_;
	indent->skip_ = (skip != 0);
	source->skip_ += indent->skip_;
	indent->next_ = source->indentstack_;
	source->indentstack_ = indent;
}

int 
UI_PrecompilerTools::evaluate( SourceFile *source, signed long int *intvalue,
							   double *floatvalue, int integer )
{
	Token token, *firsttoken, *lasttoken;
	Token *t, *nexttoken;
	Define *define;
	int defined = false;

	if (intvalue) 
		*intvalue = 0;
	if (floatvalue) 
		*floatvalue = 0;
	//
	if (!readLine(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: no value after #if/#elif");
		return false;
	} //end if
	firsttoken = NULL;
	lasttoken = NULL;
	do
	{
		//if the token is a name
		if (token.type_ == TT_NAME)
		{
			if (defined)
			{
				defined = false;
				t = copyToken(&token);
				t->next_ = NULL;
				if (lasttoken) 
					lasttoken->next_ = t;
				else 
					firsttoken = t;
				lasttoken = t;
			} //end if
			else if (!strcmp(token.string_, "defined"))
			{
				defined = true;
				t = copyToken(&token);
				t->next_ = NULL;
				if (lasttoken) 
					lasttoken->next_ = t;
				else 
					firsttoken = t;
				lasttoken = t;
			} //end if
			else
			{
				//then it must be a define
				define = findHashedDefine(source->definehash_, token.string_);

				if (!define)
				{
					Com_Printf(S_COLOR_RED "ERROR: can't evaluate %s, not defined", token.string_);
					return false;
				} //end if
				if (!expandDefineIntoSource(source, &token, define)) return false;
			} //end else
		} //end if
		//if the token is a number or a punctuation
		else if (token.type_ == TT_NUMBER || token.type_ == TT_PUNCTUATION)
		{
			t = copyToken(&token);
			t->next_ = NULL;
			if (lasttoken) 
				lasttoken->next_ = t;
			else 
				firsttoken = t;
			lasttoken = t;
		} //end else
		else //can't evaluate the token
		{
			Com_Printf(S_COLOR_RED "ERROR: can't evaluate %s", token.string_);
			return false;
		} //end else
	} while(readLine(source, &token));
	//
	if (!evaluateTokens(source, firsttoken, intvalue, floatvalue, integer)) 
		return false;
	//
	for (t = firsttoken; t; t = nexttoken)
	{
		nexttoken = t->next_;
		freeToken(t);
	} //end for
	//
	return true;
} 

int 
UI_PrecompilerTools::evaluateTokens( SourceFile* source, Token* tokens, signed long int *intvalue,
									 double *floatvalue, int integer )
{
	Operator *o, *firstoperator, *lastoperator;
	Value *v, *firstvalue, *lastvalue, *v1, *v2;
	Token *t;
	int brace = 0;
	int parentheses = 0;
	int error = 0;
	int lastwasvalue = 0;
	int negativevalue = 0;
	int questmarkintvalue = 0;
	double questmarkfloatvalue = 0;
	int gotquestmarkvalue = false;
	int lastoperatortype = 0;
	//
	Operator operator_heap[k_MAX_OPERATORS];
	int numoperators = 0;
	Value value_heap[k_MAX_VALUES];
	int numvalues = 0;

	firstoperator = lastoperator = NULL;
	firstvalue = lastvalue = NULL;
	if (intvalue) 
		*intvalue = 0;
	if (floatvalue) 
		*floatvalue = 0;
	for (t = tokens; t; t = t->next_)
	{
		switch(t->type_)
		{
			case TT_NAME:
			{
				if (lastwasvalue || negativevalue)
				{
					Com_Printf(S_COLOR_RED "ERROR: syntax error in #if/#elif");
					error = 1;
					break;
				} //end if
				if (strcmp(t->string_, "defined"))
				{
					Com_Printf(S_COLOR_RED "ERROR: undefined name %s in #if/#elif", t->string_);
					error = 1;
					break;
				} //end if
				t = t->next_;
				if (!strcmp(t->string_, "("))
				{
					brace = true;
					t = t->next_;
				} //end if
				if (!t || t->type_ != TT_NAME)
				{
					Com_Printf(S_COLOR_RED "ERROR: defined without name in #if/#elif");
					error = 1;
					break;
				} //end if
				//v = (value_t *) GetClearedMemory(sizeof(value_t));
				// alloc value
				if (numvalues >= k_MAX_VALUES) 
				{
					Com_Printf(S_COLOR_RED "ERROR:out of value space\n");		
					error = 1;										
					break;											
				}													
				else												
					v = &value_heap[numvalues++];

				if (findHashedDefine(source->definehash_, t->string_))
				{
					v->intvalue_ = 1;
					v->floatvalue_ = 1;
				} //end if
				else
				{
					v->intvalue_ = 0;
					v->floatvalue_ = 0;
				} //end else
				v->parentheses_ = parentheses;
				v->next_ = NULL;
				v->prev_ = lastvalue;
				if (lastvalue) 
					lastvalue->next_ = v;
				else 
					firstvalue = v;
				lastvalue = v;
				if (brace)
				{
					t = t->next_;
					if (!t || strcmp(t->string_, ")"))
					{
						Com_Printf(S_COLOR_RED "ERROR: defined without ) in #if/#elif");
						error = 1;
						break;
					} //end if
				} //end if
				brace = false;
				// defined() creates a value
				lastwasvalue = 1;
				break;
			} //end case
			case TT_NUMBER:
			{
				if (lastwasvalue)
				{
					Com_Printf(S_COLOR_RED "ERROR: syntax error in #if/#elif");
					error = 1;
					break;
				} //end if
				//v = (value_t *) GetClearedMemory(sizeof(value_t));
				// alloc value
				if (numvalues >= k_MAX_VALUES) 
				{
					Com_Printf(S_COLOR_RED "ERROR:out of value space\n");		
					error = 1;										
					break;											
				}													
				else												
					v = &value_heap[numvalues++];

				if (negativevalue)
				{
					v->intvalue_ = - (signed int) t->intvalue_;
					v->floatvalue_ = - t->floatvalue_;
				} //end if
				else
				{
					v->intvalue_ = t->intvalue_;
					v->floatvalue_ = t->floatvalue_;
				} //end else
				v->parentheses_ = parentheses;
				v->next_ = NULL;
				v->prev_ = lastvalue;
				if (lastvalue) 
					lastvalue->next_ = v;
				else 
					firstvalue = v;
				lastvalue = v;
				//last token was a value
				lastwasvalue = 1;
				//
				negativevalue = 0;
				break;
			} //end case
			case TT_PUNCTUATION:
			{
				if (negativevalue)
				{
					Com_Printf(S_COLOR_RED "ERROR: misplaced minus sign in #if/#elif");
					error = 1;
					break;
				} //end if
				if (t->subtype_ == P_PARENTHESESOPEN)
				{
					parentheses++;
					break;
				} //end if
				else if (t->subtype_ == P_PARENTHESESCLOSE)
				{
					parentheses--;
					if (parentheses < 0)
					{
						Com_Printf(S_COLOR_RED "ERROR: too many ) in #if/#elsif");
						error = 1;
					} //end if
					break;
				} //end else if
				//check for invalid operators on floating point values
				if (!integer)
				{
					if (t->subtype_ == P_BIN_NOT || t->subtype_ == P_MOD ||
						t->subtype_ == P_RSHIFT || t->subtype_ == P_LSHIFT ||
						t->subtype_ == P_BIN_AND || t->subtype_ == P_BIN_OR ||
						t->subtype_ == P_BIN_XOR)
					{
						Com_Printf(S_COLOR_RED "ERROR: illigal operator %s on floating point operands\n", t->string_);
						error = 1;
						break;
					} //end if
				} //end if
				switch(t->subtype_)
				{
					case P_LOGIC_NOT:
					case P_BIN_NOT:
					{
						if (lastwasvalue)
						{
							Com_Printf(S_COLOR_RED "ERROR: ! or ~ after value in #if/#elif");
							error = 1;
							break;
						} //end if
						break;
					} //end case
					case P_INC:
					case P_DEC:
					{
						Com_Printf(S_COLOR_RED "ERROR: ++ or -- used in #if/#elif");
						break;
					} //end case
					case P_SUB:
					{
						if (!lastwasvalue)
						{
							negativevalue = 1;
							break;
						} //end if
					} //end case
					
					case P_MUL:
					case P_DIV:
					case P_MOD:
					case P_ADD:

					case P_LOGIC_AND:
					case P_LOGIC_OR:
					case P_LOGIC_GEQ:
					case P_LOGIC_LEQ:
					case P_LOGIC_EQ:
					case P_LOGIC_UNEQ:

					case P_LOGIC_GREATER:
					case P_LOGIC_LESS:

					case P_RSHIFT:
					case P_LSHIFT:

					case P_BIN_AND:
					case P_BIN_OR:
					case P_BIN_XOR:

					case P_COLON:
					case P_QUESTIONMARK:
					{
						if (!lastwasvalue)
						{
							Com_Printf(S_COLOR_RED "ERROR: operator %s after operator in #if/#elif", t->string_);
							error = 1;
							break;
						} //end if
						break;
					} //end case
					default:
					{
						Com_Printf(S_COLOR_RED "ERROR: invalid operator %s in #if/#elif", t->string_);
						error = 1;
						break;
					} //end default
				} //end switch
				if (!error && !negativevalue)
				{
					//o = (operator_t *) GetClearedMemory(sizeof(operator_t));

					// alloc operator
					if (numoperators >= k_MAX_OPERATORS) 
					{
						Com_Printf(S_COLOR_RED "ERROR:out of operator space\n");	
						error = 1;										
						break;											
					}													
					else												
						o = &operator_heap[numoperators++];

					o->operator_ = t->subtype_;
					o->priority_ = operatorPriority(t->subtype_);
					o->parentheses_ = parentheses;
					o->next_ = NULL;
					o->prev_ = lastoperator;
					if (lastoperator) 
						lastoperator->next_ = o;
					else 
						firstoperator = o;
					lastoperator = o;
					lastwasvalue = 0;
				} //end if
				break;
			} //end case
			default:
			{
				Com_Printf(S_COLOR_RED "ERROR: unknown %s in #if/#elif", t->string_);
				error = 1;
				break;
			} //end default
		} //end switch
		if (error) break;
	} //end for
	if (!error)
	{
		if (!lastwasvalue)
		{
			Com_Printf(S_COLOR_RED "ERROR: trailing operator in #if/#elif");
			error = 1;
		} //end if
		else if (parentheses)
		{
			Com_Printf(S_COLOR_RED "ERROR: too many ( in #if/#elif");
			error = 1;
		} //end else if
	} //end if
	//
	gotquestmarkvalue = false;
	questmarkintvalue = 0;
	questmarkfloatvalue = 0;
	//while there are operators
	while(!error && firstoperator)
	{
		v = firstvalue;
		for (o = firstoperator; o->next_; o = o->next_)
		{
			//if the current operator is nested deeper in parentheses
			//than the next operator
			if (o->parentheses_ > o->next_->parentheses_) break;
			//if the current and next operator are nested equally deep in parentheses
			if (o->parentheses_ == o->next_->parentheses_)
			{
				//if the priority of the current operator is equal or higher
				//than the priority of the next operator
				if (o->priority_ >= o->next_->priority_) break;
			} //end if
			//if the arity of the operator isn't equal to 1
			if (o->operator_ != P_LOGIC_NOT
					&& o->operator_ != P_BIN_NOT) v = v->next_;
			//if there's no value or no next value
			if (!v)
			{
				Com_Printf(S_COLOR_RED "ERROR: mising values in #if/#elif");
				error = 1;
				break;
			} //end if
		} //end for
		if (error) 
			break;
		v1 = v;
		v2 = v->next_;

		switch(o->operator_)
		{
			case P_LOGIC_NOT:		v1->intvalue_ = !v1->intvalue_;
									v1->floatvalue_ = !v1->floatvalue_;
									break;
			case P_BIN_NOT:			v1->intvalue_ = ~v1->intvalue_;
									break;
			case P_MUL:				v1->intvalue_ *= v2->intvalue_;
									v1->floatvalue_ *= v2->floatvalue_; 
									break;
			case P_DIV:				if (!v2->intvalue_ || !v2->floatvalue_)
									{
										Com_Printf(S_COLOR_RED "ERROR: divide by zero in #if/#elif\n");
										error = 1;
										break;
									}
									v1->intvalue_ /= v2->intvalue_;
									v1->floatvalue_ /= v2->floatvalue_; break;
			case P_MOD:				if (!v2->intvalue_)
									{
										Com_Printf(S_COLOR_RED "ERROR: divide by zero in #if/#elif\n");
										error = 1;
										break;
									}
									v1->intvalue_ %= v2->intvalue_; 
									break;
			case P_ADD:				v1->intvalue_ += v2->intvalue_;
									v1->floatvalue_ += v2->floatvalue_; 
									break;
			case P_SUB:				v1->intvalue_ -= v2->intvalue_;
									v1->floatvalue_ -= v2->floatvalue_; 
									break;
			case P_LOGIC_AND:		v1->intvalue_ = v1->intvalue_ && v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ && v2->floatvalue_; 
									break;
			case P_LOGIC_OR:		v1->intvalue_ = v1->intvalue_ || v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ || v2->floatvalue_; 
									break;
			case P_LOGIC_GEQ:		v1->intvalue_ = v1->intvalue_ >= v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ >= v2->floatvalue_; 
									break;
			case P_LOGIC_LEQ:		v1->intvalue_ = v1->intvalue_ <= v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ <= v2->floatvalue_; 
									break;
			case P_LOGIC_EQ:		v1->intvalue_ = v1->intvalue_ == v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ == v2->floatvalue_; 
									break;
			case P_LOGIC_UNEQ:		v1->intvalue_ = v1->intvalue_ != v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ != v2->floatvalue_; 
									break;
			case P_LOGIC_GREATER:	v1->intvalue_ = v1->intvalue_ > v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ > v2->floatvalue_; 
									break;
			case P_LOGIC_LESS:		v1->intvalue_ = v1->intvalue_ < v2->intvalue_;
									v1->floatvalue_ = v1->floatvalue_ < v2->floatvalue_; 
									break;
			case P_RSHIFT:			v1->intvalue_ >>= v2->intvalue_;
									break;
			case P_LSHIFT:			v1->intvalue_ <<= v2->intvalue_;
									break;
			case P_BIN_AND:			v1->intvalue_ &= v2->intvalue_;
									break;
			case P_BIN_OR:			v1->intvalue_ |= v2->intvalue_;
									break;
			case P_BIN_XOR:			v1->intvalue_ ^= v2->intvalue_;
									break;
			case P_COLON:
			{
				if (!gotquestmarkvalue)
				{
					Com_Printf(S_COLOR_RED "ERROR: : without ? in #if/#elif");
					error = 1;
					break;
				} //end if
				if (integer)
				{
					if (!questmarkintvalue) 
						v1->intvalue_ = v2->intvalue_;
				} //end if
				else
				{
					if (!questmarkfloatvalue) 
						v1->floatvalue_ = v2->floatvalue_;
				} //end else
				gotquestmarkvalue = false;
				break;
			} //end case
			case P_QUESTIONMARK:
			{
				if (gotquestmarkvalue)
				{
					Com_Printf(S_COLOR_RED "ERROR: ? after ? in #if/#elif");
					error = 1;
					break;
				} //end if
				questmarkintvalue = v1->intvalue_;
				questmarkfloatvalue = v1->floatvalue_;
				gotquestmarkvalue = true;
				break;
			} //end if
		} //end switch

		if (error) 
			break;
		lastoperatortype = o->operator_;
		//if not an operator with arity 1
		if (o->operator_ != P_LOGIC_NOT
				&& o->operator_ != P_BIN_NOT)
		{
			//remove the second value if not question mark operator
			if (o->operator_ != P_QUESTIONMARK) 
				v = v->next_;
			//
			if (v->prev_) 
				v->prev_->next_ = v->next_;
			else 
				firstvalue = v->next_;
			if (v->next_) 
				v->next_->prev_ = v->prev_;
			else 
				lastvalue = v->prev_;
			//FreeMemory(v);
			//FreeValue(v);
		} //end if
		//remove the operator
		if (o->prev_) 
			o->prev_->next_ = o->next_;
		else 
			firstoperator = o->next_;
		if (o->next_) 
			o->next_->prev_ = o->prev_;
		else 
			lastoperator = o->prev_;
		//FreeMemory(o);
		//FreeOperator(o);
	} //end while
	if (firstvalue)
	{
		if (intvalue) 
			*intvalue = firstvalue->intvalue_;
		if (floatvalue) 
			*floatvalue = firstvalue->floatvalue_;
	} //end if
	for (o = firstoperator; o; o = lastoperator)
	{
		lastoperator = o->next_;
		//FreeMemory(o);
		//FreeOperator(o);
	} //end for
	for (v = firstvalue; v; v = lastvalue)
	{
		lastvalue = v->next_;
		//FreeMemory(v);
		//FreeValue(v);
	} //end for
	if (!error) 
		return true;
	if (intvalue) 
		*intvalue = 0;
	if (floatvalue) 
		*floatvalue = 0;
	return false;
} 

int 
UI_PrecompilerTools::nameHash( char *name )
{
	int register hash, i;

	hash = 0;
	for (i = 0; name[i] != '\0'; i++)
	{
		hash += name[i] * (119 + i);
		//hash += (name[i] << 7) + i;
		//hash += (name[i] << (i&15));
	} //end while
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (k_DEFINEHASHSIZE-1);
	return hash;
}

void 
UI_PrecompilerTools::stripDoubleQuotes( char *string )
{
	if (*string == '\"')
	{
		strcpy(string, string+1);
	}
	if (string[strlen(string)-1] == '\"')
	{
		string[strlen(string)-1] = '\0';
	}
}

void 
UI_PrecompilerTools::numberValue( char *string, int subtype, unsigned long int *intvalue, long double *floatvalue )
{
	unsigned long int dotfound = 0;

	*intvalue = 0;
	*floatvalue = 0;
	//floating point number
	if (subtype & TT_FLOAT)
	{
		while(*string)
		{
			if (*string == '.')
			{
				if (dotfound) 
					return;
				dotfound = 10;
				string++;
			} 
			if (dotfound)
			{
				*floatvalue = *floatvalue + (long double) (*string - '0') /	(long double) dotfound;
				dotfound *= 10;
			}
			else
			{
				*floatvalue = *floatvalue * 10.0 + (long double) (*string - '0');
			}
			string++;
		} 
		*intvalue = (unsigned long) *floatvalue;
	}
	else if (subtype & TT_DECIMAL)
	{
		while(*string) *intvalue = *intvalue * 10 + (*string++ - '0');
		*floatvalue = *intvalue;
	} 
	else if (subtype & TT_HEX)
	{
		//step over the leading 0x or 0X
		string += 2;
		while(*string)
		{
			*intvalue <<= 4;
			if (*string >= 'a' && *string <= 'f') *intvalue += *string - 'a' + 10;
			else if (*string >= 'A' && *string <= 'F') *intvalue += *string - 'A' + 10;
			else *intvalue += *string - '0';
			string++;
		} 
		*floatvalue = *intvalue;
	} 
	else if (subtype & TT_OCTAL)
	{
		//step over the first zero
		string += 1;
		while(*string) *intvalue = (*intvalue << 3) + (*string++ - '0');
		*floatvalue = *intvalue;
	} 
	else if (subtype & TT_BINARY)
	{
		//step over the leading 0b or 0B
		string += 2;
		while(*string) *intvalue = (*intvalue << 1) + (*string++ - '0');
		*floatvalue = *intvalue;
	} 
}

void 
UI_PrecompilerTools::convertPath( char *path )
{
	char *ptr;

	//remove double path seperators
	for (ptr = path; *ptr;)
	{
		if ((*ptr == '\\' || *ptr == '/') &&
				(*(ptr+1) == '\\' || *(ptr+1) == '/'))
		{
			strcpy(ptr, ptr+1);
		} //end if
		else
		{
			ptr++;
		} //end else
	} //end while
	//set OS dependent path seperators
	for (ptr = path; *ptr;)
	{
		if (*ptr == '/' || *ptr == '\\')
			*ptr = PATHSEPERATOR_CHAR;
		ptr++;
	} //end while
}

int 
UI_PrecompilerTools::operatorPriority( int op )
{
	switch(op)
	{
		case P_MUL: return 15;
		case P_DIV: return 15;
		case P_MOD: return 15;
		case P_ADD: return 14;
		case P_SUB: return 14;

		case P_LOGIC_AND: return 7;
		case P_LOGIC_OR: return 6;
		case P_LOGIC_GEQ: return 12;
		case P_LOGIC_LEQ: return 12;
		case P_LOGIC_EQ: return 11;
		case P_LOGIC_UNEQ: return 11;

		case P_LOGIC_NOT: return 16;
		case P_LOGIC_GREATER: return 12;
		case P_LOGIC_LESS: return 12;

		case P_RSHIFT: return 13;
		case P_LSHIFT: return 13;

		case P_BIN_AND: return 10;
		case P_BIN_OR: return 8;
		case P_BIN_XOR: return 9;
		case P_BIN_NOT: return 16;

		case P_COLON: return 5;
		case P_QUESTIONMARK: return 5;
	} //end switch
	return false;
}

int 
UI_PrecompilerTools::whiteSpaceBeforeToken( Token* token )
{
	return token->endwhitespace_p_ - token->whitespace_p_ > 0;
}

void
UI_PrecompilerTools::unreadSignToken( SourceFile* source )
{
	Token token;

	token.line_ = source->scriptstack_->line_;
	token.whitespace_p_ = source->scriptstack_->script_p_;
	token.endwhitespace_p_ = source->scriptstack_->script_p_;
	token.linescrossed_ = 0;
	strcpy(token.string_, "-");
	token.type_ = TT_PUNCTUATION;
	token.subtype_ = P_SUB;
	unreadSourceToken(source, &token);
}

int 
UI_PrecompilerTools::dollarEvaluate( SourceFile* source, signed long int *intvalue,
									 double *floatvalue, int integer )
{
	int indent, defined = false;
	Token token, *firsttoken, *lasttoken;
	Token *t, *nexttoken;
	Define *define;

	if (intvalue) 
		*intvalue = 0;
	if (floatvalue) 
		*floatvalue = 0;
	//
	if (!readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: no leading ( after $evalint/$evalfloat");
		return false;
	} //end if
	if (!readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: nothing to evaluate");
		return false;
	} //end if
	indent = 1;
	firsttoken = NULL;
	lasttoken = NULL;
	do
	{
		//if the token is a name
		if (token.type_ == TT_NAME)
		{
			if (defined)
			{
				defined = false;
				t = copyToken(&token);
				t->next_ = NULL;
				if (lasttoken) 
					lasttoken->next_ = t;
				else 
					firsttoken = t;
				lasttoken = t;
			} //end if
			else if (!strcmp(token.string_, "defined"))
			{
				defined = true;
				t = copyToken(&token);
				t->next_ = NULL;
				if (lasttoken) 
					lasttoken->next_ = t;
				else 
					firsttoken = t;
				lasttoken = t;
			} //end if
			else
			{
				//then it must be a define
				define = findHashedDefine(source->definehash_, token.string_);

				if (!define)
				{
					Com_Printf(S_COLOR_RED "ERROR: can't evaluate %s, not defined", token.string_);
					return false;
				} //end if
				if (!expandDefineIntoSource(source, &token, define)) return false;
			} //end else
		} //end if
		//if the token is a number or a punctuation
		else if (token.type_ == TT_NUMBER || token.type_ == TT_PUNCTUATION)
		{
			if (*token.string_ == '(') 
				indent++;
			else if (*token.string_ == ')') 
				indent--;
			if (indent <= 0) 
				break;
			t = copyToken(&token);
			t->next_ = NULL;
			if (lasttoken) 
				lasttoken->next_ = t;
			else 
				firsttoken = t;
			lasttoken = t;
		} //end else
		else //can't evaluate the token
		{
			Com_Printf(S_COLOR_RED "ERROR: can't evaluate %s", token.string_);
			return false;
		} //end else
	} while(readSourceToken(source, &token));
	//
	if (!evaluateTokens(source, firsttoken, intvalue, floatvalue, integer)) 
		return false;
	//
	for (t = firsttoken; t; t = nexttoken)
	{
		nexttoken = t->next_;
		freeToken(t);
	} //end for
	//
	return true;
}

int 
UI_PrecompilerTools::directive_undef( SourceFile* source )
{
	Token token;
	Define *define, *lastdefine;
	int hash;

	if (source->skip_ > 0) 
		return true;
	//
	if (!readLine(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: undef without name");
		return false;
	} //end if
	if (token.type_ != TT_NAME)
	{
		unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: expected name, found %s", token.string_);
		return false;
	} //end if

	hash = nameHash(token.string_);
	for (lastdefine = NULL, define = source->definehash_[hash]; define; define = define->hashnext_)
	{
		if (!strcmp(define->name_, token.string_))
		{
			if (define->flags_ & k_DEFINE_FIXED)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: can't undef %s", token.string_);
			} //end if
			else
			{
				if (lastdefine) 
					lastdefine->hashnext_ = define->hashnext_;
				else 
					source->definehash_[hash] = define->hashnext_;
				freeDefine(define);
			} //end else
			break;
		} //end if
		lastdefine = define;
	} //end for
	return true;
} 

int 
UI_PrecompilerTools::checkTokenString( SourceFile* source, char *string )
{
	Token tok;

	if (!readToken(source, &tok)) 
		return false;
	//if the token is available
	if (!strcmp(tok.string_, string)) 
		return true;
	//
	unreadSourceToken(source, &tok);
	return false;
}

void 
UI_PrecompilerTools::clearTokenWhiteSpace( Token* token )
{
	token->whitespace_p_ = 0;
	token->endwhitespace_p_ = 0;
	token->linescrossed_ = 0;
}

int
UI_PrecompilerTools::sourceFileAndLine( int handle, char *filename, int *line )
{
	if (handle < 1 || handle >= k_MAX_SOURCEFILES)
		return false;
	if (!sourceFiles_[handle])
		return false;

	strcpy(filename, sourceFiles_[handle]->filename_);
	if (sourceFiles_[handle]->scriptstack_)
		*line = sourceFiles_[handle]->scriptstack_->line_;
	else
		*line = 0;
	return true;
}

// ------------------------------------

UI_PrecompilerTools* UI_PrecompilerTools::Directive::tools_ = 0;

int
UI_PrecompilerTools::Directive::if_def( SourceFile* source, int type )
{
	Token token;
	Define* d;
	int skip;

	if (!tools_->readLine(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: #ifdef without name");
		return false;
	} //end if
	if (token.type_ != TT_NAME)
	{
		tools_->unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: expected name after #ifdef, found %s", token.string_);
		return false;
	} //end if
	d = tools_->findHashedDefine(source->definehash_, token.string_);

	skip = (type == INDENT_IFDEF) == (d == NULL);
	tools_->pushIndent(source, type, skip);
	return true;
}

int
UI_PrecompilerTools::Directive_if::func( SourceFile* source )
{
	signed long int value;
	int skip;

	if (!tools_->evaluate(source, &value, NULL, true)) 
		return false;
	skip = (value == 0);
	tools_->pushIndent(source, INDENT_IF, skip);
	return true;
}

int
UI_PrecompilerTools::Directive_ifdef::func( SourceFile* source )
{
	return if_def( source, INDENT_IFDEF );
}

int
UI_PrecompilerTools::Directive_ifndef::func( SourceFile* source )
{
	return if_def( source, INDENT_IFNDEF );
}

int
UI_PrecompilerTools::Directive_elif::func( SourceFile* source )
{
	signed long int value;
	int type, skip;

	tools_->popIndent(source, &type, &skip);
	if (!type || type == INDENT_ELSE)
	{
		Com_Printf(S_COLOR_RED "ERROR: misplaced #elif");
		return false;
	} //end if
	if (!tools_->evaluate(source, &value, NULL, true)) 
		return false;
	skip = (value == 0);
	tools_->pushIndent(source, INDENT_ELIF, skip);
	return true;
}

int
UI_PrecompilerTools::Directive_else::func( SourceFile* source )
{
	int type, skip;

	tools_->popIndent(source, &type, &skip);
	if (!type)
	{
		Com_Printf(S_COLOR_RED "ERROR: misplaced #else");
		return false;
	} //end if
	if (type == INDENT_ELSE)
	{
		Com_Printf(S_COLOR_RED "ERROR: #else after #else");
		return false;
	} //end if
	tools_->pushIndent(source, INDENT_ELSE, !skip);
	return true;
}

int
UI_PrecompilerTools::Directive_endif::func( SourceFile* source )
{
	int type, skip;

	tools_->popIndent(source, &type, &skip);
	if (!type)
	{
		Com_Printf(S_COLOR_RED "ERROR: misplaced #endif");
		return false;
	} //end if
	return true;
}

int
UI_PrecompilerTools::Directive_include::func( SourceFile* source )
{
	Script* script;
	Token token;
	char path[MAX_QPATH];

	if (source->skip_ > 0) return true;
	//
	if (!tools_->readSourceToken(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: #include without file name");
		return false;
	} //end if
	if (token.linescrossed_ > 0)
	{
		Com_Printf(S_COLOR_RED "ERROR: #include without file name");
		return false;
	} //end if
	if (token.type_ == TT_STRING)
	{
		tools_->stripDoubleQuotes(token.string_);
		tools_->convertPath(token.string_);
		script = tools_->loadScriptFile(token.string_);
		if (!script)
		{
			strcpy(path, source->includePath_);
			strcat(path, token.string_);
			script = tools_->loadScriptFile(path);
		} //end if
	} //end if
	else if (token.type_ == TT_PUNCTUATION && *token.string_ == '<')
	{
		strcpy(path, source->includePath_);
		while(tools_->readSourceToken(source, &token))
		{
			if (token.linescrossed_ > 0)
			{
				tools_->unreadSourceToken(source, &token);
				break;
			} //end if
			if (token.type_ == TT_PUNCTUATION && *token.string_ == '>') break;
			strncat(path, token.string_, MAX_QPATH);
		} //end while
		if (*token.string_ != '>')
		{
			Com_Printf(S_COLOR_YELLOW "WARNING: #include missing trailing >");
		} //end if
		if (!strlen(path))
		{
			Com_Printf(S_COLOR_RED "ERROR: #include without file name between < >");
			return false;
		} //end if
		tools_->convertPath(path);
		script = tools_->loadScriptFile(path);
	} //end if
	else
	{
		Com_Printf(S_COLOR_RED "ERROR: #include without file name");
		return false;
	} //end else
	if (!script)
	{
		Com_Printf(S_COLOR_RED "ERROR: file %s not found", path);
		return false;
	} //end if
	tools_->pushScript(source, script);
	return true;
}

int
UI_PrecompilerTools::Directive_define::func( SourceFile* source )
{
	Token token;
	Token* t;
	Token* last;
	Define* define;

	if (source->skip_ > 0) return true;
	//
	if (!tools_->readLine(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: #define without name");
		return false;
	} //end if
	if (token.type_ != TT_NAME)
	{
		tools_->unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: expected name after #define, found %s", token.string_);
		return false;
	} //end if
	//check if the define already exists
	define = tools_->findHashedDefine(source->definehash_, token.string_);

	if (define)
	{
		if (define->flags_ & k_DEFINE_FIXED)
		{
			Com_Printf(S_COLOR_RED "ERROR: can't redefine %s", token.string_);
			return false;
		} //end if
		Com_Printf(S_COLOR_YELLOW "WARNING: redefinition of %s", token.string_);
		//unread the define name before executing the #undef directive
		tools_->unreadSourceToken(source, &token);
		if (!tools_->directive_undef(source)) return false;
		//if the define was not removed (define->flags & k_DEFINE_FIXED)
		define = tools_->findHashedDefine(source->definehash_, token.string_);

	} //end if
	//allocate define
	define = reinterpret_cast<Define*>(UI_Memtools::getMemory(sizeof(Define) + strlen(token.string_) + 1));
	Com_Memset(define, 0, sizeof(Define));
	define->name_ = (char *) define + sizeof(Define);
	strcpy(define->name_, token.string_);
	//add the define to the source
	tools_->addDefineToHash(define, source->definehash_);

	//if nothing is defined, just return
	if (!tools_->readLine(source, &token)) 
		return true;
	//if it is a define with parameters
	if (!tools_->whiteSpaceBeforeToken(&token) && !strcmp(token.string_, "("))
	{
		//read the define parameters
		last = NULL;
		if (!tools_->checkTokenString(source, ")"))
		{
			while(1)
			{
				if (!tools_->readLine(source, &token))
				{
					Com_Printf(S_COLOR_RED "ERROR: expected define parameter");
					return false;
				} //end if
				//if it isn't a name
				if (token.type_ != TT_NAME)
				{
					Com_Printf(S_COLOR_RED "ERROR: invalid define parameter");
					return false;
				} //end if
				//
				if (tools_->findDefineParm(define, token.string_) >= 0)
				{
					Com_Printf(S_COLOR_RED "ERROR: two the same define parameters");
					return false;
				} //end if
				//add the define parm
				t = tools_->copyToken(&token);
				tools_->clearTokenWhiteSpace(t);
				t->next_ = NULL;
				if (last) last->next_ = t;
				else define->parms_ = t;
				last = t;
				define->numparms_++;
				//read next token
				if (!tools_->readLine(source, &token))
				{
					Com_Printf(S_COLOR_RED "ERROR: define parameters not terminated");
					return false;
				} //end if
				//
				if (!strcmp(token.string_, ")")) 
					break;
				//then it must be a comma
				if (strcmp(token.string_, ","))
				{
					Com_Printf(S_COLOR_RED "ERROR: define not terminated");
					return false;
				} //end if
			} //end while
		} //end if
		if (!tools_->readLine(source, &token)) return true;
	} //end if
	//read the defined stuff
	last = NULL;
	do
	{
		t = tools_->copyToken(&token);
		if (t->type_ == TT_NAME && !strcmp(t->string_, define->name_))
		{
			Com_Printf(S_COLOR_RED "ERROR: recursive define (removed recursion)");
			continue;
		} //end if
		tools_->clearTokenWhiteSpace(t);
		t->next_ = NULL;
		if (last) 
			last->next_ = t;
		else 
			define->tokens_ = t;
		last = t;
	} while(tools_->readLine(source, &token));
	//
	if (last)
	{
		//check for merge operators at the beginning or end
		if (!strcmp(define->tokens_->string_, "##") ||
				!strcmp(last->string_, "##"))
		{
			Com_Printf(S_COLOR_RED "ERROR: define with misplaced ##");
			return false;
		} //end if
	} //end if
	return true;
}

int
UI_PrecompilerTools::Directive_undef::func( SourceFile* source )
{
	Token token;
	Define *define, *lastdefine;
	int hash;

	if (source->skip_ > 0) 
		return true;
	//
	if (!tools_->readLine(source, &token))
	{
		Com_Printf(S_COLOR_RED "ERROR: undef without name");
		return false;
	} //end if
	if (token.type_ != TT_NAME)
	{
		tools_->unreadSourceToken(source, &token);
		Com_Printf(S_COLOR_RED "ERROR: expected name, found %s", token.string_);
		return false;
	} //end if

	hash = tools_->nameHash(token.string_);
	for (lastdefine = NULL, define = source->definehash_[hash]; define; define = define->hashnext_)
	{
		if (!strcmp(define->name_, token.string_))
		{
			if (define->flags_ & k_DEFINE_FIXED)
			{
				Com_Printf(S_COLOR_YELLOW "WARNING: can't undef %s", token.string_);
			} //end if
			else
			{
				if (lastdefine) 
					lastdefine->hashnext_ = define->hashnext_;
				else 
					source->definehash_[hash] = define->hashnext_;
				tools_->freeDefine(define);
			} //end else
			break;
		} //end if
		lastdefine = define;
	} //end for
	return true;
}

int
UI_PrecompilerTools::Directive_line::func( SourceFile* source )
{
	Com_Printf(S_COLOR_RED "ERROR: #line directive not supported");
	return false;
}

int
UI_PrecompilerTools::Directive_error::func( SourceFile* source )
{
	Token token;

	strcpy(token.string_, "");
	tools_->readSourceToken(source, &token);
	Com_Printf(S_COLOR_RED "ERROR: #error directive: %s", token.string_);
	return false;
}

int
UI_PrecompilerTools::Directive_pragma::func( SourceFile* source )
{
	Token token;

	Com_Printf(S_COLOR_YELLOW "WARNING: #pragma directive not supported");
	while(tools_->readLine(source, &token)) ;

	return true;
}

int
UI_PrecompilerTools::Directive_eval::func( SourceFile* source )
{
	signed long int value;
	Token token;

	if (!tools_->evaluate(source, &value, NULL, true)) return false;
	//
	token.line_ = source->scriptstack_->line_;
	token.whitespace_p_ = source->scriptstack_->script_p_;
	token.endwhitespace_p_ = source->scriptstack_->script_p_;
	token.linescrossed_ = 0;
	sprintf(token.string_, "%d", abs(value));
	token.type_ = TT_NUMBER;
	token.subtype_ = TT_INTEGER|TT_LONG|TT_DECIMAL;
	tools_->unreadSourceToken(source, &token);
	if (value < 0) 
		tools_->unreadSignToken(source);
	return true;
}

int
UI_PrecompilerTools::Directive_evalfloat::func( SourceFile* source )
{
	double value;
	Token token;

	if (!tools_->evaluate(source, NULL, &value, false)) return false;
	token.line_ = source->scriptstack_->line_;
	token.whitespace_p_ = source->scriptstack_->script_p_;
	token.endwhitespace_p_ = source->scriptstack_->script_p_;
	token.linescrossed_ = 0;
	sprintf(token.string_, "%1.2f", fabs(value));
	token.type_ = TT_NUMBER;
	token.subtype_ = TT_FLOAT|TT_LONG|TT_DECIMAL;
	tools_->unreadSourceToken(source, &token);
	if (value < 0) 
		tools_->unreadSignToken(source);
	return true;
}

int
UI_PrecompilerTools::DollarDirective_evalint::func( SourceFile* source )
{
	signed long int value;
	Token token;

	if (!tools_->dollarEvaluate(source, &value, NULL, true)) 
		return false;
	//
	token.line_ = source->scriptstack_->line_;
	token.whitespace_p_ = source->scriptstack_->script_p_;
	token.endwhitespace_p_ = source->scriptstack_->script_p_;
	token.linescrossed_ = 0;
	sprintf(token.string_, "%d", abs(value));
	token.type_ = TT_NUMBER;
	token.subtype_ = TT_INTEGER|TT_LONG|TT_DECIMAL;
	token.intvalue_ = value;
	token.floatvalue_ = value;
	tools_->unreadSourceToken(source, &token);
	if (value < 0) 
		tools_->unreadSignToken(source);
	return true;
} 

int
UI_PrecompilerTools::DollarDirective_evalfloat::func( SourceFile* source )
{
	double value;
	Token token;

	if (!tools_->dollarEvaluate(source, NULL, &value, false)) 
		return false;
	token.line_ = source->scriptstack_->line_;
	token.whitespace_p_ = source->scriptstack_->script_p_;
	token.endwhitespace_p_ = source->scriptstack_->script_p_;
	token.linescrossed_ = 0;
	sprintf(token.string_, "%1.2f", fabs(value));
	token.type_ = TT_NUMBER;
	token.subtype_ = TT_FLOAT|TT_LONG|TT_DECIMAL;
	token.intvalue_ = (unsigned long) value;
	token.floatvalue_ = value;
	tools_->unreadSourceToken(source, &token);
	if (value < 0) 
		tools_->unreadSignToken(source);
	return true;
} 