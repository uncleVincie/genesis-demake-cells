#ifndef _PLAYER_H_
#define _PLAYER_H_

#define LEFT    -1
#define RIGHT   1
#define TERMINAL_VELOCITY 8
#define STOMP_VELOCITY 16

//public functions only!
u16 PLAYER_init(u16 vramIndex);
void PLAYER_update(void);
void PLAYER_doJoyAction(u16 changed, u16 pressed);

#endif