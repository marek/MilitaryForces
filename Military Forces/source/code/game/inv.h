/*
 * $Id: inv.h,v 1.2 2005-09-02 08:00:21 thebjoern Exp $
*/

#ifdef __INV_H__
#define __INV_H__

inline constexpr int INVENTORY_NONE = 0;
//ammo
inline constexpr int INVENTORY_SHELLS = 18;
inline constexpr int INVENTORY_BULLETS = 19;
inline constexpr int INVENTORY_ROCKETS = 23;
inline constexpr int INVENTORY_SLUGS = 24;
//powerups
inline constexpr int INVENTORY_REDFLAG = 45;
inline constexpr int INVENTORY_BLUEFLAG = 46;
// MFQ3
inline constexpr int INVENTORY_FFAR = 50;
inline constexpr int INVENTORY_IRONBOMB = 51;
inline constexpr int INVENTORY_AUTOCANNON = 52;
inline constexpr int INVENTORY_MAINGUN = 53;
//enemy stuff
inline constexpr int ENEMY_HORIZONTAL_DIST = 200;
inline constexpr int ENEMY_HEIGHT = 201;
inline constexpr int NUM_VISIBLE_ENEMIES = 202;
inline constexpr int NUM_VISIBLE_TEAMMATES = 203;

//item numbers (make sure they are in sync with bg_itemlist in bg_misc.c)
inline constexpr int MODELINDEX_HEALTHSMALL = 1;
inline constexpr int MODELINDEX_HEALTH = 2;
inline constexpr int MODELINDEX_HEALTHLARGE = 3;
inline constexpr int MODELINDEX_HEALTHMEGA = 4;

inline constexpr int MODELINDEX_FFAR = 5;
inline constexpr int MODELINDEX_IRONBOMB = 6;
inline constexpr int MODELINDEX_AUTOCANNON = 7;
inline constexpr int MODELINDEX_MAINGUN = 8;

inline constexpr int MODELINDEX_SHELLS = 9;
inline constexpr int MODELINDEX_BULLETS = 10;
inline constexpr int MODELINDEX_ROCKETS = 11;
inline constexpr int MODELINDEX_SLUGS = 12;

inline constexpr int MODELINDEX_REDFLAG = 13;
inline constexpr int MODELINDEX_BLUEFLAG = 14;

// MFQ3
inline constexpr int WEAPONINDEX_FFAR = 1;
inline constexpr int WEAPONINDEX_IRONBOMB = 2;
inline constexpr int WEAPONINDEX_AUTOCANNON = 3;
inline constexpr int WEAPONINDEX_MAINGUN = 4;


#endif // __INV_H__
