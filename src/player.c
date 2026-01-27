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

// player sprite
Sprite *player;
Sprite *airJump;
Sprite *stompExplosionR;
Sprite *stompExplosionL;

// states
s8 xOrder = 0;
s8 yOrder = 0;
int player_vel_x = 0;
int player_pos_x = 40;
fix16 player_vel_y = FIX16(0);
fix16 player_pos_y = FIX16(100);

s8 player_direction = RIGHT;
u8 dodgeFrames = 0;
u8 dodgeCooldown = 0;
u8 jumpsLeft = 2;
u8 airJumpSpriteCooldown = 0;
u8 stompRecovery = 0;
u8 stompExplosionTimer = 0;
s8 healCounter = MAX_HEAL_COUNTER;

static void jump(void);
static void stomp(void);
static void stand(void);
static void run(s8 direction);
static void positionPlayer(void);
static void handleDodge(void);
static void handleStompRecovery(void);
static bool airborne(void);
static bool stomping(void);
static void debug(int value);
static void pollDpad(void);
static void fixinToHeal(void);

/*
gets called upon game reset
*/
u16 PLAYER_init(u16 vramIndex)
{
    SPR_init();
    player = SPR_addSprite(&player_sprite, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 1, FALSE, FALSE));
    airJump = SPR_addSprite(&player_airJump, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    stompExplosionR = SPR_addSprite(&player_stomp, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, FALSE));
    stompExplosionL = SPR_addSprite(&player_stomp, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, TRUE));
    SPR_setVisibility(airJump, HIDDEN);
    SPR_setVisibility(stompExplosionR, HIDDEN);
    SPR_setVisibility(stompExplosionL, HIDDEN);
    SPR_setAnimationLoop(stompExplosionR,FALSE);
    SPR_setAnimationLoop(stompExplosionL,FALSE);

    return vramIndex; // static vram allocation not used for player
}

/*
gets called every frame by the game driver
*/
void PLAYER_update(void)
{
    pollDpad();
    handleDodge();
    handleStompRecovery();
    positionPlayer();
    airJumpSpriteCooldown > 0 ? airJumpSpriteCooldown-- : SPR_setVisibility(airJump, HIDDEN);
    debug(healCounter);
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

    if (value & BUTTON_Z)
    {
        //lock out d-pad inputs
        xOrder = 0;
        yOrder = 0;
        if (!airborne()) fixinToHeal();
    }
    else
    {
        if (healCounter > 0) healCounter = MAX_HEAL_COUNTER;
    }

}

void positionPlayer()
{
    if (0 < healCounter && healCounter < MAX_HEAL_COUNTER)
    {
        //let healing function handle this, but don't allow any other animation
    }
    else if (healCounter <= 0 && healCounter > MIN_HEAL_COUNTER)
    {
        SPR_setAutoAnimation(player, FALSE);
        SPR_setAnimAndFrame(player, HEALING_ANIM, 1);
        healCounter--;
    }
    else if (healCounter == MIN_HEAL_COUNTER)
    {
        SPR_setAutoAnimation(player, TRUE);
        healCounter = MAX_HEAL_COUNTER;
    }
    else if (xOrder == 0 && dodgeFrames == 0 && player_vel_y == 0 && stompRecovery == 0)
        stand();
    else if (airborne() && ((xOrder == RIGHT && player_direction == LEFT) || (xOrder == LEFT && player_direction == RIGHT)) && !stomping()) // changing direction while falling
    {
        player_vel_x = xOrder * FALLING_X_SPEED; // go slower if changing direction while falling
        player_direction = xOrder;
        player_direction < 0 ? SPR_setHFlip(player, TRUE) : SPR_setHFlip(player, FALSE);
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
    {
        if (player_vel_y < TERMINAL_VELOCITY) player_vel_y += GRAVITY;
    } else
    {
        player_vel_y = 0;
    }

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

    // handle stomp recovery
    if (!airborne() && stompRecovery > 0) // if player landed after stomping
    {
        SPR_setAutoAnimation(player, FALSE);
        SPR_setAnimAndFrame(player, STOMPING_ANIM, 0);
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

void handleStompRecovery()
{
    if (stompRecovery == STOMP_RECOVERY_FRAMES && !airborne()) {
        stompExplosionTimer = STOMP_RECOVERY_FRAMES;
        SPR_setPosition(stompExplosionR, player_pos_x+PLAYER_WIDTH/2, F16_toRoundedInt(player_pos_y)+PLAYER_HEIGHT-56);
        SPR_setPosition(stompExplosionL, player_pos_x+PLAYER_WIDTH/2-56, F16_toRoundedInt(player_pos_y)+PLAYER_HEIGHT-56);
        SPR_setAnimAndFrame(stompExplosionL, 0, 0);
        SPR_setAnimAndFrame(stompExplosionR, 0, 0);
        SPR_setVisibility(stompExplosionR, VISIBLE);
        SPR_setVisibility(stompExplosionL, VISIBLE);
    }

    if (stompExplosionTimer > 0) stompExplosionTimer--;

    if (stompExplosionTimer == 0)
    {
        SPR_setVisibility(stompExplosionR, HIDDEN);
        SPR_setVisibility(stompExplosionL, HIDDEN);
    } 

    if (stompRecovery > 0 && !airborne())
        stompRecovery--;

    //stomp interrupts
    if (abs(xOrder) > 0 && !airborne())
        stompRecovery = 0;
}

/*
pressed = 1 if pressed and 0 if not
changed = 1 if state is different than last frame's state, 0 if not
*/
void PLAYER_doJoyAction(u16 changed, u16 pressed)
{
    // NOTE: need to use bitwise operators here, since enums are hex

    // handle uninterruptible motions
    if (healCounter < MAX_HEAL_COUNTER)
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
        stompRecovery = 0;
    }
    else if (changed & pressed & BUTTON_B)
    {
        if (yOrder == 1 && airborne()) //if pressing down in the air
        {
            stomp();
        } else
        {
            jump();
            stompRecovery = 0; //stomp recovery cancel
        }
        dodgeFrames = 0; // dodge cancel
    }
}

void run(s8 direction)
{
    player_direction = direction;
    player_vel_x = player_direction * WALKING_SPEED; // pixel/frame
    SPR_setAnim(player, WALKING_ANIM);
    player_direction < 0 ? SPR_setHFlip(player, TRUE) : SPR_setHFlip(player, FALSE);
    stompRecovery = 0; //stomp recovery cancel
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

void stomp()
{
    player_vel_y = STOMP_VELOCITY;
    stompRecovery = STOMP_RECOVERY_FRAMES;
    player_vel_x = 0;
}

void fixinToHeal()
{
    if (healCounter > 0)
    {
        SPR_setAutoAnimation(player, FALSE);
        SPR_setAnimAndFrame(player, HEALING_ANIM, 0);
        player_vel_x = 0;
        healCounter--;
    }

}

bool airborne()
{
    return F16_toRoundedInt(player_pos_y) < 100;
}

bool stomping()
{
    return player_vel_y >= STOMP_VELOCITY;
}
