#include "sound/discrete.h"

class skyraid_state : public driver_device
{
public:
	skyraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_pos_ram(*this, "pos_ram"),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_discrete(*this, "discrete"){ }

	int m_analog_range;
	int m_analog_offset;

	int m_scroll;

	required_shared_ptr<UINT8> m_pos_ram;
	required_shared_ptr<UINT8> m_alpha_num_ram;
	required_shared_ptr<UINT8> m_obj_ram;
	required_device<discrete_device> m_discrete;
	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(skyraid_port_0_r);
	DECLARE_WRITE8_MEMBER(skyraid_range_w);
	DECLARE_WRITE8_MEMBER(skyraid_offset_w);
	DECLARE_WRITE8_MEMBER(skyraid_scroll_w);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_skyraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(skyraid_sound_w);
};

/*----------- defined in audio/skyraid.c -----------*/
DISCRETE_SOUND_EXTERN( skyraid );
