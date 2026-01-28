#ifndef _PLAYER_H_
#define _PLAYER_H_

#define LEFT    -1
#define RIGHT   1
#define TERMINAL_VELOCITY   FIX16(7)
#define STOMP_VELOCITY  FIX16(8)

#define STANDING_ANIM   0
#define WALKING_ANIM    1
#define DODGING_ANIM    2
#define JUMPING_ANIM    3
#define STOMPING_ANIM   4
#define HEALING_ANIM    5

#define WALKING_SPEED   2 // pixels/frame
#define FALLING_X_SPEED 1
#define GRAVITY     FIX16(0.24)
#define JUMP_SPEED  FIX16(3.5)
#define DEFAULT_DODGE_COOLDOWN  54
#define MAX_DODGE_FRAMES    30
#define STOMP_RECOVERY_FRAMES   24
#define MAX_HEAL_COUNTER 44
#define MIN_HEAL_COUNTER -36

#define PLAYER_WIDTH    24
#define PLAYER_HEIGHT   32

//public functions only!
u16 PLAYER_init(u16 vramIndex);
void PLAYER_update(void);
void PLAYER_doJoyAction(u16 changed, u16 pressed);

#endif