// dllmain.cpp : Defines the entry point for the DLL application.
#include "E:\Program Files (x86)\Microsoft Visual Studio\Projects\DarkLyricsAPI\DarkLyricsAPI\lyrichandler.h"
#include <Windows.h>
#include <strsafe.h>
#include <string>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <exception>
#include "api.h"
#include <api\service\waservicefactory.h>
#include <api\application\api_application.h>
#include <bfc\platform\types.h>

#include "embedwnd.h"
#include "resource.h"

#include <Agave\Language\api_language.h>
#include <Agave\Language\lang.h>
#include <Winamp\wa_ipc.h>
#include <Winamp\gen.h>
#include <Winamp\ipc_pe.h>
#define WA_DLG_IMPLEMENT
#include <Winamp\wa_dlg.h>


#define PLUGIN_NAME "Lyrics Finder"
#define PLUGIN_VERSION "v1.0"
#define FILE_INFO_BUFFER_SIZE 128
#define MAX_THREAD_COUNT 1

static const GUID wndStateGUID = { 0x3fcd6a40, 0x95d2, 0x4b0a, { 0x8a, 0x96, 0x24, 0x7e, 0xc5, 0xc3, 0x32, 0x9b } };
static const GUID GenEmbedWndExampleLangGUID =
{ 0x486676e6, 0x9306, 0x4fcf, { 0x9d, 0x9b, 0x76, 0x5a, 0x65, 0xf9, 0xfe, 0xb8 } };

api_service     *WASABI_API_SVC = 0;
api_language    *WASABI_API_LNG = 0;
api_application *WASABI_API_APP = 0;

INT_PTR CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WaWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void GetAlbumLyrics(HWND hwnd);

HINSTANCE         WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
embedWindowState  myWndState = { 0 };
WNDPROC           lpWndProcOld = 0, lpWndProc = 0;
HWND              embedWnd = NULL, childWnd = NULL, hwndInner = NULL;
HMENU             menu, context_menu;
WCHAR*            ini_file;
WCHAR             wa_path[MAX_PATH] = { 0 };
UINT              LYRICS_MENUID, EMBEDWND_ID;
std::atomic<int>  active_threads{};
std::wstring      active_song;
std::mutex        album_mutex;
LyricHandler      handler;
bool              isActivated = true;
COLORREF clrBackground     = RGB(0, 0, 0),
		 clrCuePoint       = RGB(117, 116, 139),
		 clrGeneratingText = RGB(0, 128, 0);
		 

// Winamp PLUGIN specific funcs
void config();
void quit();
int  init();
// -------

winampGeneralPurposePlugin plugin =
{
    GPPHDR_VER,
    0,
    init,
    config,
    quit,
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	return &plugin;
}

winampGeneralPurposePlugin* getModule(int which)
{
	switch (which)
	{
	case 0: return &plugin;

	default:return NULL;
	}
}

template <class api_T>
void ServiceBuild(api_T*& api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory* factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T*)factory->getInterface();
	}
}

LRESULT CALLBACK WaWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	HandleEmbeddedWindowWinampWindowMessages(embedWnd, EMBEDWND_ID, &myWndState, TRUE, hwnd, message, wParam, lParam);

	if (message == WM_WA_IPC)
	{
		switch (message)
		{
		case WM_USER:
		{
			if (lParam == IPC_PLAYING_FILE)
			{
				if(active_threads < MAX_THREAD_COUNT)
					std::thread(GetAlbumLyrics, hwnd).detach();
			}
			break;
		}
		}
	}
	LRESULT res = CallWindowProc(lpWndProcOld, hwnd, message, wParam, lParam);
	HandleEmbeddedWindowWinampWindowMessages(embedWnd, EMBEDWND_ID, &myWndState, FALSE, hwnd, message, wParam, lParam);
	return res;
}

void config()
{
	
}

void quit()
{
	//if (no_uninstall)
	{
		// this will attempt to save any required settings here if not doing an uninstall
		DestroyEmbeddedWindow(&myWndState);
	}
}

int init()
{
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION) < 0x5053)
	{
		MessageBoxA(plugin.hwndParent, "This plug-in requires Winamp v5.53 and up for it to work.\t\n"
			"Please upgrade your Winamp client to be able to use this.",
			plugin.description, MB_OK | MB_ICONINFORMATION);
		return 1;
	}
	else
	{
		WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
		if (WASABI_API_SVC == (api_service*)1)
			WASABI_API_SVC = NULL;

		if (WASABI_API_SVC)
		{
			ServiceBuild(WASABI_API_LNG, languageApiGUID);
			WASABI_API_START_LANG(plugin.hDllInstance, GenEmbedWndExampleLangGUID);

			static char szDescription[256]; 
			StringCchPrintfA(szDescription, 256, WASABI_API_LNGSTRING(IDS_LANGUAGE_EXAMPLE), PLUGIN_VERSION);
			plugin.description = szDescription;		

			ini_file = (LPWSTR)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILE);

			if (IsWindowUnicode(plugin.hwndParent))
			{
				lpWndProcOld = (WNDPROC)SetWindowLongPtrW(plugin.hwndParent, GWLP_WNDPROC, (LONG)WaWndProc);
			}
			else
			{
				lpWndProcOld = (WNDPROC)SetWindowLongPtrA(plugin.hwndParent, GWLP_WNDPROC, (LONG)WaWndProc);
			}

			ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);

			embedWnd = CreateEmbeddedWindow(&myWndState, wndStateGUID);
	
			// once the window is created we can then specify the window title and menu integration
			SetWindowText(embedWnd, WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME));

			childWnd = WASABI_API_CREATEDIALOG(IDD_DIALOGVIEW, embedWnd, ChildWndProc);
			if (childWnd && WASABI_API_APP)
				WASABI_API_APP->app_addModelessDialog(childWnd);


			EMBEDWND_ID = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_REGISTER_LOWORD_COMMAND);
			GetPrivateProfileInt(L"wnd_open", L"OpenWND", TRUE, NULL);
			
			ACCEL accel = { FVIRTKEY | FALT,'1', EMBEDWND_ID };
			HACCEL hAccel = CreateAcceleratorTable(&accel, 1);
			if (hAccel)
			{
				WASABI_API_APP->app_addAccelerators(childWnd, &hAccel, 1, TRANSLATE_MODE_NORMAL);
			}			

			AddEmbeddedWindowToMenus(TRUE, EMBEDWND_ID, WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME), -1); // Change background colour based on skin.

			return GEN_INIT_SUCCESS;
		}
	}
	return 1;
}

