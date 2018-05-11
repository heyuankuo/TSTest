/************************************************************
* 功 能： 查找桌面快捷方式图标
* 类 名： test_1
* 创建人：heyuankuo
* 创建时间：2018-5-11 11:35:48
* Ver 变更日期 负责人 变更内容
* ───────────────────────────────────
* V0.01 2018-5-11 heyuankuo 初版
*
* Copyright (c) 2018 ZYRJ Corporation. All rights reserved.
*************************************************************/
#include "stdafx.h"
#include <iostream>
#include <objbase.h>
#include <armadillo>
#include "Commctrl.h"
#include "ExceptionLog.h"
#include "Util.h"
#include "Shlobj.h"
#include "Shlwapi.h"
#include "VersionHelpers.h"
#include <fstream>

using std::ifstream;
using std::ios_base;

using namespace std;
using namespace arma;

/**
 * 获取系统桌面
 * 
 */
HWND GetRealDesktopWnd()
{
	// if (win_util::GetWinVersion() >= win_util::WINVERSION_VISTA)
	{
		HWND hwnd = ::FindWindow(_T("WorkerW"), _T(""));
		while (hwnd)
		{
			HWND hwndShellView = ::FindWindowEx(hwnd, NULL, _T("SHELLDLL_DefView"), _T(""));
			if (hwndShellView)
			{
#ifdef SHELLDLL_DefView_AS_DESKTOP
				return hwndShellView;
#else
				return hwnd;
#endif
			}
			hwnd = ::FindWindowEx(NULL, hwnd, _T("WorkerW"), _T(""));
		}
	}

	// xp 
	HWND hwnd = ::FindWindow(_T("Progman"), _T("Program Manager"));
#ifdef SHELLDLL_DefView_AS_DESKTOP
	return ::FindWindowEx(hwnd, NULL, _T("SHELLDLL_DefView"), _T(""));
#else
	return hwnd;
#endif

};

// 将DLL 读取到内存中
void ReadDll2Mem(LPCTSTR filename, BYTE **dllbuf, UINT *buflen)
{
	WIN32_FIND_DATA fileInfo = { 0 };
	HANDLE hd = FindFirstFile(filename, &fileInfo);
	*buflen = fileInfo.nFileSizeLow;
	FindClose(hd);

	*dllbuf = new BYTE[*buflen]();

	ifstream ifs(filename, ios_base::binary);
	ifs.read((char *)*dllbuf, *buflen);
	ifs.clear();
	ifs.close();
}

int main()
{
	DWORD dwProcessID = GetProcessID(L"notepad.exe");
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION, FALSE, dwProcessID);//获取进程句柄
	if (NULL == hProcess)
	{
		int err = GetLastError();
	}

	BYTE *dllbuf = NULL;
	UINT buflen = 0;
	ReadDll2Mem(L"E:\\workspace\\CEFTest\\CefTest_1\\x64\\Release\\HookTest.dll", &dllbuf, &buflen);

	LPVOID RemoteBuf = VirtualAllocEx(hProcess, NULL, buflen, MEM_COMMIT, PAGE_READWRITE);//远程申请内存
	if (NULL == RemoteBuf)
	{
		// 申请失败
	}
	else
	{
		SIZE_T wsize = 0;
		WriteProcessMemory(hProcess, RemoteBuf, dllbuf, buflen, &wsize);

		VirtualFreeEx(hProcess, RemoteBuf, buflen, MEM_COMMIT);
	}
	CloseHandle(hProcess);
	delete[] dllbuf;
	
	HWND  hwndSysListView32 = NULL;
	enOsInfo os_info;
	if (GetOsInfo(&os_info))
	{
		if (OS_WIN7_64 == os_info)
		{
			HWND  hwndParent = ::FindWindow(L"Progman", L"Program Manager");
			HWND  hwndSHELLDLL_DefView = ::FindWindowEx(hwndParent, NULL, L"SHELLDLL_DefView", NULL);
			hwndSysListView32 = ::FindWindowEx(hwndSHELLDLL_DefView, NULL, L"SysListView32", L"FolderView");
		}
	}
	else
	{
		
	}

	int Nm = ListView_GetItemCount(hwndSysListView32);
	int sNm = 0;

	if (Nm >= 10)
	{
		sNm = 10;
	}
	else
	{
		sNm = Nm;
	}

	for (int i = 0; i < sNm; i++)
	{
		int x = 960 + 150 * cos(i * 36 * 3.1415926 / 180);
		int y = 540 + 150 * sin(i * 36 * 3.1415926 / 180);
		HRESULT hr = ::SendMessage(hwndSysListView32, LVM_SETITEMPOSITION, i, MAKELPARAM(x, y));
		if (!SUCCEEDED(hr))
		{
			printf("SendMessage err %08X\n", hr);
		}
	}

	BOOL result = ListView_RedrawItems(hwndSysListView32, 0, ListView_GetItemCount(hwndSysListView32) - 1);
	if (FALSE == result)
	{
		printf("ListView_RedrawItems err \n");
	}
	::UpdateWindow(hwndSysListView32);
	return 0;
}

