#ifndef _POTS_H
#define _POTS_H
#include <array>
#ifdef __cplusplus
extern "C"
{
#endif
#define CAPTURE_DEPTH 256
#define MIN_DEVIATION	3
#define POT_MIN 15
#define POT_MAX 4090
	class cPots {
	public:
		void init();
		bool isUpdated(uint8_t potid) { return updated[potid]; }
		uint16_t getPot(uint8_t id);
		void readAll();
		void setPotCallback(uint8_t pot, void(*callback)());
	private:
//		uint dma_chan;
		bool updated[3];
		uint16_t pot[3];
		uint16_t capture_buf[CAPTURE_DEPTH];
		void capture(uint8_t channel);
		static std::array<void(*)(), 3>potCallback;

	};
#ifdef __cplusplus
}
#endif
#endif