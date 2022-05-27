#ifndef _POTS_H
#define _POTS_H
#ifdef __cplusplus
extern "C"
{
#endif
#define CAPTURE_DEPTH 256

	class cPots {
	public:
		cPots();
		bool isUpdated(uint8_t potid) { return updated[potid]; }
		uint16_t getPot(uint8_t id);
		void readAll();
	private:
//		uint dma_chan;
		bool updated[3];
		uint16_t pot[3];
		uint8_t capture_buf[CAPTURE_DEPTH];
		void capture(uint8_t channel);
	};
#ifdef __cplusplus
}
#endif
#endif