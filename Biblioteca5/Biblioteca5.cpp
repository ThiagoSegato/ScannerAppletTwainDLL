#include <Windows.h>
#include "twain.h"
#include "Biblioteca5.h"
#include "gdiplus.h"
#include <iostream>


#define BLOCK_BEGIN(x)      do {
#define BLOCK_END(x)        } while (0);
#define EXIT_CURRENT_BLOCK  break;

static HINSTANCE g_hinstDLL;
static HMODULE g_hLib;
static DSMENTRYPROC g_pDSM_Entry;
static TW_IDENTITY g_AppID;
char *pathImg;

/*void Teste(char msg[100], char title[50]){
	MessageBox (0, msg, title, MB_OK);
	return;
}*/

//#pragma comment(lib, "gdiplus.lib")

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	
	
	UINT num = 0;
	UINT size = 0;

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	//if(size == 0)
	//	return -1;

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return 0;
}

/*bool dibToJpeg(UINT *pDIB, int iWidth, int iHeight, int iBits){
	
	//create our DIB bitmap info header
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = iWidth;
	bi.bmiHeader.biHeight = iHeight;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = ((iBits / 8) * iWidth * iHeight);

	//create the dest image
	Gdiplus::Bitmap DestBmp(iWidth, iHeight, PixelFormat32bppARGB);
	Gdiplus::Rect rect1(0, 0, iWidth, iHeight);

	Gdiplus::BitmapData bitmapData;
	memset(&bitmapData, 0, sizeof(bitmapData));
	DestBmp.LockBits(
		&rect1, 
		Gdiplus::ImageLockModeRead,
		PixelFormat32bppARGB,
		&bitmapData);

	int nStride1 = bitmapData.Stride;
	if(nStride1 < 0)
		nStride1 = -nStride1;

	UINT *DestPixels = (UINT*)bitmapData.Scan0;

	for(UINT row = 0; row < bitmapData.Height; ++row)
	{
		for(UINT col = 0; col < bitmapData.Width; ++col)
		{
			DestPixels[row * nStride1 / 4 + col] = pDIB[row * iWidth + col];
		}
	}

	DestBmp.UnlockBits(&bitmapData);

	//set our output type
	CLSID Clsid;
	
	//const WCHAR* _type;
	//char* type = "image/jpeg";
	//MultiByteToWideChar(CP_ACP, 0, type, (int) strlen(type) + 1, (LPWSTR)_type, sizeof(_type)/sizeof(WCHAR));

	GetEncoderClsid(L"image/jpeg", &Clsid);

	DestBmp.Save(L"C:\\carmel.jpg", &Clsid);

	return TRUE;


	/*
	//create stream with 0 size
	IStream *pIStream = NULL;
	if(CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream) != S_OK)
	{
		return FALSE;
	}

	//write the file to the stream object
	DWORD dwSize = (iBits / 8) * iWidth * iHeight;
	pIStream->Write(&bi, sizeof(BITMAPINFO), NULL);
	pIStream->Write(pDIB, dwSize, NULL);

	//now move pointer to the beginning of the file
	LARGE_INTEGER lnOffset;
	lnOffset.QuadPart = 0;
	if(pIStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
	{
		pIStream->Release();
		return FALSE;
	}

	//create image pointer from stream
	Gdiplus::Image *pDIBImage = Gdiplus::Image::FromStream(pIStream);
	
}*/