INT_PTR CALLBACK ChildWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case IDC_REFRESH_BUTTON:
	{
		const wchar_t* filename = (const wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
		wchar_t title[FILE_INFO_BUFFER_SIZE]{ 0 };
		extendedFileInfoStructW FileInfo_s{
			filename,
			L"TITLE",
			title,
			FILE_INFO_BUFFER_SIZE,
		};
		SendMessage(hwnd, WM_WA_IPC, (WPARAM)&FileInfo_s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);
		if (active_song != title)
		{
			GetAlbumLyrics(hwnd);
		}
	}
	case WM_VSCROLL:
	{
		SCROLLINFO scrollInfo;
		scrollInfo.cbSize = sizeof(scrollInfo);
		scrollInfo.fMask = SIF_ALL;

		GetScrollInfo(hwnd, SB_VERT, &scrollInfo);
		int nPos = scrollInfo.nPos;
		switch (LOWORD(wParam))
		{
		case SB_LINEDOWN:
			nPos -= 1;
			break;
		case SB_LINEUP:
			nPos += 1;
			break;
		case SB_PAGELEFT:
			nPos -= scrollInfo.nPage;
			break;
		case SB_PAGERIGHT:
			nPos += scrollInfo.nPage;
			break;
		case SB_THUMBTRACK:
			nPos = scrollInfo.nTrackPos;
			break;
		default:
			break;
		}
		
		break;
	}
	default: break;
	}
	return HandleEmbeddedWindowChildMessages(embedWnd, EMBEDWND_ID, hwnd, message, wParam, lParam);
}

void GetAlbumLyrics(HWND hwnd)
{
	while (!album_mutex.try_lock()) { Sleep(10); } // Could be circumvented with a global thread.
	const wchar_t* filename = (const wchar_t*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME);
	wchar_t active_song_artist[FILE_INFO_BUFFER_SIZE]{ 0 }, active_song_album[FILE_INFO_BUFFER_SIZE]{ 0 }, title[FILE_INFO_BUFFER_SIZE]{ 0 };

	extendedFileInfoStructW FileInfo_s{
		filename,
		L"ALBUM", //L"%artist% - %title% - %playcount% - %test%",
		active_song_album,
		FILE_INFO_BUFFER_SIZE,
	};
	SendMessage(hwnd, WM_WA_IPC, (WPARAM)&FileInfo_s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);

	FileInfo_s.metadata = L"ARTIST";
	FileInfo_s.ret = active_song_artist;
	SendMessage(hwnd, WM_WA_IPC, (WPARAM)&FileInfo_s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);

	FileInfo_s.metadata = L"TITLE";
	FileInfo_s.ret = title;
	SendMessage(hwnd, WM_WA_IPC, (WPARAM)&FileInfo_s, IPC_GET_EXTENDED_FILE_INFOW_HOOKABLE);

	if (active_song != title)
	{
		if (handler.GetAlbum().name == active_song_album)
		{
			const wchar_t* current{ handler[active_song].c_str() };
			SetDlgItemText(childWnd, IDC_LYRIC_STRING, current);
		}

		try
		{
			handler.GetLyrics(LyricsUtil::WstringToUTF8(active_song_artist), LyricsUtil::WstringToUTF8(active_song_album), LyricsUtil::DarkLyricsDecoder);
			//LyricsUtil::DarkLyricsDecoder(LyricsUtil::WstringToUTF8(active_song_artist), LyricsUtil::WstringToUTF8(active_song_album), album);
			active_song = std::wstring(title);

			int success = lstrcmpW(handler.GetAlbum().name.c_str(), L"failed");
			if (handler.GetSize() > 0)
			{				
				const wchar_t* current{ handler[active_song].c_str() };
				SetDlgItemText(childWnd, IDC_LYRIC_STRING, current); 
			}
			else
			{			
				SetDlgItemText(childWnd, IDC_LYRIC_STRING, L"Lyrics not found.");
			}
		}
		catch (std::exception& e)
		{
			MessageBoxA(hwnd, e.what(), "EXCEPTION THROWN", MB_OK);
		}
	}
	album_mutex.unlock(); // Unlock.
	--active_threads;
}