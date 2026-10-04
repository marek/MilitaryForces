#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

namespace
{
	constexpr std::array<Bots::Character, 16> builtinCharacters = { {
		{ "Alpha", 0.9f, 0.3f, 0.4f, 0.5f, 0.9f, 0.3f, 0.2f, { 1, 1, 1, 1, 1 } },
		{ "Bravo", 0.6f, 0.6f, 0.7f, 0.8f, 0.7f, 0.6f, 0.5f, { 1, 1, 1, 1, 1 } },
		{ "Charlie", 0.4f, 0.9f, 0.9f, 0.6f, 0.8f, 0.4f, 0.9f, { 1, 1, 1, 1, 1 } },
		{ "Delta", 1.0f, 0.2f, 0.2f, 0.3f, 0.6f, 0.9f, 0.1f, { 1, 1, 1, 1, 1 } },
		{ "Echo", 0.7f, 0.5f, 0.6f, 0.9f, 0.85f, 0.7f, 0.6f, { 1, 1, 1, 1, 1 } },
		{ "Foxtrot", 0.5f, 0.7f, 0.5f, 0.4f, 0.5f, 0.2f, 0.4f, { 1, 1, 1, 1, 1 } },
		{ "Golf", 0.8f, 0.4f, 0.8f, 0.7f, 0.95f, 0.8f, 0.8f, { 1, 1, 1, 1, 1 } },
		{ "Hotel", 0.3f, 0.8f, 0.6f, 0.9f, 0.6f, 0.5f, 0.3f, { 1, 1, 1, 1, 1 } },
		{ "India", 0.9f, 0.5f, 0.3f, 0.2f, 0.75f, 0.6f, 0.2f, { 1, 1, 1, 1, 1 } },
		{ "Juliet", 0.6f, 0.4f, 0.9f, 0.6f, 0.9f, 0.3f, 0.9f, { 1, 1, 1, 1, 1 } },
		{ "Kilo", 0.75f, 0.75f, 0.5f, 0.5f, 0.65f, 0.5f, 0.5f, { 1, 1, 1, 1, 1 } },
		{ "Lima", 0.5f, 0.3f, 0.3f, 0.8f, 0.55f, 0.7f, 0.1f, { 1, 1, 1, 1, 1 } },
		{ "Mike", 0.85f, 0.6f, 0.7f, 0.4f, 0.8f, 0.9f, 0.7f, { 1, 1, 1, 1, 1 } },
		{ "November", 0.4f, 0.6f, 0.8f, 0.7f, 0.7f, 0.4f, 0.6f, { 1, 1, 1, 1, 1 } },
		{ "Oscar", 0.95f, 0.35f, 0.5f, 0.6f, 1.0f, 1.0f, 0.4f, { 1, 1, 1, 1, 1 } },
		{ "Papa", 0.65f, 0.85f, 0.4f, 0.5f, 0.6f, 0.1f, 0.8f, { 1, 1, 1, 1, 1 } }
	} };
}

void Bots::loadCharacters()
{
	characters_.clear();

	fileHandle_t f;
	const int len = FileSystem::fOpenFileByMode( "bots/characters.txt", &f, FS_READ );
	if( len > 0 && len < 32768 )
	{
		std::vector<char> text( len + 1, 0 );
		FileSystem::read( text.data(), len, f );
		char* line = text.data();
		while( line && *line )
		{
			char* next = strchr( line, '\n' );
			if( next )
				*next++ = 0;
			Character who = {};
			for( float& weight : who.prefer )
				weight = 1.0f;
			const int got = sscanf( line, "%31s %f %f %f %f %f %f %f %f %f %f %f %f", who.name, &who.aggression, &who.caution, &who.patience,
				&who.teamwork, &who.aim, &who.honour, &who.cunning, &who.prefer[0], &who.prefer[1], &who.prefer[2], &who.prefer[3], &who.prefer[4] );
			if( got >= 8 && who.name[0] != '#' && who.name[0] != '/' )
				characters_.push_back( who );
			line = next;
		}
	}
	if( len >= 0 )
		FileSystem::fCloseFile( f );

	if( characters_.empty() )
		characters_.assign( builtinCharacters.begin(), builtinCharacters.end() );
}

const Bots::Character& Bots::character( const State& bot )
{
	if( characters_.empty() )
		loadCharacters();
	return characters_[bot.character % characters_.size()];
}

float Bots::skill( const State& bot )
{
	return std::clamp( g_botSkill.value, 1.0f, 5.0f ) / 5.0f * (0.5f + 0.5f * character( bot ).aim);
}

int Bots::memoryTime( const State& bot )
{
	return 12000 + static_cast<int>( 30000 * character( bot ).patience );
}
