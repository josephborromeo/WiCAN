#include <stdint.h>

void write_to_usb (uint8_t* tx_buffer, uint8_t buff_size);
void flush_usb(void);
void usb_init(void);