#ifndef __UI_HASHUTILS_H__
#define __UI_HASHUTILS_H__

#include <map>
#include <string>




struct UI_Utils;
struct KeywordHash;


struct UI_HashUtils
{					
							UI_HashUtils();
							~UI_HashUtils();
	
	static void				setUtils( UI_Utils* utils );

	KeywordHash*			findItemKeyword( const char* keyword ); 
	KeywordHash*			findMenuKeyword( const char* keyword ); 

private:
	// disable
							UI_HashUtils( UI_HashUtils const& );
	UI_HashUtils&			operator=( UI_HashUtils const& );

	typedef std::map<int, KeywordHash*> KeywordMap;
	typedef std::map<int, KeywordHash*>::iterator KeywordMapIter;

	KeywordHash*			findKeyword( KeywordMap& map, const char* keyword );

	int						getKey( const char* keyword );

	bool					addItemKeyword( KeywordHash* kh );
	bool					addMenuKeyword( KeywordHash* kh );

	bool					addKeyword( KeywordMap& map, KeywordHash* kh );

	// data
	KeywordMap				itemKeywords_;
	KeywordMap				menuKeywords_;

	static UI_Utils*		utils_;

	static const int		k_KEYWORDHASH_SIZE = 512;
};





#endif // __UI_HASHUTILS_H__

