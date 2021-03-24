MEMORY
{
  FLASH : ORIGIN = 0x00000000, LENGTH = 1024K
  RAM : ORIGIN = 0x20000000, LENGTH = 256K
  DWT : ORIGIN = 0xE0001000, LENGTH = 4K
  SCB : ORIGIN = 0xE000ED00, LENGTH = 240
  SCB_DEBUG : ORIGIN = 0xE000EDF0, LENGTH = 16
}

SECTIONS
{
	.dwt_regs (NOLOAD) : {
		*(.dwt_regs)
	} > DWT

	.scb_regs (NOLOAD) : {
		*(.scb_regs)
	} > SCB

	.scb_debug_regs (NOLOAD) : {
		*(.scb_debug_regs)
	} > SCB_DEBUG
}

