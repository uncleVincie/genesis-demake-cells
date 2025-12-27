#include <genesis.h>
#include <res_maps.h>
#include <string.h>

// maps (BGA and BGB)
Map *bgb;
Map *bga;

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
    PAL_setPalette(PAL1, palette_bgb.data, DMA);
    bga = MAP_create(&bga_map, BG_A, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));
    bgb = MAP_create(&bgb_map, BG_B, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, bgBaseTileIndex[1]));
    MAP_scrollTo(bga, 0, 0);
    SYS_doVBlankProcess();
    MAP_scrollTo(bgb, 0, 50);
    SYS_doVBlankProcess();

    while (1)
    {

        SYS_doVBlankProcess();
    }
    return (0);
}
