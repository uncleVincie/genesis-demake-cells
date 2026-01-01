#include <genesis.h>

#ifndef _PLAYER_H_
#define _PLAYER_H_

//public functions only!
u16 PLAYER_init(u16 vramIndex);
void PLAYER_updateScreenPosition(void);
void PLAYER_doJoyAction(u16 changed, u16 pressed);

#endif