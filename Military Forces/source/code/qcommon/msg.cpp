/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "../game/q_shared.h"
#include "qcommon.h"

static huffman_t		msgHuff;

static bool			msgInit = false;

int pcount[256];

cvar_t			*msg_huffstats;
int				msg_huffstatsHold;
static int		msg_huffCounts[256];

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

int oldsize = 0;

void MSG_initHuffman();

void Message::init( byte *data, int length ) {
	if (!msgInit) {
		MSG_initHuffman();
	}
	Com_Memset (this, 0, sizeof(*this));
	this->data = data;
	this->maxsize = length;
}

void Message::initOOB( byte *data, int length ) {
	if (!msgInit) {
		MSG_initHuffman();
	}
	Com_Memset (this, 0, sizeof(*this));
	this->data = data;
	this->maxsize = length;
	this->oob = true;
}

void Message::clear() {
	this->cursize = 0;
	this->overflowed = false;
	this->bit = 0;					//<- in bits
}


void Message::bitstream() {
	this->oob = false;
}

void Message::beginReading() {
	this->readcount = 0;
	this->bit = 0;
	this->oob = false;
}

void Message::beginReadingOOB() {
	this->readcount = 0;
	this->bit = 0;
	this->oob = true;
}

void Message::copy( byte *data, int length, Message *src )
{
	if (length<src->cursize) {
		Com_Error( ERR_DROP, "MSG_Copy: can't copy into a smaller Message buffer");
	}
	Com_Memcpy(this, src, sizeof(Message));
	this->data = data;
	Com_Memcpy(this->data, src->data, src->cursize);
}

/*
=============================================================================

bit functions
  
=============================================================================
*/

int	overflows;

// negative bit values include signs
void Message::writeBits( int value, int bits ) {
	int	i;

	oldsize += bits;

	if ( this->overflowed ) {
		return;
	}

	if ( bits == 0 || bits < -31 || bits > 32 ) {
		Com_Error( ERR_DROP, "MSG_WriteBits: bad bits %i", bits );
	}

	if ( bits < 0 ) {
		bits = -bits;
	}

	if ( this->oob ) {
		if ( this->cursize + ( bits >> 3 ) > this->maxsize ) {
			this->overflowed = true;
			return;
		}

		if ( bits == 8 ) {
			this->data[this->cursize] = value;
			this->cursize += 1;
			this->bit += 8;
		} else if ( bits == 16 ) {
			short temp = value;

			CopyLittleShort( &this->data[this->cursize], &temp );
			this->cursize += 2;
			this->bit += 16;
		} else if ( bits==32 ) {
			CopyLittleLong( &this->data[this->cursize], &value );
			this->cursize += 4;
			this->bit += 32;
		} else {
			Com_Error( ERR_DROP, "can't write %d bits", bits );
		}
	} else {
		value &= (0xffffffff >> (32 - bits));
		if ( bits&7 ) {
			int nbits;
			nbits = bits&7;
			if ( this->bit + nbits >= this->maxsize << 3 ) {
				this->overflowed = true;
				return;
			}
			for( i = 0; i < nbits; i++ ) {
				Huff_putBit( (value & 1), this->data, &this->bit );
				value = (value >> 1);
			}
			bits = bits - nbits;
		}
		if ( bits ) {
			for( i = 0; i < bits; i += 8 ) {
				if ( msg_huffstats && msg_huffstats->integer && !msg_huffstatsHold ) {
					msg_huffCounts[value & 0xff]++;
				}
				Huff_offsetTransmit( &msgHuff.compressor, (value & 0xff), this->data, &this->bit, this->maxsize << 3 );
				value = (value >> 8);

				if ( this->bit >= this->maxsize << 3 ) {
					this->overflowed = true;
					return;
				}
			}
		}
		this->cursize = (this->bit >> 3) + 1;
	}
}

int Message::readBits( int bits ) {
	int			value;
	int			get;
	bool	sgn;
	int			i, nbits;
	int			totalBits;
//	FILE*	fp;

	if ( this->readcount > this->cursize ) {
		return 0;
	}

	value = 0;

	if ( bits < 0 ) {
		bits = -bits;
		sgn = true;
	} else {
		sgn = false;
	}
	totalBits = bits;

	if (this->oob) {
		if (this->readcount + (bits>>3) > this->cursize) {
			this->readcount = this->cursize + 1;
			return 0;
		}

		if(bits==8)
		{
			value = this->data[this->readcount];
			this->readcount += 1;
			this->bit += 8;
		}
		else if(bits==16)
		{
			short temp;
			
			CopyLittleShort(&temp, &this->data[this->readcount]);
			value = temp;
			this->readcount += 2;
			this->bit += 16;
		}
		else if(bits==32)
		{
			CopyLittleLong(&value, &this->data[this->readcount]);
			this->readcount += 4;
			this->bit += 32;
		}
		else
			Com_Error(ERR_DROP, "can't read %d bits", bits);
	} else {
		nbits = 0;
		if (bits&7) {
			nbits = bits&7;
			if (this->bit + nbits > this->cursize << 3) {
				this->readcount = this->cursize + 1;
				return 0;
			}
			for(i=0;i<nbits;i++) {
				value |= (Huff_getBit(this->data, &this->bit)<<i);
			}
			bits = bits - nbits;
		}
		if (bits) {
//			fp = fopen("c:\\netchan.bin", "a");
			for(i=0;i<bits;i+=8) {
				Huff_offsetReceive (msgHuff.decompressor.tree, &get, this->data, &this->bit, this->cursize<<3);
//				fwrite(&get, 1, 1, fp);
				value = (unsigned int)value | ((unsigned int)get<<(i+nbits));

				if (this->bit > this->cursize<<3) {
					this->readcount = this->cursize + 1;
					return 0;
				}
			}
//			fclose(fp);
		}
		this->readcount = (this->bit>>3)+1;
	}
	if ( sgn && totalBits > 0 && totalBits < 32 ) {
		if ( value & ( 1 << ( totalBits - 1 ) ) ) {
			value |= -1 ^ ( ( 1 << totalBits ) - 1 );
		}
	}

	return value;
}



