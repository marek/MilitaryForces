/*
 * $Id: syn.h,v 1.2 2005-09-02 08:00:21 thebjoern Exp $
*/

//===========================================================================
//
// Name:			syn.h
// Function:		synonyms
// Programmer:		Mr Elusive
// Last update:		-
// Tab Size:		4 (real tabs)
// Notes:			-
//===========================================================================

#ifndef __SYN_H__
#define __SYN_H__

inline constexpr unsigned int CONTEXT_ALL = 0xFFFFFFFF;
inline constexpr int CONTEXT_NORMAL = 1;
inline constexpr int CONTEXT_NEARBYITEM = 2;
inline constexpr int CONTEXT_CTFREDTEAM = 4;
inline constexpr int CONTEXT_CTFBLUETEAM = 8;
inline constexpr int CONTEXT_REPLY = 16;
inline constexpr int CONTEXT_OBELISKREDTEAM = 32;
inline constexpr int CONTEXT_OBELISKBLUETEAM = 64;
inline constexpr int CONTEXT_HARVESTERREDTEAM = 128;
inline constexpr int CONTEXT_HARVESTERBLUETEAM = 256;

inline constexpr int CONTEXT_NAMES = 1024;

#endif // __SYN_H__
