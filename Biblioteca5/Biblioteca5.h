#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	//__declspec(dllexport) void Teste(char msg[100], char title[50]);
	__declspec(dllexport) void selectSource();
	__declspec(dllexport) int aquire(char *);	

#ifdef __cplusplus
}
#endif