//================================================================================

//
// writing functions
//

void Message::writeChar( int c ) {
#ifdef PARANOID
	if (c < -128 || c > 127)
		Com_Error (ERR_FATAL, "MSG_WriteChar: range error");
#endif

	this->writeBits( c, 8 );
}

void Message::writeByte( int c ) {
#ifdef PARANOID
	if (c < 0 || c > 255)
		Com_Error (ERR_FATAL, "MSG_WriteByte: range error");
#endif

	this->writeBits( c, 8 );
}

void Message::writeData( const void *data, int length ) {
	int i;
	for(i=0;i<length;i++) {
		this->writeByte( ((byte *)data)[i]);
	}
}

void Message::writeShort( int c ) {
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Com_Error (ERR_FATAL, "MSG_WriteShort: range error");
#endif

	this->writeBits( c, 16 );
}

void Message::writeLong( int c ) {
	this->writeBits( c, 32 );
}

void Message::writeFloat( float f ) {
	floatint_t dat;
	dat.f = f;
	this->writeBits( dat.i, 32 );
}

void Message::writeString( const char *s ) {
	if ( !s ) {
		this->writeData( "", 1);
	} else {
		int		l,i;
		char	string[MAX_STRING_CHARS];

		l = strlen( s );
		if ( l >= MAX_STRING_CHARS ) {
			Com_Printf( "MSG_WriteString: MAX_STRING_CHARS" );
			this->writeData( "", 1);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0x80+ and '%' chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 || string[i] == '%' ) {
				string[i] = '.';
			}
		}

		this->writeData( string, l+1);
	}
}

void Message::writeBigString( const char *s ) {
	if ( !s ) {
		this->writeData( "", 1);
	} else {
		int		l,i;
		char	string[BIG_INFO_STRING];

		l = strlen( s );
		if ( l >= BIG_INFO_STRING ) {
			Com_Printf( "MSG_WriteString: BIG_INFO_STRING" );
			this->writeData( "", 1);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0x80+ and '%' chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 || string[i] == '%' ) {
				string[i] = '.';
			}
		}

		this->writeData( string, l+1);
	}
}

void Message::writeAngle( float f ) {
	this->writeByte( (int)(f*256/360) & 255);
}

void Message::writeAngle16( float f ) {
	this->writeShort( ANGLE2SHORT(f));
}


//============================================================

//
// reading functions
//

// returns -1 if no more characters are available
int Message::readChar() {
	int	c;
	
	c = (signed char)this->readBits( 8 );
	if ( this->readcount > this->cursize ) {
		c = -1;
	}	
	
	return c;
}

int Message::readByte() {
	int	c;
	
	c = (unsigned char)this->readBits( 8 );
	if ( this->readcount > this->cursize ) {
		c = -1;
	}	
	return c;
}

int Message::lookaheadByte() {
	const int bloc = Huff_getBloc();
	const int readcount = this->readcount;
	const int bit = this->bit;
	int c = this->readByte();
	Huff_setBloc(bloc);
	this->readcount = readcount;
	this->bit = bit;
	return c;
}

int Message::readShort() {
	int	c;
	
	c = (short)this->readBits( 16 );
	if ( this->readcount > this->cursize ) {
		c = -1;
	}	

	return c;
}

int Message::readLong() {
	int	c;
	
	c = this->readBits( 32 );
	if ( this->readcount > this->cursize ) {
		c = -1;
	}	
	
	return c;
}

float Message::readFloat() {
	floatint_t dat;
	
	dat.i = this->readBits( 32 );
	if ( this->readcount > this->cursize ) {
		dat.f = -1;
	}	
	
	return dat.f;	
}

