#define DEXED       uart1
#define MIDIPORT    uart0

extern bool led_usb_state;
extern bool led_uart_state;

void midi_task(void); 
void led_task(void);
void midicore();