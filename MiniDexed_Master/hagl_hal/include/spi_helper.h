#ifdef __cplusplus
extern "C"
{
#endif
enum cs { csKEY, csSD, csLCD };
void spi_cs(uint8_t port);
void spiAllHigh();
#ifdef __cplusplus
}
#endif