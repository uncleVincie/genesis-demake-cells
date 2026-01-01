#include "player.h"
#include "res_sprites.h"

/*The edges of the play field*/
//TODO: move to a location that makes more sense
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

const int WALKING_SPEED = 2;

const int STANDING_ANIM = 0;
const int WALKING_ANIM = 1;
const int DODGING_ANIM = 2;

// player sprite
Sprite* player;

// states
int player_vel_x = 0;
int player_pos_x = 40;
const int player_pos_y = 100;
const int player_width = 24;
const int player_height = 32;

// static void jump(void);
// static void dodge(void);
// static void run(void);

u16 PLAYER_init(u16 vramIndex)
{
    SPR_init();
    player = SPR_addSprite(&player_sprite, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, FALSE));

    return vramIndex; // static vram allocation not used for player
}

void PLAYER_updateScreenPosition(void)
{
    /*Add the player's velocity to its position*/
    player_pos_x += player_vel_x;

    /*Keep the player within the bounds of the screen*/
    if (player_pos_x < LEFT_EDGE)
        player_pos_x = LEFT_EDGE;
    if (player_pos_x + player_width > RIGHT_EDGE)
        player_pos_x = RIGHT_EDGE - player_width;

    /*Let the Sprite engine position the sprite*/
    SPR_setPosition(player, player_pos_x, player_pos_y);
}

/*
pressed = 1 if pressed and 0 if not
changed = 1 if state is different than last frame's state, 0 if not
*/
void PLAYER_doJoyAction(u16 changed, u16 pressed)
{
    /*Set player velocity if left or right are pressed;
         *set velocity to 0 if no direction is pressed
         NOTE: need to use bitwise operators here, since enums are hex */
    if (pressed & BUTTON_RIGHT)
    {
        player_vel_x = 2; // pixel/frame
        SPR_setAnim(player, 1);
        SPR_setHFlip(player, FALSE);
    }
    else if (pressed & BUTTON_LEFT)
    {
        player_vel_x = -2;
        SPR_setAnim(player, 1);
        SPR_setHFlip(player, TRUE);
    }
    else
    {
        if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT))
        {
            player_vel_x = 0;
            SPR_setAnim(player, 0);
        }
    }
}