char * Message::readString() {
	static char	string[MAX_STRING_CHARS];
	int		l,c;
	
	l = 0;
	do {
		c = this->readByte();		// use ReadByte so -1 is out of bounds
		if ( c == -1 || c == 0 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		if ( c > 127 ) {
			c = '.';
		}
		// break only after reading all expected data from bitstream
		if ( l >= sizeof(string)-1 ) {
			break;
		}
		string[l++] = c;
	} while (true);
	
	string[l] = '\0';
	
	return string;
}

char * Message::readBigString() {
	static char	string[BIG_INFO_STRING];
	int		l,c;
	
	l = 0;
	do {
		c = this->readByte();		// use ReadByte so -1 is out of bounds
		if ( c == -1 || c == 0 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		if ( c > 127 ) {
			c = '.';
		}
		// break only after reading all expected data from bitstream
		if ( l >= sizeof(string)-1 ) {
			break;
		}
		string[l++] = c;
	} while (true);
	
	string[l] = '\0';
	
	return string;
}

char * Message::readStringLine() {
	static char	string[MAX_STRING_CHARS];
	int		l,c;

	l = 0;
	do {
		c = this->readByte();		// use ReadByte so -1 is out of bounds
		if (c == -1 || c == 0 || c == '\n') {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		if ( c > 127 ) {
			c = '.';
		}
		// break only after reading all expected data from bitstream
		if ( l >= sizeof(string)-1 ) {
			break;
		}
		string[l++] = c;
	} while (true);
	
	string[l] = '\0';
	
	return string;
}

float Message::readAngle16() {
	return SHORT2ANGLE(this->readShort());
}

void Message::readData( void *data, int len ) {
	int		i;

	for (i=0 ; i<len ; i++) {
		((byte *)data)[i] = this->readByte();
	}
}

// a string hasher which gives the same hash value even if the
// string is later modified via the legacy MSG read/write code
int Message::hashKey(const char *string, int maxlen) {
	int hash, i;

	hash = 0;
	for (i = 0; i < maxlen && string[i] != '\0'; i++) {
		if (string[i] & 0x80 || string[i] == '%')
			hash += '.' * (119 + i);
		else
			hash += string[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}


/*
=============================================================================

delta functions
  
=============================================================================
*/

extern cvar_t *cl_shownet;

#define	LOG(x) if( cl_shownet && cl_shownet->integer == 4 ) { Com_Printf("%s ", x ); };

/*
=============================================================================

delta functions with keys
  
=============================================================================
*/

int kbitmask[32] = {
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F,	0x0000003F,	0x0000007F,	0x000000FF,
	0x000001FF,	0x000003FF,	0x000007FF,	0x00000FFF,
	0x00001FFF,	0x00003FFF,	0x00007FFF,	0x0000FFFF,
	0x0001FFFF,	0x0003FFFF,	0x0007FFFF,	0x000FFFFF,
	0x001FFFFf,	0x003FFFFF,	0x007FFFFF,	0x00FFFFFF,
	0x01FFFFFF,	0x03FFFFFF,	0x07FFFFFF,	0x0FFFFFFF,
	0x1FFFFFFF,	0x3FFFFFFF,	0x7FFFFFFF,	0xFFFFFFFF,
};

void Message::writeDeltaKey( int key, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		this->writeBits( 0, 1 );
		return;
	}
	this->writeBits( 1, 1 );
	this->writeBits( newV ^ key, bits );
}

int Message::readDeltaKey( int key, int oldV, int bits ) {
	if ( this->readBits( 1 ) ) {
		return this->readBits( bits ) ^ (key & kbitmask[ bits - 1 ]);
	}
	return oldV;
}

void Message::writeDeltaKeyFloat( int key, float oldV, float newV ) {
	floatint_t fi;
	if ( oldV == newV ) {
		this->writeBits( 0, 1 );
		return;
	}
	this->writeBits( 1, 1 );
	fi.f = newV;
	this->writeBits( fi.i ^ key, 32 );
}

float Message::readDeltaKeyFloat( int key, float oldV ) {
	if ( this->readBits( 1 ) ) {
		floatint_t fi;

		fi.i = this->readBits( 32 ) ^ key;
		return fi.f;
	}
	return oldV;
}


/*
============================================================================

usercmd_t communication

============================================================================
*/

/*
=====================
MSG_WriteDeltaUsercmdKey
=====================
*/
void Message::writeDeltaUsercmdKey( int key, usercmd_t *from, usercmd_t *to ) {
	if ( to->serverTime - from->serverTime < 256 ) {
		this->writeBits( 1, 1 );
		this->writeBits( to->serverTime - from->serverTime, 8 );
	} else {
		this->writeBits( 0, 1 );
		this->writeBits( to->serverTime, 32 );
	}
	if (from->angles[0] == to->angles[0] &&
		from->angles[1] == to->angles[1] &&
		from->angles[2] == to->angles[2] &&
		from->forwardmove == to->forwardmove &&
		from->rightmove == to->rightmove &&
		from->upmove == to->upmove &&
		from->buttons == to->buttons &&
		from->weapon == to->weapon) {
			this->writeBits( 0, 1 );				// no change
			oldsize += 7;
			return;
	}
	key ^= to->serverTime;
	this->writeBits( 1, 1 );
	this->writeDeltaKey( key, from->angles[0], to->angles[0], 16 );
	this->writeDeltaKey( key, from->angles[1], to->angles[1], 16 );
	this->writeDeltaKey( key, from->angles[2], to->angles[2], 16 );
	this->writeDeltaKey( key, from->forwardmove, to->forwardmove, 8 );
	this->writeDeltaKey( key, from->rightmove, to->rightmove, 8 );
	this->writeDeltaKey( key, from->upmove, to->upmove, 8 );
	this->writeDeltaKey( key, from->buttons, to->buttons, 16 );
	this->writeDeltaKey( key, from->weapon, to->weapon, 8 );
}


/*
=====================
MSG_ReadDeltaUsercmdKey
=====================
*/
void Message::readDeltaUsercmdKey( int key, usercmd_t *from, usercmd_t *to ) {
	if ( this->readBits( 1 ) ) {
		to->serverTime = from->serverTime + this->readBits( 8 );
	} else {
		to->serverTime = this->readBits( 32 );
	}
	if ( this->readBits( 1 ) ) {
		key ^= to->serverTime;
		to->angles[0] = this->readDeltaKey( key, from->angles[0], 16);
		to->angles[1] = this->readDeltaKey( key, from->angles[1], 16);
		to->angles[2] = this->readDeltaKey( key, from->angles[2], 16);
		to->forwardmove = this->readDeltaKey( key, from->forwardmove, 8);
		if( to->forwardmove == -128 )
			to->forwardmove = -127;
		to->rightmove = this->readDeltaKey( key, from->rightmove, 8);
		if( to->rightmove == -128 )
			to->rightmove = -127;
		to->upmove = this->readDeltaKey( key, from->upmove, 8);
		if( to->upmove == -128 )
			to->upmove = -127;
		to->buttons = this->readDeltaKey( key, from->buttons, 16);
		to->weapon = this->readDeltaKey( key, from->weapon, 8);
	} else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];
		to->angles[2] = from->angles[2];
		to->forwardmove = from->forwardmove;
		to->rightmove = from->rightmove;
		to->upmove = from->upmove;
		to->buttons = from->buttons;
		to->weapon = from->weapon;
	}
}

/*
=============================================================================

entityState_t communication
  
=============================================================================
*/

/*
=================
MSG_ReportChangeVectors_f

Prints out a table from the current statistics for copying to code
=================
*/
void MSG_ReportChangeVectors_f( ) {
	int i;
	for(i=0;i<256;i++) {
		if (pcount[i]) {
			Com_Printf("%d used %d\n", i, pcount[i]);
		}
	}
}

typedef struct {
	char	*name;
	int		offset;
	int		bits;		// 0 = float
} netField_t;

#define	NETF_POS	100
#define	NETF_ANG	101
#define	NETF_POS_BITS	22
#define	NETF_ANG_BITS	16

static bool MSG_QuantizeFloat( int kind, float f, int *q ) {
	float	v;
	int		bits;

	if ( kind == NETF_POS ) {
		v = f * 8.0f;
		bits = NETF_POS_BITS;
	} else {
		v = f * 64.0f;
		bits = NETF_ANG_BITS;
	}
	if ( !( v > -( 1 << ( bits - 1 ) ) + 1 && v < ( 1 << ( bits - 1 ) ) - 1 ) ) {
		return false;
	}
	*q = (int)floorf( v + 0.5f );
	return true;
}

static bool MSG_FieldSame( const netField_t *field, const int *fromF, const int *toF ) {
	int		a, b;

	if ( *fromF == *toF ) {
		return true;
	}
	if ( field->bits < NETF_POS ) {
		return false;
	}
	return MSG_QuantizeFloat( field->bits, *(const float *)fromF, &a )
		&& MSG_QuantizeFloat( field->bits, *(const float *)toF, &b ) && a == b;
}

// using the stringizing operator to save typing...
#define	NETF(x) #x,(int)(intptr_t)&((entityState_t*)0)->x

netField_t	entityStateFields[] = 
{
{ NETF(pos.trTime), 32 },
{ NETF(pos.trBase[0]), NETF_POS },
{ NETF(pos.trBase[1]), NETF_POS },
{ NETF(pos.trDelta[0]), NETF_POS },
{ NETF(pos.trDelta[1]), NETF_POS },
{ NETF(pos.trBase[2]), NETF_POS },
{ NETF(apos.trBase[1]), NETF_ANG },
{ NETF(pos.trDelta[2]), NETF_POS },
{ NETF(apos.trBase[0]), NETF_ANG },
{ NETF(event), 16 },
{ NETF(angles2[1]), NETF_ANG },
{ NETF(eType), 8 },
{ NETF(eventParm), 8 },
{ NETF(tracktarget), 32 },
{ NETF(groundEntityNum), -(GENTITYNUM_BITS+1) },
{ NETF(pos.trType), 8 },
{ NETF(eFlags), 32 },
{ NETF(otherEntityNum), -(GENTITYNUM_BITS+1) },
{ NETF(ONOFF), 32 },
{ NETF(clientNum), 8 },
{ NETF(angles[1]), NETF_ANG },
{ NETF(pos.trDuration), 32 },
{ NETF(apos.trType), 8 },
{ NETF(origin[0]), 0 },
{ NETF(origin[1]), 0 },
{ NETF(origin[2]), 0 },
{ NETF(solid), 24 },
{ NETF(objectives), 8 },
{ NETF(modelindex), 8 },
{ NETF(otherEntityNum2), -(GENTITYNUM_BITS+1) },
{ NETF(loopSound), 16 },
{ NETF(generic1), 8 },
{ NETF(origin2[2]), 0 },
{ NETF(origin2[0]), 0 },
{ NETF(origin2[1]), 0 },
{ NETF(modelindex2), 8 },
{ NETF(angles[0]), NETF_ANG },
{ NETF(time), 32 },
{ NETF(apos.trTime), 32 },
{ NETF(apos.trDuration), 32 },
{ NETF(apos.trBase[2]), NETF_ANG },
{ NETF(apos.trDelta[0]), NETF_POS },
{ NETF(apos.trDelta[1]), NETF_POS },
{ NETF(apos.trDelta[2]), NETF_POS },
{ NETF(weaponIndex), 16 },
{ NETF(angles[2]), NETF_ANG },
{ NETF(angles2[0]), NETF_ANG },
{ NETF(angles2[2]), NETF_ANG },
{ NETF(vehicleAnim), 16 },
{ NETF(frame), 16 },
{ NETF(weaponNum), 16 },
{ NETF(vehicleEnt), GENTITYNUM_BITS },
{ NETF(seat), 4 },
{ NETF(seatMask), 16 },
{ NETF(mountMask), 32 },
{ NETF(revealTime), 32 },
{ NETF(turretYaw[0]), NETF_ANG },
{ NETF(turretYaw[1]), NETF_ANG },
{ NETF(turretYaw[2]), NETF_ANG },
{ NETF(turretYaw[3]), NETF_ANG },
{ NETF(gunPitch[0]), NETF_ANG },
{ NETF(gunPitch[1]), NETF_ANG },
{ NETF(gunPitch[2]), NETF_ANG },
{ NETF(gunPitch[3]), NETF_ANG }
};


// if (int)f == f and (int)f + ( 1<<(FLOAT_INT_BITS-1) ) < ( 1 << FLOAT_INT_BITS )
// the float will be sent with FLOAT_INT_BITS, otherwise all 32 bits will be sent
#define	FLOAT_INT_BITS	13
#define	FLOAT_INT_BIAS	(1<<(FLOAT_INT_BITS-1))

/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message, including the entity number.
Can delta from either a baseline or a previous packet_entity
If to is NULL, a remove entity update will be sent
If force is not set, then nothing at all will be generated if the entity is
identical, under the assumption that the in-order delta code will catch it.
==================
*/
bool Message::writeEntity( struct entityState_s *from, struct entityState_s *to, bool force, int create, uint64_t forced, uint64_t *written ) {
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			trunc;
	float		fullFloat;
	int			*fromF, *toF;
	uint64_t	mask;

	numFields = sizeof(entityStateFields)/sizeof(entityStateFields[0]);

	// all fields should be 32 bits to avoid any compiler packing issues,
	// the "number" field is not part of the field list (hence the "+ 1"),
	// if this assert fails, someone added a field to the entityState_t
	// struct without updating the message fields
	assert( numFields + 1 == sizeof( *from )/4 );

	// a NULL to is a delta remove message
	if ( to == nullptr ) {
		if ( from == nullptr ) {
			return false;
		}
		this->writeBits( from->number, GENTITYNUM_BITS );
		if ( create >= 0 ) {
			this->writeBits( 0, 1 );
		}
		this->writeBits( 1, 1 );
		return true;
	}

	if ( to->number < 0 || to->number >= MAX_GENTITIES ) {
		Com_Error (ERR_FATAL, "MSG_WriteDeltaEntity: Bad entity number: %i", to->number );
	}

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = entityStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( ( ( forced >> i ) & 1 ) || !MSG_FieldSame( field, fromF, toF ) ) {
			lc = i+1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return false;		// nothing at all
		}
		// write two bits for no change
		this->writeBits( to->number, GENTITYNUM_BITS );
		if ( create >= 0 ) {
			this->writeBits( create, 1 );
		}
		this->writeBits( 0, 1 );		// not removed
		this->writeBits( 0, 1 );		// no delta
		return true;
	}

	this->writeBits( to->number, GENTITYNUM_BITS );
	if ( create >= 0 ) {
		this->writeBits( create, 1 );
	}
	this->writeBits( 0, 1 );			// not removed
	this->writeBits( 1, 1 );			// we have a delta

	this->writeByte( lc );	// # of changes

	oldsize += numFields;
	mask = 0;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( !( ( forced >> i ) & 1 ) && MSG_FieldSame( field, fromF, toF ) ) {
			this->writeBits( 0, 1 );	// no change
			continue;
		}

		this->writeBits( 1, 1 );	// changed
		mask |= (uint64_t)1 << i;

		if ( field->bits == 0 || field->bits >= NETF_POS ) {
			// float
			int		quantized;

			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if (fullFloat == 0.0f) {
					this->writeBits( 0, 1 );
					oldsize += FLOAT_INT_BITS;
			} else {
				this->writeBits( 1, 1 );
				if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
					trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
					// send as small integer
					this->writeBits( 0, 1 );
					if ( field->bits >= NETF_POS ) {
						this->writeBits( 0, 1 );
					}
					this->writeBits( trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
				} else if ( field->bits >= NETF_POS && MSG_QuantizeFloat( field->bits, fullFloat, &quantized ) ) {
					this->writeBits( 0, 1 );
					this->writeBits( 1, 1 );
					this->writeBits( quantized, field->bits == NETF_POS ? -NETF_POS_BITS : -NETF_ANG_BITS );
				} else {
					// send as full floating point value
					this->writeBits( 1, 1 );
					this->writeBits( *toF, 32 );
				}
			}
		} else {
			if (*toF == 0) {
				this->writeBits( 0, 1 );
			} else {
				this->writeBits( 1, 1 );
				// integer
				this->writeBits( *toF, field->bits );
			}
		}
	}

	if ( written ) {
		*written = mask;
	}
	return true;
}

