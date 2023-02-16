
#define SOF 0xAA
#define EOF 0xBB

void usb_init(void);
const char* slcan_format(twai_message_t message);