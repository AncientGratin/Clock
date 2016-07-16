#include <windows.h>
#include <math.h>
#include "resource.h"

#define PI 3.141593

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 시계바늘 구현을 위한 구조체
typedef struct Needle {
	int edgeLen;
	int rootLen;
	int sideLen;
	POINT edgePos, rootPos, leftPos, rightPos;	// leftPos, rightPos는 edgePos가 12시 방향일 때의 기준
} NEEDLE;

LOGFONT lf;
COLORREF Col;


HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("시계");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
    , LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;

    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_CLOCK);
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    RegisterClass(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 200,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    while(GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_INITDIALOG:
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, IDOK);
			return true;
		}
		break;
	}
	return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
	SYSTEMTIME st;
	RECT crt;
	HPEN hPen, OldPen;
	HBRUSH OldBrush;
	CHOOSEFONT CFT;
	HFONT MyFont, OldFont;
	
	static TCHAR sTime[128];
	static TCHAR sTimeExceptSecond[128];
	static TCHAR sDate[128];
	static bool isAnalog = false;
	static bool showSecond = true;
	static bool showDate = true;
	static bool top = false;
	int radius;
	static NEEDLE ndlHour, ndlMin, ndlSec;
	POINT secRoot, secEdge;
	
    switch(iMessage)
    {
	
    case WM_CREATE:
        hWndMain = hWnd;
		SetTimer(hWnd, 1, 50, NULL);
		SendMessage(hWnd, WM_TIMER, 1, 0);

		lf.lfHeight = 20;
		lf.lfCharSet = HANGEUL_CHARSET;
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
		lstrcpy(lf.lfFaceName, TEXT("바탕"));
        return 0;
	case WM_INITMENU:
		if(top)
			CheckMenuItem((HMENU)wParam, 40017, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem((HMENU)wParam, 40017, MF_BYCOMMAND | MF_UNCHECKED);
		if(isAnalog)
		{
			CheckMenuItem((HMENU)wParam, ID_ANALOG, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem((HMENU)wParam, ID_DIGITAL, MF_BYCOMMAND | MF_UNCHECKED);
			EnableMenuItem((HMENU)wParam, ID_FONT, MF_BYCOMMAND | MF_GRAYED);
		}
		else
		{
			CheckMenuItem((HMENU)wParam, ID_DIGITAL, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem((HMENU)wParam, ID_ANALOG, MF_BYCOMMAND | MF_UNCHECKED);
			EnableMenuItem((HMENU)wParam, ID_FONT, MF_BYCOMMAND | MF_ENABLED);
		}
		if(showSecond)
			CheckMenuItem((HMENU)wParam, ID_SECOND, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem((HMENU)wParam, ID_SECOND, MF_BYCOMMAND | MF_UNCHECKED);
		if(showDate)
			CheckMenuItem((HMENU)wParam, ID_DATE, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem((HMENU)wParam, ID_DATE, MF_BYCOMMAND | MF_UNCHECKED);
	case WM_TIMER:
		GetLocalTime(&st);
		GetClientRect(hWnd, &crt);
		// 시침, 분침, 초침 크기 지정

		// 반지름
		radius = min(crt.right, crt.bottom) / 2;

		// 시침
		ndlHour.edgeLen = radius * 0.55;
		ndlHour.rootLen = radius * 0.12;
		ndlHour.sideLen = radius * 0.06;
		ndlHour.edgePos.x = crt.right / 2 + ndlHour.edgeLen * cos(st.wHour * PI / 6 + st.wMinute * PI / 360 - PI / 2);
		ndlHour.edgePos.y = crt.bottom / 2 + ndlHour.edgeLen * sin(st.wHour * PI / 6 + st.wMinute * PI / 360 - PI / 2);
		ndlHour.rootPos.x = crt.right / 2 + ndlHour.rootLen * cos(st.wHour * PI / 6 + st.wMinute * PI / 360 + PI / 2);
		ndlHour.rootPos.y = crt.bottom / 2 + ndlHour.rootLen * sin(st.wHour * PI / 6 + st.wMinute * PI / 360 + PI / 2);
		ndlHour.leftPos.x = crt.right / 2 + ndlHour.sideLen * cos(st.wHour * PI / 6 + st.wMinute * PI / 360); 
		ndlHour.leftPos.y = crt.bottom / 2 + ndlHour.sideLen * sin(st.wHour * PI / 6 + st.wMinute * PI / 360);
		ndlHour.rightPos.x = crt.right / 2 + ndlHour.sideLen * cos(st.wHour * PI / 6 + st.wMinute * PI / 360 + PI); 
		ndlHour.rightPos.y = crt.bottom / 2 + ndlHour.sideLen * sin(st.wHour * PI / 6 + st.wMinute * PI / 360 + PI);
		// 분침
		ndlMin.edgeLen = radius * 0.65;
		ndlMin.rootLen = radius * 0.16;
		ndlMin.sideLen = radius * 0.04;
		ndlMin.edgePos.x = crt.right / 2 + ndlMin.edgeLen * cos(st.wMinute * PI / 30 + st.wSecond * PI / 1800 - PI / 2);
		ndlMin.edgePos.y = crt.bottom / 2 + ndlMin.edgeLen * sin(st.wMinute * PI / 30 + st.wSecond * PI / 1800 - PI / 2);
		ndlMin.rootPos.x = crt.right / 2 + ndlMin.rootLen * cos(st.wMinute * PI / 30 + st.wSecond * PI / 1800 + PI / 2);
		ndlMin.rootPos.y = crt.bottom / 2 + ndlMin.rootLen * sin(st.wMinute * PI / 30 + st.wSecond * PI / 1800 + PI / 2);
		ndlMin.leftPos.x = crt.right / 2 + ndlMin.sideLen * cos(st.wMinute * PI / 30 + st.wSecond * PI / 1800); 
		ndlMin.leftPos.y = crt.bottom / 2 + ndlMin.sideLen * sin(st.wMinute * PI / 30 + st.wSecond * PI / 1800);
		ndlMin.rightPos.x = crt.right / 2 + ndlMin.sideLen * cos(st.wMinute * PI / 30 + st.wSecond * PI / 1800 + PI);
		ndlMin.rightPos.y = crt.bottom / 2 + ndlMin.sideLen * sin(st.wMinute * PI / 30 + st.wSecond * PI / 1800 + PI);

		// 초침
		ndlSec.edgeLen = radius * 0.65;
		ndlSec.rootLen = 0;
		ndlSec.sideLen = 0;
		ndlSec.edgePos.x = crt.right / 2 + ndlSec.edgeLen * cos(st.wSecond * PI / 30 - PI / 2);
		ndlSec.edgePos.y = crt.bottom / 2 + ndlSec.edgeLen * sin(st.wSecond * PI / 30 - PI / 2);
		ndlSec.rootPos.x = crt.right / 2;
		ndlSec.rootPos.y = crt.bottom / 2;
		ndlSec.leftPos.x = 0;	// 사용하지 않음
		ndlSec.leftPos.y = 0;	//
		ndlSec.rightPos.x = 0;	// 
		ndlSec.rightPos.y = 0;	//

		wsprintf(sTime, TEXT("%02d:%02d:%02d %cM"), st.wHour % 12, st.wMinute, st.wSecond, st.wHour < 12 ? 'A' : 'P');
		wsprintf(sTimeExceptSecond, TEXT("%02d:%02d %cM"), st.wHour % 12, st.wMinute, st.wHour < 12 ? 'A' : 'P');
		wsprintf(sDate, TEXT("%02d/%02d/%02d"), st.wYear, st.wMonth, st.wDay);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_ANALOG:
			isAnalog = true;
			break;
		case ID_DIGITAL:
			isAnalog = false;
			break;
		case ID_FONT:
			memset(&CFT, 0, sizeof(CHOOSEFONT));
			CFT.lStructSize = sizeof(CHOOSEFONT);
			CFT.hwndOwner = hWnd;
			CFT.lpLogFont = &lf;
			CFT.Flags = CF_EFFECTS | CF_SCREENFONTS;
			if(ChooseFont(&CFT))
			{
				Col = CFT.rgbColors;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case ID_HIDE:
			break;
		case ID_SECOND:
			showSecond = !showSecond;
			break;
		case ID_DATE:
			showDate = !showDate;
			break;
		case ID_ABOUT:
			if(DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT),
				hWnd, AboutDlgProc) == IDOK)
			{
			}
			break;
		case 40017:
			top = !top;
			SetWindowPos(hWnd, top ? HWND_TOPMOST : HWND_NOTOPMOST,
				0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		return 0;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
		if(isAnalog)
		{
			
			OldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
			hPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));
			OldPen = (HPEN)SelectObject(hdc, hPen);

			// 시침을 그린다.
			MoveToEx(hdc, ndlHour.rootPos.x, ndlHour.rootPos.y, NULL);
			LineTo(hdc, ndlHour.leftPos.x, ndlHour.leftPos.y);
			LineTo(hdc, ndlHour.edgePos.x, ndlHour.edgePos.y);
			LineTo(hdc, ndlHour.rightPos.x, ndlHour.rightPos.y);
			LineTo(hdc, ndlHour.rootPos.x, ndlHour.rootPos.y);

			// 분침을 그린다.
			MoveToEx(hdc, ndlMin.rootPos.x, ndlMin.rootPos.y, NULL);
			LineTo(hdc, ndlMin.leftPos.x, ndlMin.leftPos.y);
			LineTo(hdc, ndlMin.edgePos.x, ndlMin.edgePos.y);
			LineTo(hdc, ndlMin.rightPos.x, ndlMin.rightPos.y);
			LineTo(hdc, ndlMin.rootPos.x, ndlMin.rootPos.y);

			// 초침 표시 체크시 초침을 그린다.
			if(showSecond)
			{
				hPen = CreatePen(PS_SOLID, 0, RGB(128, 128, 128));
				OldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, ndlSec.rootPos.x, ndlSec.rootPos.y, NULL);
				LineTo(hdc, ndlSec.edgePos.x, ndlSec.edgePos.y);
			}
			DeleteObject(SelectObject(hdc, OldPen));
			SelectObject(hdc, OldBrush);
		}
		else
		{
			MyFont = CreateFontIndirect(&lf);
			OldFont = (HFONT)SelectObject(hdc, MyFont);
			SetTextColor(hdc, Col);
			if(showSecond)
				TextOut(hdc, 10, 10, sTime, lstrlen(sTime));
			else
				TextOut(hdc, 10, 10, sTimeExceptSecond, lstrlen(sTimeExceptSecond));
			if(showDate)
				TextOut(hdc, 10, 30, sDate, lstrlen(sDate));
			SelectObject(hdc, OldFont);
			DeleteObject(MyFont);
		}
        EndPaint(hWnd, &ps);
        return 0;
    case WM_DESTROY:
		KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}