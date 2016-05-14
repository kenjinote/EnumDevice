#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "setupapi")

#include <windows.h>
#include <setupapi.h>
#define INITGUID
#include <devpkey.h>
#undef INITGUID

TCHAR szClassName[] = TEXT("Window");

LPTSTR HeapAllocDevicePropertyString(
	HANDLE              HeapHandle,
	HDEVINFO            DeviceInfoSet,
	PSP_DEVINFO_DATA    DeviceInfoData,
	const DEVPROPKEY*   PropertyKey,
	DEVPROPTYPE*        PropertyType,
	PDWORD              CopiedSize,
	DWORD               Flags)
{
	DEVPROPTYPE PropType;
	DWORD dwSize;
	if (!SetupDiGetDeviceProperty(
		DeviceInfoSet,
		DeviceInfoData,
		PropertyKey,
		&PropType,
		0, 0,
		&dwSize,
		0) && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		if (PropertyType != 0)
		{
			*PropertyType = DEVPROP_TYPE_NULL;
		}
		if (CopiedSize != 0)
		{
			*CopiedSize = 0;
		}
		return 0;
	}
	if (PropertyType != 0)
	{
		*PropertyType = PropType;
	}
	if (CopiedSize != 0)
	{
		*CopiedSize = dwSize;
	}
	if (PropType != DEVPROP_TYPE_STRING)
	{
		return 0;
	}
	LPTSTR buffer = (LPTSTR)HeapAlloc(HeapHandle, 0, dwSize);
	if (!SetupDiGetDeviceProperty(
		DeviceInfoSet,
		DeviceInfoData,
		PropertyKey,
		&PropType,
		(PBYTE)buffer, dwSize,
		&dwSize,
		0))
	{
		HeapFree(HeapHandle, 0, buffer);
		return 0;
	}
	return buffer;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton;
	static HWND hList;
	switch (msg)
	{
	case WM_CREATE:
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hList = CreateWindow(TEXT("LISTBOX"), 0, WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hButton, 10, 10, 256, 32, TRUE);
		MoveWindow(hList, 0, 50, LOWORD(lParam), HIWORD(lParam) - 50, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SendMessage(hList, LB_RESETCONTENT, 0, 0);
			const HDEVINFO DevInfoHandle = SetupDiGetClassDevs(0, 0, 0, DIGCF_ALLCLASSES | DIGCF_PRESENT);
			SP_DEVINFO_DATA DevInfoData = { sizeof SP_DEVINFO_DATA };
			TCHAR szText[1024];
			for (DWORD i = 0; SetupDiEnumDeviceInfo(DevInfoHandle, i, &DevInfoData); ++i)
			{
				LPTSTR ClassName = HeapAllocDevicePropertyString(GetProcessHeap(), DevInfoHandle, &DevInfoData, &DEVPKEY_Device_Class, 0, 0, 0);
				LPTSTR DeviceDesc = HeapAllocDevicePropertyString(GetProcessHeap(), DevInfoHandle, &DevInfoData, &DEVPKEY_Device_DeviceDesc, 0, 0, 0);
				wsprintf(szText, TEXT("%s - %s"), ClassName ? ClassName : TEXT(""), DeviceDesc ? DeviceDesc : TEXT(""));
				SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szText);
				HeapFree(GetProcessHeap(), 0, DeviceDesc);
				HeapFree(GetProcessHeap(), 0, ClassName);
			}
			SetupDiDestroyDeviceInfoList(DevInfoHandle);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("現在システムに存在する全てのクラス・インターフェイスを列挙"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
