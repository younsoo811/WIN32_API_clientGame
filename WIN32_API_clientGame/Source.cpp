#include <Windows.h>
#include<tchar.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
HINSTANCE hInst;
#define WM_ASYNC WM_USER+2

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS WndClass;

	hInst = hInstance;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = _T("Window Class Name");

	RegisterClass(&WndClass);

	hwnd = CreateWindow(
		_T("Window Class Name"),
		_T("Client Window"),
		WS_OVERLAPPEDWINDOW,
		50,
		50,
		900,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static WSADATA wsadata;
	static SOCKET s;
	static SOCKADDR_IN addr = { 0 };
	static TCHAR msg[10];

	static int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	static int shape = 0;
	static int recv_x1 = 0, recv_x2 = 0, recv_y1 = 0, recv_y2 = 0;
	static int recv_shape = 0;

	static int token2[5];
	static int mx = 0, my = 0;
	static bool Drag = false;
	static bool MyTurn = FALSE;

	HPEN redPen, blackPen, bluePen;
	static int  mX, mY;
	static int circle[4][8] = { 0, };
	static int recv_circle[4][8] = { 0, };

	int msgLen;
	char buffer[100];
	char *context;
	char *token;
	int pos_y = 0;
	int i = 0;

	switch (iMsg)
	{
	case WM_CREATE:
		WSAStartup(MAKEWORD(2, 2), &wsadata);                    //1.윈속 사용시작하기
		s = socket(AF_INET, SOCK_STREAM, 0);                    //2.소켓 생성하기
		addr.sin_family = AF_INET;
		addr.sin_port = 20;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		WSAAsyncSelect(s, hwnd, WM_ASYNC, FD_READ);                //서버에서 보낸 데이터를 받아야 하므로 FD_READ 이벤트에 대해 WM_ASYNC윈도우 메시지를 등록
		if (connect(s, (LPSOCKADDR)&addr, sizeof(addr)) == -1)    //3.연결 요구하기(접속실패시 종료)
			return 0;
		break;
	case WM_ASYNC:                                                //양방향 통신이므로 클라이언트가 다른 메시지를 처리하는 도중 서버에서 데이터를 보내면
		switch (lParam)                                            //WM_ASYNC와 lParam이 같이 도착한다.
		{
		case FD_READ:
			recv(s, buffer, 100, 0);
			_tcscpy_s(msg, buffer);
			context = NULL;
			token = strtok_s(msg, ",", &context);            //context에는 분리된 후 남은 문자열이 들어간다.
			while (token != NULL)
			{
				token2[i++] = atoi(token);
				//TextOut(hdc, 0, pos_y, token, _tcslen(token));
				token = strtok_s(NULL, ",", &context);
				pos_y += 20;
			}
			recv_circle[token2[0]][token2[1]] = 1;
			MyTurn = TRUE;
			InvalidateRgn(hwnd, NULL, TRUE);
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if (MyTurn)
		{
			mX = LOWORD(lParam);
			mY = HIWORD(lParam);
			if ((mY / 100) < 4 && (mX / 100) < 8)
			{
				circle[mY / 100][mX / 100] = 1;
				_stprintf_s(buffer, _T("%d,%d"), mY / 100, mX / 100);
				send(s, (LPSTR)buffer, 10, 0);
				//_stprintf_s(buffer, NULL);
			}
			InvalidateRgn(hwnd, NULL, TRUE);
			MyTurn = FALSE;
		}
		break;
	case WM_MOUSEMOVE:
		if (Drag)
		{
			x2 = LOWORD(lParam);
			y2 = HIWORD(lParam);
			InvalidateRgn(hwnd, NULL, TRUE);
		}
		break;
	case WM_LBUTTONUP:

		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		for (int i = 0; i < 5; i++)            //--------------프레임
		{
			MoveToEx(hdc, 0, i * 100, NULL);
			LineTo(hdc, 800, i * 100);
		}
		for (int i = 0; i < 9; i++)
		{
			MoveToEx(hdc, i * 100, 0, NULL);
			LineTo(hdc, i * 100, 400);
		}                                    //--------------프레임

		bluePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				SelectObject(hdc, bluePen);
				if (circle[i][j])
					Ellipse(hdc, 100 * j, 100 * i, 100 * j + 100, 100 * i + 100);
			}
		}

		redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				SelectObject(hdc, redPen);
				if (recv_circle[i][j])
					Ellipse(hdc, 100 * j, 100 * i, 100 * j + 100, 100 * i + 100);
			}
		}
		//_stprintf_s(msg, NULL);
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		closesocket(s);
		WSACleanup();
		PostQuitMessage(0);
		break;

	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
