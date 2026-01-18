#include <genesis.h>
#include <string.h>

#include "player.h"
#include "res_sprites.h"

/*The edges of the play field*/
// TODO: move to a location that makes more sense
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 100;

const int STANDING_ANIM = 0;
const int WALKING_ANIM = 1;
const int DODGING_ANIM = 2;
const int JUMPING_ANIM = 3;

const int WALKING_SPEED = 2; // pixels/frame
const int FALLING_X_SPEED = 1;
const fix16 GRAVITY = FIX16(0.24);
const fix16 JUMP_SPEED = FIX16(3.5);
const u8 DEFAULT_DODGE_COOLDOWN = 54;
const u8 MAX_DODGE_FRAMES = 30;

// player sprite
Sprite *player;
Sprite *airJump;

// states
s8 xOrder = 0;
s8 yOrder = 0;
int player_vel_x = 0;
int player_pos_x = 40;
fix16 player_vel_y = FIX16(0);
fix16 player_pos_y = FIX16(100);
const int PLAYER_WIDTH = 24;
const int PLAYER_HEIGHT = 32;
s8 player_direction = RIGHT;
u8 dodgeFrames = 0;
u8 dodgeCooldown = 0;
u8 jumpsLeft = 2;
u8 airJumpSpriteCooldown = 0;

static void jump(void);
static void stand(void);
static void run(s8 direction);
static void positionPlayer(void);
static void handleDodge(void);
static bool airborne(void);
static void debug(int value);
static void pollDpad(void);

/*
gets called upon game reset
*/
u16 PLAYER_init(u16 vramIndex)
{
    SPR_init();
    player = SPR_addSprite(&player_sprite, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, FALSE));
    airJump = SPR_addSprite(&player_airJump, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    SPR_setVisibility(airJump, HIDDEN);

    return vramIndex; // static vram allocation not used for player
}

/*
gets called every frame by the game driver
*/
void PLAYER_update(void)
{
    pollDpad();
    handleDodge();
    positionPlayer();
    airJumpSpriteCooldown > 0 ? airJumpSpriteCooldown-- : SPR_setVisibility(airJump, HIDDEN);
    debug(player_direction);
}

void debug(int value)
{
    char str_value[10];
    sprintf(str_value, "%d", value);
    VDP_clearText(1, 2, 10);
    VDP_drawText(str_value, 1, 2);
}

void pollDpad()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_UP)
        yOrder = -1;
    else if (value & BUTTON_DOWN)
        yOrder = +1;
    else
        yOrder = 0;

    if (value & BUTTON_LEFT)
        xOrder = -1;
    else if (value & BUTTON_RIGHT)
        xOrder = +1;
    else
        xOrder = 0;
}

void positionPlayer()
{
    // calculate velocity
    if (xOrder == 0 && dodgeFrames == 0 && player_vel_y == 0)
        stand();
    else if (airborne() && ((xOrder == RIGHT && player_direction == LEFT) || (xOrder == LEFT && player_direction == RIGHT))) // changing direction while falling
    {
        player_vel_x = xOrder * FALLING_X_SPEED; // go slower if changing direction while falling
        player_direction = xOrder;
        player_direction < 0 ? SPR_setHFlip(player, TRUE) : SPR_setHFlip(player, FALSE);
        VDP_drawText("ENTERING AIRBORNE BLOCK",1,3);
    }
    else if (dodgeFrames == 0 && xOrder != 0 && !airborne()) // on the ground, not rolling, with dpad input
    {
        player_vel_x = xOrder * WALKING_SPEED;
        run(xOrder);
    }

    // Add the player's velocity to its position
    player_pos_x += player_vel_x;
    player_pos_y += player_vel_y;

    // apply gravity
    if (airborne())
        player_vel_y += GRAVITY;
    else
        player_vel_y = 0;

    // handle falling
    if (dodgeFrames == 0 && player_vel_y > FIX16(0))
    {
        SPR_setAutoAnimation(player, FALSE);
        SPR_setAnimAndFrame(player, JUMPING_ANIM, 2); // last animation of the jump
    }
    else
    {
        SPR_setAutoAnimation(player, TRUE);
    }

    // Keep the player within the bounds of the screen
    if (player_pos_x < LEFT_EDGE)
        player_pos_x = LEFT_EDGE;
    if (player_pos_x + PLAYER_WIDTH > RIGHT_EDGE)
        player_pos_x = RIGHT_EDGE - PLAYER_WIDTH;
    if (player_pos_y > F16(BOTTOM_EDGE))
        player_pos_y = F16(BOTTOM_EDGE);

    // Let the Sprite engine position the sprite
    SPR_setPosition(player, player_pos_x, F16_toRoundedInt(player_pos_y));
}

void handleDodge()
{

    if (dodgeFrames > 0)
        dodgeFrames--;
    if (dodgeCooldown > 0)
        dodgeCooldown--;

    // dodge interrupts
    if ((xOrder == RIGHT && player_direction == LEFT) || (xOrder == LEFT && player_direction == RIGHT))
        dodgeFrames = 0;
}

/*
pressed = 1 if pressed and 0 if not
changed = 1 if state is different than last frame's state, 0 if not
*/
void PLAYER_doJoyAction(u16 changed, u16 pressed)
{
    // NOTE: need to use bitwise operators here, since enums are hex

    // handle uninterruptible motions
    if (FALSE)
    {
        return;
    }

    if (changed & pressed & BUTTON_C)
    {
        if (dodgeCooldown > 0)
            return;                                      // cannot roll until cooldown timer expires
        player_vel_x = player_direction * WALKING_SPEED; // roll at roll speed, even if starting from rest
        SPR_setAnim(player, DODGING_ANIM);
        dodgeFrames = MAX_DODGE_FRAMES;
        dodgeCooldown = DEFAULT_DODGE_COOLDOWN;
    }
    else if (changed & pressed & BUTTON_B)
    {
        jump();
        dodgeFrames = 0; // dodge cancel
    }
}

void run(s8 direction)
{
    player_direction = direction;
    player_vel_x = player_direction * WALKING_SPEED; // pixel/frame
    SPR_setAnim(player, WALKING_ANIM);
    player_direction < 0 ? SPR_setHFlip(player, TRUE) : SPR_setHFlip(player, FALSE);
}

void stand()
{
    player_vel_x = 0;
    SPR_setAnim(player, STANDING_ANIM);
}

void jump()
{
    if (!airborne()) // ground jump
    {
        jumpsLeft = 2;
        SPR_setAnim(player, JUMPING_ANIM);
        jumpsLeft--;
        player_vel_y = -JUMP_SPEED;
    }
    else if (airborne() && jumpsLeft > 0) // air jump
    {
        SPR_setAnimAndFrame(player, JUMPING_ANIM, 0);
        SPR_setPosition(airJump, player_pos_x - 8, F16_toRoundedInt(player_pos_y) + PLAYER_HEIGHT - 4);
        SPR_setVisibility(airJump, VISIBLE);
        airJumpSpriteCooldown = 7;
        jumpsLeft--;
        player_vel_y = -JUMP_SPEED;
    }
    else
    {
        // do nothing
    }
}

bool airborne()
{
    return F16_toRoundedInt(player_pos_y) < 100;
}
