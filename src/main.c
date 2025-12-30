#include <genesis.h>
#include <res_maps.h>
#include <res_sprites.h>
#include <string.h>

// maps (BGA and BGB)
Map *bgb;
Map *bga;

// sprites
Sprite *player;

// BG start tile index
u16 bgBaseTileIndex[2];

int main(bool hardReset)
{

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
    player = SPR_addSprite(&player_sprite, 40, 100, TILE_ATTR(PAL3, 0, FALSE, FALSE));

    while (1)
    {

        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}