int aquire(char *imgTitle){
	
	int sucess = 0;
	HWND hwnd = CreateWindow ("STATIC",
							"",
							WS_POPUPWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							HWND_DESKTOP,
							0,
							g_hinstDLL,
							0);

	if (hwnd == 0){
		//MessageBox (0, "Unable to create private window (acquire).", "Erro", MB_OK);
		MessageBox (0, "Não foi possível iniciar a conexão com o windows.", "Erro 01", MB_OK);
		return sucess;
	}

	SetWindowPos (hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

	TW_UINT16 rc;
	
	BLOCK_BEGIN(1)

	rc = (*g_pDSM_Entry) (&g_AppID,
                         0,
                         DG_CONTROL,
                         DAT_PARENT,
                         MSG_OPENDSM,
                         (TW_MEMREF) &hwnd);
	
	if (rc != TWRC_SUCCESS){
		//MessageBox (0, "Unable to open data source manager (acquire).", "Erro", MB_OK);
		MessageBox (0, "Não foi possível abrir o gerenciador de dados.", "Erro 02", MB_OK);
		EXIT_CURRENT_BLOCK
	}

	BLOCK_BEGIN(2)

	TW_IDENTITY srcID;

	ZeroMemory (&srcID, sizeof(srcID));
	rc = (*g_pDSM_Entry) (&g_AppID,
							0,
							DG_CONTROL,
							DAT_IDENTITY,
							MSG_GETDEFAULT,
							&srcID);

	if (rc == TWRC_FAILURE){
		//MessageBox (0, "Unable to obtain default data source name (acquire).", "Erro", MB_OK);
		MessageBox (0, "Não foi possível obter o nome do scanner padrão.", "Erro 03", MB_OK);
		EXIT_CURRENT_BLOCK
	}
	
	rc = (*g_pDSM_Entry) (&g_AppID,
                         0,
                         DG_CONTROL,
                         DAT_IDENTITY,
                         MSG_OPENDS,
                         &srcID);

	if (rc != TWRC_SUCCESS){
		//MessageBox (0, "Unable to open default data source (acquire).", "Erro", MB_OK);
		MessageBox (0, "Não foi possível se comunicar com o scanner.", "Erro 04", MB_OK);
		EXIT_CURRENT_BLOCK
	}
	
	BLOCK_BEGIN(3)

	TW_USERINTERFACE ui;
	ui.ShowUI = TRUE;
	ui.ModalUI = FALSE;
	ui.hParent = hwnd;

	rc = (*g_pDSM_Entry) (&g_AppID,
                         &srcID,
                         DG_CONTROL,
                         DAT_USERINTERFACE,
                         MSG_ENABLEDS,
                         &ui);

	if (rc != TWRC_SUCCESS){
		//MessageBox (0, "Unable to enable default data source (acquire).", "Erro", MB_OK);
		MessageBox (0, "Não foi possível habilitar o scanner padrão.", "Erro 05", MB_OK);
		EXIT_CURRENT_BLOCK
	}

	MSG msg;
	TW_EVENT event;
	TW_PENDINGXFERS pxfers;

	while (GetMessage ((LPMSG) &msg, 0, 0, 0)){

		event.pEvent = (TW_MEMREF) &msg;
		event.TWMessage = MSG_NULL;

		rc = (*g_pDSM_Entry) (&g_AppID,
							&srcID,
							DG_CONTROL,
							DAT_EVENT,
							MSG_PROCESSEVENT,
							(TW_MEMREF) &event);

		if (rc == TWRC_NOTDSEVENT){             
			TranslateMessage ((LPMSG) &msg);
			DispatchMessage ((LPMSG) &msg);
			continue;
		}

		if (event.TWMessage == MSG_CLOSEDSREQ)
			break;

		if (event.TWMessage == MSG_XFERREADY){
			// Obtain information about the first image to be transferred.

			TW_IMAGEINFO ii;
			rc = (*g_pDSM_Entry) (&g_AppID,
								&srcID,
								DG_IMAGE,
								DAT_IMAGEINFO,
								MSG_GET,
								(TW_MEMREF) &ii);

			if (rc == TWRC_FAILURE){
				// Cancel all transfers.

				(*g_pDSM_Entry) (&g_AppID,
								&srcID,
								DG_CONTROL,
								DAT_PENDINGXFERS,
								MSG_RESET,
								(TW_MEMREF) &pxfers);

				//MessageBox (0, "Unable to obtain image information (acquire).", "Erro", MB_OK);		
				MessageBox (0, "Não foi possível obter as informações da imagem.", "Erro 06", MB_OK);
				break;
			}

			if (ii.Compression != TWCP_NONE ||
              ii.BitsPerPixel != 8 &&
              ii.BitsPerPixel != 24){
				// Cancel all transfers.

				(*g_pDSM_Entry) (&g_AppID,
								&srcID,
								DG_CONTROL,
								DAT_PENDINGXFERS,
								MSG_RESET,
								(TW_MEMREF) &pxfers);

				//MessageBox (0, "Image compressed or not 8-bit/24-bit.", "Erro", MB_OK);
				MessageBox (0, "Cabeçalho da imagem inconsistentes.", "Erro 07", MB_OK);
				break;
			}

			TW_UINT32 handle;

			
			rc = (*g_pDSM_Entry) (&g_AppID,
								&srcID,
								DG_IMAGE,
								DAT_IMAGENATIVEXFER,
								MSG_GET,
								(TW_MEMREF) &handle);

			if (rc != TWRC_XFERDONE){
				// Cancel all remaining transfers.

				(*g_pDSM_Entry) (&g_AppID,
								&srcID,
								DG_CONTROL,
								DAT_PENDINGXFERS,
								MSG_RESET,
								(TW_MEMREF) &pxfers);

				//MessageBox (0, "User aborted transfer or failure (acquire).", "Erro", MB_OK);		
				MessageBox (0, "Transferência abortada pelo usuário.", "Erro 08", MB_OK);
				break;
			}

			//LPBITMAPINFOHEADER lpbmih;
			//lpbmih = (LPBITMAPINFOHEADER) GlobalLock ((HANDLE) handle);
			
			//if (ii.BitsPerPixel == 8){
				//MessageBox (0, "xfer8.", "Informação", MB_OK);	//image = xferDIB8toImage (lpbmih, env);
			//}
			//else{
				//MessageBox (0, "xfer24.", "Informação", MB_OK); //image = xferDIB24toImage (lpbmih, env);
			//}

			//DIB - Device Idependent Bitmap

			////////////////////////////////////
			/*
			BITMAPFILEHEADER bfh;
			BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*) GlobalLock((HANDLE) handle);
			
			HANDLE hf = NULL;
			char path[300] = "";	
			sprintf(path, "%s%s%s%s", pathImg, "\\", imgTitle, ".jpg");
			MessageBox (0, path, "Deposit IMGs", MB_OK);
			hf = CreateFile(path, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			if(hf){
								
					DWORD dwBytesWritten = 0l;					
					bfh.bfType = ( (WORD) ('M' << 8) | 'B');
					bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
						((((pBIH->biWidth * pBIH->biBitCount + 31)/32) * 4) * pBIH->biHeight)
						+ pBIH->biClrUsed * sizeof(RGBQUAD);
					bfh.bfReserved1 = 0;
					bfh.bfReserved2 = 0;
					bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
						+ pBIH->biClrUsed * sizeof(RGBQUAD);

					WriteFile(hf, &bfh, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
					WriteFile(hf, pBIH, bfh.bfSize - sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
				
				CloseHandle(hf);
				sucess = 1;
			}
			*/
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			BITMAPFILEHEADER bfh;
			BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*) GlobalLock((HANDLE) handle);
			bfh.bfType = ( (WORD) ('M' << 8) | 'B');
			bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
				((((pBIH->biWidth * pBIH->biBitCount + 31)/32) * 4) * pBIH->biHeight)
				+ pBIH->biClrUsed * sizeof(RGBQUAD);
			bfh.bfReserved1 = 0;
			bfh.bfReserved2 = 0;
			bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
				+ pBIH->biClrUsed * sizeof(RGBQUAD);

			IStream* pIStream = NULL;
			if(CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*) &pIStream) != S_OK)
			{
				sucess = -1;
				MessageBox (0, "Erro na criação do ponteiro da memória", "Erro 09", MB_OK);
			}
			
			pIStream->Write(&bfh, sizeof(BITMAPFILEHEADER), NULL);
			pIStream->Write(pBIH, bfh.bfSize - sizeof(BITMAPFILEHEADER), NULL);

			//now move the pointer to the beginning of the file
			LARGE_INTEGER lnOffset;
			lnOffset.QuadPart = 0;
			if(pIStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
			{
				sucess = -1;
				pIStream->Release();
				MessageBox (0, "Erro na criação do ponteiro da memória", "Erro 10", MB_OK); 
			}

			//create image pointer from stream
			Gdiplus::Image* pDIBImage = Gdiplus::Image::FromStream(pIStream);
			
			//create output stream with 0 size
			IStream* pOutIStream = NULL;
			if(CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*) &pOutIStream) != S_OK)
			{
				sucess = -1;
				delete pDIBImage;
				pIStream->Release();
				MessageBox(0, "Erro na criação do ponteiro da memória", "Erro 11", MB_OK);
			}
			
			//set our output type
			CLSID pngClsid;
			GetEncoderClsid(L"image/jpeg", &pngClsid);

			//setup encoder parameters
			Gdiplus::EncoderParameters encParam;
			encParam.Count = 1;
			encParam.Parameter[0].Guid = Gdiplus::EncoderQuality;
			encParam.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
			encParam.Parameter[0].NumberOfValues = 1;

			//setup compression level
			ULONG quality = 75;
			encParam.Parameter[0].Value = &quality;

			//now save the image to the stream
			Gdiplus::Status SaveStatus = pDIBImage->Save(pOutIStream, &pngClsid, &encParam);
			if(SaveStatus != S_OK)
			{
				sucess = -1;
				delete pDIBImage;
				pIStream->Release();
				pOutIStream->Release();
				MessageBox(0, "Erro converter a digitalização", "Erro 12", MB_OK);
			}
			
			//get the size of the output stream
			ULARGE_INTEGER ulnSize;
			lnOffset.QuadPart = 0;
			if(pOutIStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
			{
				sucess = -1;
				delete pDIBImage;
				pIStream->Release();
				pOutIStream->Release();
				MessageBox(0, "Erro converter a digitalização", "Erro 13", MB_OK);
			}

			//now move the pointer to the beginning of the stream
			lnOffset.QuadPart = 0;
			if(pOutIStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
			{
				sucess = -1;
				delete pDIBImage;
				pIStream->Release();
				pOutIStream->Release();
				MessageBox(0, "Erro converter a digitalização", "Erro 13", MB_OK);
			}

			//copy the stream jpg to memory
			DWORD dwJpgSize = (DWORD) ulnSize.QuadPart;
			BYTE* pJPG = new BYTE[dwJpgSize];
			if(pOutIStream->Read(pJPG, dwJpgSize, NULL) != S_OK)
			{
				sucess = -1;
				delete pDIBImage;
				delete pJPG;
				pIStream->Release();
				pOutIStream->Release();
			}

			//cleanup memory
			delete pDIBImage;
			pIStream->Release();
			pOutIStream->Release();

			if(sucess == 0){
				char path[300] = "";	
				sprintf(path, "%s%s%s%s", pathImg, "\\", imgTitle, ".jpg");
				//MessageBox (0, path, "Deposit IMGs", MB_OK);
				HANDLE hf = CreateFile(path, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
				if(hf){								
					DWORD dwBytesWritten = NULL;					
					WriteFile(hf, pJPG, dwJpgSize, &dwBytesWritten, NULL);								
					CloseHandle(hf);
					sucess = 1;
				}
			}

			GlobalUnlock ((HANDLE) handle);
			GlobalFree ((HANDLE) handle);
			
			/*if (image == 0)
				MessageBox (0, "Could not transfer DIB to Image (acquire).", "Erro", MB_OK);*/
				
			//GlobalUnlock ((HANDLE) handle);
			//GlobalFree ((HANDLE) handle);

			// Cancel all remaining transfers.

			(*g_pDSM_Entry) (&g_AppID,
							&srcID,
							DG_CONTROL,
							DAT_PENDINGXFERS,
							MSG_RESET,
							(TW_MEMREF) &pxfers);

			rc = TWRC_SUCCESS;
			break;
		}
	}

	// Disable the data source.
	(*g_pDSM_Entry) (&g_AppID,
					&srcID,
					DG_CONTROL,
					DAT_USERINTERFACE,
					MSG_DISABLEDS,
					&ui);

	BLOCK_END(3)

	// Close the data source.
	(*g_pDSM_Entry) (&g_AppID,
					0,
					DG_CONTROL,
					DAT_IDENTITY,
					MSG_CLOSEDS,
					&srcID);

	BLOCK_END(2)

	// Close the data source manager.
	(*g_pDSM_Entry) (&g_AppID,
					0,
					DG_CONTROL,
					DAT_PARENT,
					MSG_CLOSEDSM,
					(TW_MEMREF) &hwnd);

	BLOCK_END(1)

	// Destroy window.
	DestroyWindow (hwnd);

	return sucess;
}


