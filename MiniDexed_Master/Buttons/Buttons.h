#ifndef _BUTTONS_H
#define _BUTTONS_H
#ifdef __cplusplus
extern "C"
{
#endif
class cButtons {
public:
	cButtons();
	uint8_t getState();
	void getButtons();
private:
	uint8_t state;
	uint64_t last_seen;
	uint8_t cs;
};
#ifdef __cplusplus
}
#endif
#endif