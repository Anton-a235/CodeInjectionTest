#ifndef AGENT_H
#define AGENT_H

#include <stdio.h>

#include <Windows.h>

DWORD CommandShell(LPSTR cmd, LPSTR response);
VOID ExecCmd(LPSTR cmd, LPSTR output);
VOID CallFunction(LPSTR callStr, LPSTR resltStr);

#endif
