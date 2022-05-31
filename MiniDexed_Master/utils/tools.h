
extern wchar_t wmessage[];
#ifdef __cplusplus
extern "C"
{
#endif
	void charToWChar(const char* text);
	void hagl_print(const char* charstring, uint16_t x, uint16_t y, uint16_t color, uint8_t fontsize);
	long map(long x, long in_min, long in_max, long out_min, long out_max);
	int min(int a, int b);
	int max(int a, int b);
#ifdef __cplusplus
}
#endif