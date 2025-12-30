#include <genesis.h>
#include <res_maps.h>
#include <res_sprites.h>
#include <string.h>

/*The edges of the play field*/
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 224;

// maps (BGA and BGB)
Map *bgb;
Map *bga;

// sprites
Sprite *player;

// BG start tile index
u16 bgBaseTileIndex[2];

// states
int player_vel_x = 0;
int player_pos_x = 40;
const int player_pos_y = 100;
const int player_width = 24;
const int player_height = 32;

/*
The callback function for joypad inputs.
pressed = 1 if pressed and 0 if not
changed = 1 if state is different than last frame's state, 0 if not
*/
void joyHandler(u16 joy, u16 changed, u16 pressed)
{
    if (joy == JOY_1)
    {
        /*Set player velocity if left or right are pressed;
         *set velocity to 0 if no direction is pressed
         NOTE: need to use bitwise operators here, since enums are hex */
        if (pressed & BUTTON_RIGHT)
        {
            player_vel_x = 3; // pixel/frame
        }
        else if (pressed & BUTTON_LEFT)
        {
            player_vel_x = -3;
        }
        else
        {
            if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT))
            {
                player_vel_x = 0;
            }
        }
    }
}

void positionPlayer()
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

int main(bool hardReset)
{

    // Input
    JOY_init();
    JOY_setEventHandler(&joyHandler);

    // Background
    int ind = 0;
    VDP_loadTileSet(&bga_tileset, ind, DMA);
    bgBaseTileIndex[0] = ind;
    ind += bga_tileset.numTile;
    VDP_loadTileSet(&bgb_tileset, ind, DMA);
    bgBaseTileIndex[1] = ind;
    ind += bgb_tileset.numTile;

    PAL_setPalette(PAL0, palette_bga.data, DMA);
    PAL_setPalette(PAL3, player_sprite.palette->data, DMA);
    bga = MAP_create(&bga_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));
    bgb = MAP_create(&bgb_map, BG_B, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bgBaseTileIndex[1]));
    MAP_scrollTo(bga, 0, 0); //first call loads the bga tiles into VRAM
    SYS_doVBlankProcess(); //call to clear the DMA cache
    MAP_scrollTo(bgb, 0, 0); //first call loads the bgb tiles into VRAM
    SYS_doVBlankProcess(); //call to clear the DMA cache again

    SYS_showFrameLoad(TRUE);

    // Sprites
    SPR_init();
    player = SPR_addSprite(&player_sprite, player_pos_x, player_pos_y, TILE_ATTR(PAL3, 0, FALSE, FALSE));

    while (1)
    {
        positionPlayer();

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}