void Message::writeDeltaEntity( struct entityState_s *from, struct entityState_s *to, bool force ) {
	this->writeEntity( from, to, force, -1, 0, nullptr );
}

/*
==================
MSG_WriteEntityUpdate

Writes one entity of a snapshot: the fields that differ from what the
client has plus the forced ones. A create is a full state from the baseline.
If to is NULL, a remove entity update will be sent
Returns false when nothing was written
==================
*/
bool Message::writeEntityUpdate( struct entityState_s *from, struct entityState_s *to, bool create, uint64_t forced, uint64_t *written ) {
	if ( written ) {
		*written = 0;
	}
	return this->writeEntity( from, to, create, create ? 1 : 0, forced, written );
}

/*
==================
MSG_ReadDeltaEntity

The entity number has already been read from the message, which
is how the from state is identified.

If the delta removes the entity, entityState_t->number will be set to MAX_GENTITIES-1

Can go from either a baseline or a previous packet_entity
==================
*/
extern	cvar_t	*cl_shownet;

void Message::readDeltaEntity( entityState_t *from, entityState_t *to, int number ) {
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			*fromF, *toF;
	int			print;
	int			trunc;
	int			startBit, endBit;

	if ( number < 0 || number >= MAX_GENTITIES) {
		Com_Error( ERR_DROP, "Bad delta entity number: %i", number );
	}

	if ( this->bit == 0 ) {
		startBit = this->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( this->readcount - 1 ) * 8 + this->bit - GENTITYNUM_BITS;
	}

	// check for a remove
	if ( this->readBits( 1 ) == 1 ) {
		Com_Memset( to, 0, sizeof( *to ) );	
		to->number = ENTITYNUM_WIRE_END;
		if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
			Com_Printf( "%3i: #%-3i remove\n", this->readcount, number );
		}
		return;
	}

	// check for no delta
	if ( this->readBits( 1 ) == 0 ) {
		*to = *from;
		to->number = number;
		return;
	}

	numFields = sizeof(entityStateFields)/sizeof(entityStateFields[0]);
	lc = this->readByte();

	if ( lc > numFields || lc < 0 ) {
		Com_Error( ERR_DROP, "invalid entityState field count" );
	}

	// shownet 2/3 will interleave with other printed info, -1 will
	// just print the delta records`
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) ) {
		print = 1;
		Com_Printf( "%3i: #%-3i ", this->readcount, to->number );
	} else {
		print = 0;
	}

	to->number = number;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( ! this->readBits( 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			if ( field->bits == 0 || field->bits >= NETF_POS ) {
				// float
				if ( this->readBits( 1 ) == 0 ) {
						*(float *)toF = 0.0f; 
				} else {
					if ( this->readBits( 1 ) == 0 ) {
						if ( field->bits >= NETF_POS && this->readBits( 1 ) == 1 ) {
							if ( field->bits == NETF_POS ) {
								*(float *)toF = this->readBits( -NETF_POS_BITS ) / 8.0f;
							} else {
								*(float *)toF = this->readBits( -NETF_ANG_BITS ) / 64.0f;
							}
							if ( print ) {
								Com_Printf( "%s:%f ", field->name, *(float *)toF );
							}
						} else {
							// integral float
							trunc = this->readBits( FLOAT_INT_BITS );
							// bias to allow equal parts positive and negative
							trunc -= FLOAT_INT_BIAS;
							*(float *)toF = trunc; 
							if ( print ) {
								Com_Printf( "%s:%i ", field->name, trunc );
							}
						}
					} else {
						// full floating point value
						*toF = this->readBits( 32 );
						if ( print ) {
							Com_Printf( "%s:%f ", field->name, *(float *)toF );
						}
					}
				}
			} else {
				if ( this->readBits( 1 ) == 0 ) {
					*toF = 0;
				} else {
					// integer
					*toF = this->readBits( field->bits );
					if ( print ) {
						Com_Printf( "%s:%i ", field->name, *toF );
					}
				}
			}
//			pcount[i]++;
		}
	}
	for ( i = lc, field = &entityStateFields[lc] ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}

	// print ?
	if ( print ) {
		if ( this->bit == 0 ) {
			endBit = this->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( this->readcount - 1 ) * 8 + this->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
}


/*
============================================================================

plyer_state_t communication

============================================================================
*/

// using the stringizing operator to save typing...
#define	PSF(x) #x,(int)(intptr_t)&((playerState_t*)0)->x


netField_t	playerStateFields[] = 
{
{ PSF(commandTime), 32 },			
{ PSF(origin[0]), 0 },	
{ PSF(origin[1]), 0 },
{ PSF(throttle), 8 },	
{ PSF(velocity[0]), 0 },	
{ PSF(velocity[1]), 0 },	
{ PSF(viewangles[1]), 0 },
{ PSF(viewangles[0]), 0 },
{ PSF(tracktarget), 32 },	
{ PSF(origin[2]), 0 },
{ PSF(velocity[2]), 0 },	
{ PSF(weaponIndex), 16 },	
{ PSF(pm_time), 32 },	 
{ PSF(eventSequence), 16 },	
{ PSF(objectives), 8 },
{ PSF(vehicleAnim), 16 },	
{ PSF(events[0]), 8 },	
{ PSF(turretAngle), 32 },	
{ PSF(gunAngle), 32 },	
{ PSF(events[1]), 8 },	
{ PSF(pm_flags), 32 },	
{ PSF(groundEntityNum), -(GENTITYNUM_BITS+1) },	
{ PSF(fixed_throttle), 8 },
{ PSF(eFlags), 32 },	
{ PSF(externalEvent), 10 },	
{ PSF(ONOFF), 32 },		
{ PSF(speed), 16 },		
{ PSF(delta_angles[1]), 16 },	
{ PSF(externalEventParm), 8 },	
{ PSF(viewheight), 8 },
{ PSF(damageEvent), 8 },	
{ PSF(damageYaw), 8 },	
{ PSF(damagePitch), 8 },
{ PSF(damageCount), 8 },	
{ PSF(generic1), 8 },	
{ PSF(pm_type), 8 },					
{ PSF(delta_angles[0]), 16 },	
{ PSF(delta_angles[2]), 16 },	
{ PSF(externalEventTime), 32 },	
{ PSF(eventParms[0]), 8 },	
{ PSF(eventParms[1]), 8 },	
{ PSF(clientNum), 8 },	
{ PSF(viewangles[2]), 0 },	
{ PSF(vehicleAngles[0]), 0 },	
{ PSF(vehicleAngles[1]), 0 },	
{ PSF(vehicleAngles[2]), 0 },	
{ PSF(jumppad_ent), 10 },	
{ PSF(loopSound), 16 },
{ PSF(weaponNum), 16 }	
};

/*
=============
MSG_WriteDeltaPlayerstate

=============
*/
void Message::writeDeltaPlayerstate( struct playerState_s *from, struct playerState_s *to ) {
	int				i;
	playerState_t	dummy;
	int				statsbits;
	int				persistantbits;
	int				ammobits;
	int				timerbits;
	long			vwepBits;
	int				numFields;
	int				c;
	netField_t		*field;
	int				*fromF, *toF;
	float			fullFloat;
	int				trunc, lc;

	if (!from) {
		from = &dummy;
		Com_Memset (&dummy, 0, sizeof(dummy));
	}

	c = this->cursize;

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );

	lc = 0;
	for ( i = 0, field = playerStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	this->writeByte( lc );	// # of changes

	oldsize += numFields - lc;

	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			this->writeBits( 0, 1 );	// no change
			continue;
		}

		this->writeBits( 1, 1 );	// changed
//		pcount[i]++;

		if ( field->bits == 0 ) {
			// float
			fullFloat = *(float *)toF;
			trunc = (int)fullFloat;

			if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
				trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
				// send as small integer
				this->writeBits( 0, 1 );
				this->writeBits( trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
			} else {
				// send as full floating point value
				this->writeBits( 1, 1 );
				this->writeBits( *toF, 32 );
			}
		} else {
			// integer
			this->writeBits( *toF, field->bits );
		}
	}
	c = this->cursize - c;


	//
	// send the arrays
	//
	statsbits = 0;
	for (i=0 ; i<16 ; i++) {
		if (to->stats[i] != from->stats[i]) {
			statsbits |= 1<<i;
		}
	}
	persistantbits = 0;
	for (i=0 ; i<16 ; i++) {
		if (to->persistant[i] != from->persistant[i]) {
			persistantbits |= 1<<i;
		}
	}
	ammobits = 0;
	for (i=0 ; i<16 ; i++) {
		if (to->ammo[i] != from->ammo[i]) {
			ammobits |= 1<<i;
		}
	}
	timerbits = 0;
	for (i=0 ; i<16 ; i++) {
		if (to->timers[i] != from->timers[i]) {
			timerbits |= 1<<i;
		}
	}
	// check vwep changes
	vwepBits = 0;
	for (i=0 ; i<32 ; i++) {
		if (to->numWeaponsOnMount[i] != from->numWeaponsOnMount[i]) {
			vwepBits |= 1<<i;
		}
	}

	if (!statsbits && !persistantbits && !ammobits && !timerbits && !vwepBits) {
		this->writeBits( 0, 1 );	// no change
		oldsize += 5;// 4 ?
		return;
	}
	this->writeBits( 1, 1 );	// changed

	if ( statsbits ) {
		this->writeBits( 1, 1 );	// changed
		this->writeShort( statsbits );
		for (i=0 ; i<16 ; i++)
			if (statsbits & (1<<i) )
				this->writeShort( to->stats[i]);
	} else {
		this->writeBits( 0, 1 );	// no change
	}


	if ( persistantbits ) {
		this->writeBits( 1, 1 );	// changed
		this->writeShort( persistantbits );
		for (i=0 ; i<16 ; i++)
			if (persistantbits & (1<<i) )
				this->writeShort( to->persistant[i]);
	} else {
		this->writeBits( 0, 1 );	// no change
	}


	if ( ammobits ) {
		this->writeBits( 1, 1 );	// changed
		this->writeShort( ammobits );
		for (i=0 ; i<16 ; i++)
			if (ammobits & (1<<i) )
				this->writeShort( to->ammo[i]);
	} else {
		this->writeBits( 0, 1 );	// no change
	}


	if ( timerbits ) {
		this->writeBits( 1, 1 );	// changed
		this->writeShort( timerbits );
		for (i=0 ; i<16 ; i++)
			if (timerbits & (1<<i) )
				this->writeLong( to->timers[i] );
	} else {
		this->writeBits( 0, 1 );	// no change
	}

	// numWeaponsOnMount array
	if ( vwepBits ) {
		this->writeBits( 1, 1 );	// changed
		this->writeLong( vwepBits );
		for (i=0 ; i<32 ; i++)
			if (vwepBits & (1<<i) )
				this->writeBits( to->numWeaponsOnMount[i], 6);
	} else {
		this->writeBits( 0, 1 );	// no change
	}
}


