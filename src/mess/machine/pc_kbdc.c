/***************************************************************************

  PC Keyboard connector interface

The following basic program can be useful for identifying scancodes:
10 sc%=0:sp%=0
20 sc%=inp(96)
30 if sc%<>sp% then print hex$(sc%):sp%=sc%
40 goto 20

***************************************************************************/


#include "emu.h"
#include "emuopts.h"
#include "machine/pc_kbdc.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PC_KBDC_SLOT = &device_creator<pc_kbdc_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc_kbdc_slot_device - constructor
//-------------------------------------------------
pc_kbdc_slot_device::pc_kbdc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PC_KBDC_SLOT, "PC_KBDC_SLOT", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


void pc_kbdc_slot_device::static_set_pc_kbdc_slot(device_t &device, device_t *kbdc_device)
{
	pc_kbdc_slot_device &pc_kbdc = dynamic_cast<pc_kbdc_slot_device &>(device);
	pc_kbdc.m_kbdc_device = kbdc_device;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc_kbdc_slot_device::device_start()
{
	device_pc_kbd_interface *pc_kbd = dynamic_cast<device_pc_kbd_interface *>(get_card_device());

	if (pc_kbd)
	{
		device_pc_kbd_interface::static_set_pc_kbdc( *pc_kbd, m_kbdc_device );
	}
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PC_KBDC = &device_creator<pc_kbdc_device>;


//-------------------------------------------------
//  pc_kbdc_device - constructor
//-------------------------------------------------
pc_kbdc_device::pc_kbdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, PC_KBDC, "PC_KBDC", tag, owner, clock),
		m_keyboard( NULL )
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------
void pc_kbdc_device::device_config_complete()
{
	// inherit a copy of the static data
	const pc_kbdc_interface *intf = reinterpret_cast<const pc_kbdc_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<pc_kbdc_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_clock_cb, 0, sizeof(m_out_clock_cb));
		memset(&m_out_data_cb, 0, sizeof(m_out_data_cb));
	}
}


void pc_kbdc_device::set_keyboard( device_pc_kbd_interface *keyboard )
{
	m_keyboard = keyboard;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void pc_kbdc_device::device_start()
{
	// resolve callbacks
	m_out_clock_func.resolve(m_out_clock_cb, *this);
	m_out_data_func.resolve(m_out_data_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void pc_kbdc_device::device_reset()
{
	m_clock_state = -1;     /* initial state of calculated clock line */
	m_data_state = -1;      /* initial state of calculated data line */

	// Initially assume both keyboard and mainboard have released their data and clock lines
	m_mb_clock_state = 1;
	m_mb_data_state = 1;
	m_kb_clock_state = 1;
	m_kb_data_state = 1;
}


void pc_kbdc_device::update_clock_state()
{
	int new_clock_state = m_mb_clock_state & m_kb_clock_state;

	if ( new_clock_state != m_clock_state )
	{
		// We first set our state to prevent possible endless loops
		m_clock_state = new_clock_state;

		// Send state to keyboard interface logic on mainboard
		m_out_clock_func( m_clock_state );

		// Send state to keyboard
		if ( m_keyboard )
		{
			m_keyboard->clock_write( m_clock_state );
		}
	}
}


void pc_kbdc_device::update_data_state()
{
	int new_data_state = m_mb_data_state & m_kb_data_state;

	if ( new_data_state != m_data_state )
	{
		// We first set our state to prevent possible endless loops
		m_data_state = new_data_state;

		// Send state to keyboard interface logic on mainboard
		m_out_data_func( m_data_state );

		// Send state to keyboard
		if ( m_keyboard )
		{
			m_keyboard->data_write( m_data_state );
		}
	}
}


WRITE_LINE_MEMBER( pc_kbdc_device::clock_write_from_mb )
{
	m_mb_clock_state = state;
	update_clock_state();
}


WRITE_LINE_MEMBER( pc_kbdc_device::data_write_from_mb )
{
	m_mb_data_state = state;
	update_data_state();
}


WRITE_LINE_MEMBER( pc_kbdc_device::clock_write_from_kb )
{
	m_kb_clock_state = state;
	update_clock_state();
}


WRITE_LINE_MEMBER( pc_kbdc_device::data_write_from_kb )
{
	m_kb_data_state = state;
	update_data_state();
}


//**************************************************************************
//  DEVICE PC KBD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pc_kbd_interface - constructor
//-------------------------------------------------

device_pc_kbd_interface::device_pc_kbd_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_pc_kbdc(NULL),
		m_pc_kbdc_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_pc_kbd_interface - destructor
//-------------------------------------------------

device_pc_kbd_interface::~device_pc_kbd_interface()
{
}


WRITE_LINE_MEMBER( device_pc_kbd_interface::clock_write )
{
}


WRITE_LINE_MEMBER( device_pc_kbd_interface::data_write )
{
}


void device_pc_kbd_interface::static_set_pc_kbdc(device_t &device, device_t *kbdc_device)
{
	device_pc_kbd_interface &pc_kbd = dynamic_cast<device_pc_kbd_interface &>(device);
	pc_kbd.m_pc_kbdc = dynamic_cast<pc_kbdc_device *>(kbdc_device);
}


void device_pc_kbd_interface::set_pc_kbdc_device()
{
	if ( m_pc_kbdc )
	{
		m_pc_kbdc->set_keyboard( this );
	}
}