void selectSource(){

	HWND hwnd = CreateWindow ("STATIC",
                             "",
                             WS_POPUPWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             HWND_DESKTOP,
                             0,
                             g_hinstDLL,
                             0);

	if (hwnd == 0){
		//MessageBox (0, "Unable to create private window.", "Erro", MB_OK);
		MessageBox (0, "Não foi possível iniciar a conexão com o windows.", "Erro 11", MB_OK);
		return;
	}

	SetWindowPos (hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);

	TW_UINT16 rc;
	TW_IDENTITY srcID;

	BLOCK_BEGIN(1)

	rc = (*g_pDSM_Entry) (&g_AppID,
                        0,
                        DG_CONTROL,
                        DAT_PARENT,
                        MSG_OPENDSM,
                        (TW_MEMREF) &hwnd);
	
	if (rc != TWRC_SUCCESS){
		//MessageBox (0, "Unable to open data source manager.", "Erro", MB_OK);
		MessageBox (0, "Não foi possível abrir o gerenciador de dados.", "Erro 12", MB_OK);
		EXIT_CURRENT_BLOCK
	}

	ZeroMemory (&srcID, sizeof(srcID));
	rc = (*g_pDSM_Entry) (&g_AppID,
						0,
						DG_CONTROL,
						DAT_IDENTITY,
						MSG_USERSELECT,
						(TW_MEMREF) &srcID);

	if (rc == TWRC_FAILURE)
		//MessageBox (0, "Unable to display user interface.", "Erro", MB_OK);
		MessageBox (0, "Não foi possível visualizar opções do usuário.", "Erro 13", MB_OK);

	(*g_pDSM_Entry) (&g_AppID,
                    0,
                    DG_CONTROL,
                    DAT_PARENT,
                    MSG_CLOSEDSM,
                    (TW_MEMREF) &hwnd);

	BLOCK_END(1)

	DestroyWindow (hwnd);

}


BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
	if (fdwReason == DLL_PROCESS_ATTACH){

		pathImg = std::getenv("TEMP");		
		//strcat(pathImg, "\\flexscan_img.bmp");

		g_hinstDLL = hinstDLL;

		char szPath [150]; // Probably only 140 is needed, but why not be safe?

		GetWindowsDirectory(szPath, 128);

		int iLen = lstrlen (szPath);

		if (iLen != 0 && szPath [iLen-1] != '\\')
		   lstrcat (szPath, "\\");

		lstrcat (szPath, "TWAIN_32.DLL");

		//MessageBox (0, szPath, "Tentando Abrir...", MB_OK);
		//MessageBox (0, pathImg, "TEMP", MB_OK);

		OFSTRUCT ofs;
		if (OpenFile (szPath, &ofs, OF_EXIST) != -1)
		   g_hLib = LoadLibrary (szPath);

		if (g_hLib == 0)
		{
		   MessageBox (0, "Unable to open TWAIN_32.DLL", "JTWAIN", MB_OK);
		   return FALSE;
		}

		g_pDSM_Entry = (DSMENTRYPROC) GetProcAddress (g_hLib, "DSM_Entry");

		if (g_pDSM_Entry == 0)
		{
		   MessageBox (0, "Unable to fetch DSM_Entry address", "JTWAIN", MB_OK);
		   return FALSE;
		}

		g_AppID.Id = 0;
		g_AppID.Version.MajorNum = 1;
		g_AppID.Version.MinorNum = 0;
		g_AppID.Version.Language = TWLG_ENGLISH_USA;
		g_AppID.Version.Country = TWCY_USA;

		lstrcpy (g_AppID.Version.Info, "FlexScanLibrary 1.0");

		g_AppID.ProtocolMajor = TWON_PROTOCOLMAJOR;
		g_AppID.ProtocolMinor = TWON_PROTOCOLMINOR;
		g_AppID.SupportedGroups = DG_CONTROL | DG_IMAGE;

		lstrcpy (g_AppID.Manufacturer, "FlexScan");
		lstrcpy (g_AppID.ProductFamily, "FlexScan Image Capture");
		lstrcpy (g_AppID.ProductName, "FlexScan");
	}
	else
	if (fdwReason == DLL_PROCESS_DETACH){
	   if (g_hLib != 0)
		   FreeLibrary (g_hLib);
	}
	return TRUE;
}