/*
===================
MSG_ReadDeltaPlayerstate
===================
*/
void Message::readDeltaPlayerstate( playerState_t *from, playerState_t *to ) {
	int			i, lc;
	int			bits;
	netField_t	*field;
	int			numFields;
	int			startBit, endBit;
	int			print;
	int			*fromF, *toF;
	int			trunc;
	playerState_t	dummy;
	long		vwepBits;

	if ( !from ) {
		from = &dummy;
		Com_Memset( &dummy, 0, sizeof( dummy ) );
	}
	*to = *from;

	if ( this->bit == 0 ) {
		startBit = this->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( this->readcount - 1 ) * 8 + this->bit - GENTITYNUM_BITS;
	}

	// shownet 2/3 will interleave with other printed info, -2 will
	// just print the delta records
	if ( cl_shownet && ( cl_shownet->integer >= 2 || cl_shownet->integer == -2 ) ) {
		print = 1;
		Com_Printf( "%3i: playerstate ", this->readcount );
	} else {
		print = 0;
	}

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );
	lc = this->readByte();

	if ( lc > numFields || lc < 0 ) {
		Com_Error( ERR_DROP, "invalid playerState field count" );
	}

	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( ! this->readBits( 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			if ( field->bits == 0 ) {
				// float
				if ( this->readBits( 1 ) == 0 ) {
					// integral float
					trunc = this->readBits( FLOAT_INT_BITS );
					// bias to allow equal parts positive and negative
					trunc -= FLOAT_INT_BIAS;
					*(float *)toF = trunc; 
					if ( print ) {
						Com_Printf( "%s:%i ", field->name, trunc );
					}
				} else {
					// full floating point value
					*toF = this->readBits( 32 );
					if ( print ) {
						Com_Printf( "%s:%f ", field->name, *(float *)toF );
					}
				}
			} else {
				// integer
				*toF = this->readBits( field->bits );
				if ( print ) {
					Com_Printf( "%s:%i ", field->name, *toF );
				}
			}
		}
	}
	for ( i=lc,field = &playerStateFields[lc];i<numFields; i++, field++) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}


	// read the arrays
	if (this->readBits( 1 ) ) {
		// parse stats
		if ( this->readBits( 1 ) ) {
			LOG("PS_STATS");
			bits = this->readShort();
			for (i=0 ; i<16 ; i++) {
				if (bits & (1<<i) ) {
					to->stats[i] = this->readShort();
				}
			}
		}

		// parse persistant stats
		if ( this->readBits( 1 ) ) {
			LOG("PS_PERSISTANT");
			bits = this->readShort();
			for (i=0 ; i<16 ; i++) {
				if (bits & (1<<i) ) {
					to->persistant[i] = this->readShort();
				}
			}
		}

		// parse ammo
		if ( this->readBits( 1 ) ) {
			LOG("PS_AMMO");
			bits = this->readShort();
			for (i=0 ; i<16 ; i++) {
				if (bits & (1<<i) ) {
					to->ammo[i] = this->readShort();
				}
			}
		}

		// parse timers
		if ( this->readBits( 1 ) ) {
			LOG("PS_TIMERS");
			bits = this->readShort();
			for (i=0 ; i<16 ; i++) {
				if (bits & (1<<i) ) {
					to->timers[i] = this->readLong();
				}
			}
		}

		// parse vwep array
		if ( this->readBits( 1 ) ) {
			LOG("PS_VWEP");
			vwepBits = this->readLong();
			for (i=0 ; i<32 ; i++) {
				if (vwepBits & (1<<i) ) {
					to->numWeaponsOnMount[i] = this->readBits( 6);
				}
			}
		}

	}

	if ( print ) {
		if ( this->bit == 0 ) {
			endBit = this->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( this->readcount - 1 ) * 8 + this->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
}

int msg_hData[256] = {
250315,			// 0
41193,			// 1
6292,			// 2
7106,			// 3
3730,			// 4
3750,			// 5
6110,			// 6
23283,			// 7
33317,			// 8
6950,			// 9
7838,			// 10
9714,			// 11
9257,			// 12
17259,			// 13
3949,			// 14
1778,			// 15
8288,			// 16
1604,			// 17
1590,			// 18
1663,			// 19
1100,			// 20
1213,			// 21
1238,			// 22
1134,			// 23
1749,			// 24
1059,			// 25
1246,			// 26
1149,			// 27
1273,			// 28
4486,			// 29
2805,			// 30
3472,			// 31
21819,			// 32
1159,			// 33
1670,			// 34
1066,			// 35
1043,			// 36
1012,			// 37
1053,			// 38
1070,			// 39
1726,			// 40
888,			// 41
1180,			// 42
850,			// 43
960,			// 44
780,			// 45
1752,			// 46
3296,			// 47
10630,			// 48
4514,			// 49
5881,			// 50
2685,			// 51
4650,			// 52
3837,			// 53
2093,			// 54
1867,			// 55
2584,			// 56
1949,			// 57
1972,			// 58
940,			// 59
1134,			// 60
1788,			// 61
1670,			// 62
1206,			// 63
5719,			// 64
6128,			// 65
7222,			// 66
6654,			// 67
3710,			// 68
3795,			// 69
1492,			// 70
1524,			// 71
2215,			// 72
1140,			// 73
1355,			// 74
971,			// 75
2180,			// 76
1248,			// 77
1328,			// 78
1195,			// 79
1770,			// 80
1078,			// 81
1264,			// 82
1266,			// 83
1168,			// 84
965,			// 85
1155,			// 86
1186,			// 87
1347,			// 88
1228,			// 89
1529,			// 90
1600,			// 91
2617,			// 92
2048,			// 93
2546,			// 94
3275,			// 95
2410,			// 96
3585,			// 97
2504,			// 98
2800,			// 99
2675,			// 100
6146,			// 101
3663,			// 102
2840,			// 103
14253,			// 104
3164,			// 105
2221,			// 106
1687,			// 107
3208,			// 108
2739,			// 109
3512,			// 110
4796,			// 111
4091,			// 112
3515,			// 113
5288,			// 114
4016,			// 115
7937,			// 116
6031,			// 117
5360,			// 118
3924,			// 119
4892,			// 120
3743,			// 121
4566,			// 122
4807,			// 123
5852,			// 124
6400,			// 125
6225,			// 126
8291,			// 127
23243,			// 128
7838,			// 129
7073,			// 130
8935,			// 131
5437,			// 132
4483,			// 133
3641,			// 134
5256,			// 135
5312,			// 136
5328,			// 137
5370,			// 138
3492,			// 139
2458,			// 140
1694,			// 141
1821,			// 142
2121,			// 143
1916,			// 144
1149,			// 145
1516,			// 146
1367,			// 147
1236,			// 148
1029,			// 149
1258,			// 150
1104,			// 151
1245,			// 152
1006,			// 153
1149,			// 154
1025,			// 155
1241,			// 156
952,			// 157
1287,			// 158
997,			// 159
1713,			// 160
1009,			// 161
1187,			// 162
879,			// 163
1099,			// 164
929,			// 165
1078,			// 166
951,			// 167
1656,			// 168
930,			// 169
1153,			// 170
1030,			// 171
1262,			// 172
1062,			// 173
1214,			// 174
1060,			// 175
1621,			// 176
930,			// 177
1106,			// 178
912,			// 179
1034,			// 180
892,			// 181
1158,			// 182
990,			// 183
1175,			// 184
850,			// 185
1121,			// 186
903,			// 187
1087,			// 188
920,			// 189
1144,			// 190
1056,			// 191
3462,			// 192
2240,			// 193
4397,			// 194
12136,			// 195
7758,			// 196
1345,			// 197
1307,			// 198
3278,			// 199
1950,			// 200
886,			// 201
1023,			// 202
1112,			// 203
1077,			// 204
1042,			// 205
1061,			// 206
1071,			// 207
1484,			// 208
1001,			// 209
1096,			// 210
915,			// 211
1052,			// 212
995,			// 213
1070,			// 214
876,			// 215
1111,			// 216
851,			// 217
1059,			// 218
805,			// 219
1112,			// 220
923,			// 221
1103,			// 222
817,			// 223
1899,			// 224
1872,			// 225
976,			// 226
841,			// 227
1127,			// 228
956,			// 229
1159,			// 230
950,			// 231
7791,			// 232
954,			// 233
1289,			// 234
933,			// 235
1127,			// 236
3207,			// 237
1020,			// 238
927,			// 239
1355,			// 240
768,			// 241
1040,			// 242
745,			// 243
952,			// 244
805,			// 245
1073,			// 246
740,			// 247
1013,			// 248
805,			// 249
1008,			// 250
796,			// 251
996,			// 252
1057,			// 253
11457,			// 254
13504,			// 255
};

void MSG_initHuffman() {
	int i,j;

	msgInit = true;
	Huff_Init(&msgHuff);
	for(i=0;i<256;i++) {
		for (j=0;j<msg_hData[i];j++) {
			Huff_addRef(&msgHuff.compressor,	(byte)i);			// Do update
			Huff_addRef(&msgHuff.decompressor,	(byte)i);			// Do update
		}
	}
}

/*
==================
MSG_HuffStats_f

Writes the byte counts gathered with msg_huffstats 1 as a msg_hData table
==================
*/
void MSG_HuffStats_f( ) {
	char	out[256 * 32];
	int		i;

	out[0] = 0;
	for ( i = 0 ; i < 256 ; i++ ) {
		Q_strcat( out, sizeof( out ), va( "%d,\t\t\t// %d\n", msg_huffCounts[i], i ) );
	}
	FS_WriteFile( "huffstats.txt", out, strlen( out ) );
	Com_Printf( "wrote huffstats.txt\n" );
}

void MSG_InitHuffStats( ) {
	msg_huffstats = Cvar_Get( "msg_huffstats", "0", CVAR_TEMP );
	Cmd_AddCommand( "huffstats", MSG_HuffStats_f );
}

//===========================================================================
