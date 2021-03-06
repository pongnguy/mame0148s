/* Big Striker (bootleg) Video Hardware */

#include "emu.h"
#include "includes/bigstrkb.h"


/* Sprites */

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/*- SPR RAM Format -**

	 16 bytes per sprite

	  nnnn nnnn  nnnn nnnn  aaaa aaaa  aaaa aaaa  xxxx xxxx  xxxx xxxx  yyyy yyyy  yyyy yyyy
	    ( rest unused )
	**- End of Comments -*/

	bigstrkb_state *state = machine.driver_data<bigstrkb_state>();
	gfx_element *gfx = machine.gfx[2];
	UINT16 *source = state->m_spriteram;
	UINT16 *finish = source + 0x800/2;

	while( source<finish )
	{
		int xpos, ypos, num, attr;

		int flipx, col;

		xpos = source[2];
		ypos = source[3];
		num = source[0];
		attr = source[1];

		ypos = 0xffff - ypos;


		xpos -= 126;
		ypos -= 16;

		flipx = attr & 0x0100;
		col = attr & 0x000f;

		drawgfx_transpen(bitmap,cliprect,gfx,num,col,flipx,0,xpos,ypos,15);
		source+=8;
	}
}

/* Tilemaps */

TILEMAP_MAPPER_MEMBER(bigstrkb_state::bsb_bg_scan)
{
	int offset;

	offset = ((col&0xf)*16) + (row&0xf);
	offset += (col >> 4) * 0x100;
	offset += (row >> 4) * 0x800;

	return offset;
}

TILE_GET_INFO_MEMBER(bigstrkb_state::get_bsb_tile_info)
{
	int tileno,col;

	tileno = m_videoram[tile_index] & 0x0fff;
	col=    m_videoram[tile_index] & 0xf000;

	SET_TILE_INFO_MEMBER(0,tileno,col>>12,0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(bigstrkb_state::get_bsb_tile2_info)
{
	int tileno,col;

	tileno = m_videoram2[tile_index] & 0x0fff;
	col=    m_videoram2[tile_index] & 0xf000;

	SET_TILE_INFO_MEMBER(1,tileno,col>>12,0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram2_w)
{
	m_videoram2[offset] = data;
	m_tilemap2->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(bigstrkb_state::get_bsb_tile3_info)
{
	int tileno,col;

	tileno = m_videoram3[tile_index] & 0x0fff;
	col=    m_videoram3[tile_index] & 0xf000;

	SET_TILE_INFO_MEMBER(1,tileno+0x2000,(col>>12)+(0x100/16),0);
}

WRITE16_MEMBER(bigstrkb_state::bsb_videoram3_w)
{
	m_videoram3[offset] = data;
	m_tilemap3->mark_tile_dirty(offset);
}

/* Video Start / Update */

void bigstrkb_state::video_start()
{
	m_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(bigstrkb_state::get_bsb_tile_info),this),TILEMAP_SCAN_COLS, 8, 8,64,32);
	m_tilemap2 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(bigstrkb_state::get_bsb_tile2_info),this),tilemap_mapper_delegate(FUNC(bigstrkb_state::bsb_bg_scan),this), 16, 16,128,64);
	m_tilemap3 = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(bigstrkb_state::get_bsb_tile3_info),this),tilemap_mapper_delegate(FUNC(bigstrkb_state::bsb_bg_scan),this), 16, 16,128,64);

	m_tilemap->set_transparent_pen(15);
	//m_tilemap2->set_transparent_pen(15);
	m_tilemap3->set_transparent_pen(15);
}

UINT32 bigstrkb_state::screen_update_bigstrkb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

//  bitmap.fill(get_black_pen(machine()), cliprect);

	m_tilemap2->set_scrollx(0, m_vidreg1[0]+(256-14));
	m_tilemap2->set_scrolly(0, m_vidreg2[0]);

	m_tilemap3->set_scrollx(0, m_vidreg1[1]+(256-14));
	m_tilemap3->set_scrolly(0, m_vidreg2[1]);

	m_tilemap2->draw(bitmap, cliprect, 0,0);
	m_tilemap3->draw(bitmap, cliprect, 0,0);

	draw_sprites(machine(),bitmap,cliprect);
	m_tilemap->draw(bitmap, cliprect, 0,0);

//  popmessage ("Regs %08x %08x %08x %08x",bsb_vidreg2[0],bsb_vidreg2[1],bsb_vidreg2[2],bsb_vidreg2[3]);
	return 0;
}
