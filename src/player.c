#include <genesis.h>
#include <string.h>

#include "player.h"
#include "res_sprites.h"

/*The edges of the play field*/
// TODO: move to a location that makes more sense
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

const int WALKING_SPEED = 2;

const int STANDING_ANIM = 0;
const int WALKING_ANIM = 1;
const int DODGING_ANIM = 2;

// player sprite
Sprite *player;

// states
int player_vel_x = 0;
int player_pos_x = 40;
const int player_pos_y = 100;
const int player_width = 24;
const int player_height = 32;
s8 player_direction = RIGHT;
u8 dodgeFrames = 0;
u8 dodgeCooldown = 60;
// char str_dodgeFrames[2] = "0";
// char str_playerDirection[2] = "0";

// static void jump(void);
static void stand(void);
static void run(s8 direction);
static void positionPlayer(void);
static void handleDodge(void);

/*
gets called upon game reset
*/
u16 PLAYER_init(u16 vramIndex)
{
    SPR_init();
    player = SPR_addSprite(&player_sprite, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, FALSE));

    return vramIndex; // static vram allocation not used for player
}

/*
gets called every frame by the game driver
*/
void PLAYER_update(void)
{
    handleDodge();
    positionPlayer();
}

void positionPlayer()
{
    // Add the player's velocity to its position
    player_pos_x += player_vel_x;

    // Keep the player within the bounds of the screen
    if (player_pos_x < LEFT_EDGE)
        player_pos_x = LEFT_EDGE;
    if (player_pos_x + player_width > RIGHT_EDGE)
        player_pos_x = RIGHT_EDGE - player_width;

    // Let the Sprite engine position the sprite
    SPR_setPosition(player, player_pos_x, player_pos_y);
}

void handleDodge()
{
    if (dodgeFrames > 0) dodgeFrames--;
    if (dodgeCooldown > 0) dodgeCooldown--;

    if (dodgeFrames == 1)
    {
        // poll controller for held input
        u16 value = JOY_readJoypad(JOY_1);
        if ((value & BUTTON_LEFT) && player_direction == LEFT)
        {
            //if LEFT or RIGHT is held, continue run
            run(LEFT);
        } else if ((value & BUTTON_RIGHT) && player_direction == RIGHT)
        {
            run(RIGHT);
        } else
        {
            // no held d-pad, make player stop and stand
            stand();
        }
    }
    // debug stuff
    // sprintf(str_dodgeFrames,"%d",dodgeFrames);
    // sprintf(str_playerDirection,"%d",player_direction);
    // VDP_clearText(1,2,2);
    // VDP_clearText(1,3,2);
    // VDP_drawText(str_dodgeFrames,1,2);
    // VDP_drawText(str_playerDirection,1,3);
}

/*
pressed = 1 if pressed and 0 if not
changed = 1 if state is different than last frame's state, 0 if not
*/
void PLAYER_doJoyAction(u16 changed, u16 pressed)
{
    //NOTE: need to use bitwise operators here, since enums are hex

    // handle uninterruptible motions
    if (FALSE)
    {
        return;
    }

    if (changed & pressed & BUTTON_C)
    {
        if (dodgeCooldown > 0) return; //cannot roll until cooldown timer expires
        player_vel_x = player_direction * WALKING_SPEED; // roll at roll speed, even if starting from rest
        SPR_setAnim(player, DODGING_ANIM);
        dodgeFrames = 30;
        dodgeCooldown = 60;
    }
    else if (pressed & BUTTON_RIGHT)
    {
        if (player_direction == RIGHT && dodgeFrames > 0) return; //if already dodging in this direction
        run(RIGHT);
        dodgeFrames = 0; //cancel dodge frames due to change in direction
    }
    else if (pressed & BUTTON_LEFT)
    {
        if (player_direction == LEFT && dodgeFrames > 0) return; //if already dodging in this direction
        run(LEFT);
        dodgeFrames = 0; //cancel dodge frames due to change in direction
    }
    else
    {
        if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT))
        {
            stand();
            dodgeFrames = 0;
        }
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
