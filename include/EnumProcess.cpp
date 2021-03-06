/**************************************************************************
*   Copyright (C) 2004-2007 by Michael Medin <michael@medin.name>         *
*                                                                         *
*   This code is part of NSClient++ - http://trac.nakednuns.org/nscp      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>
#include <iostream>

#include "EnumProcess.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CEnumProcess::CEnumProcess() : PSAPI(NULL), VDMDBG(NULL), FVDMEnumTaskWOWEx(NULL)
{
	PSAPI = ::LoadLibrary(_TEXT("PSAPI"));
	if (PSAPI)  
	{
		// Find PSAPI functions
		FEnumProcesses = (PFEnumProcesses)::GetProcAddress(PSAPI, "EnumProcesses");
		FEnumProcessModules = (PFEnumProcessModules)::GetProcAddress(PSAPI, "EnumProcessModules");
#ifdef UNICODE
		FGetModuleFileNameEx = (PFGetModuleFileNameEx)::GetProcAddress(PSAPI, "GetModuleFileNameExW");
#else
		FGetModuleFileNameEx = (PFGetModuleFileNameEx)::GetProcAddress(PSAPI, "GetModuleFileNameExA");
#endif
	}

	VDMDBG = ::LoadLibrary(_TEXT("VDMDBG"));
	if (VDMDBG)
	{
		// Find VDMdbg functions
		FVDMEnumTaskWOWEx = (PFVDMEnumTaskWOWEx)::GetProcAddress(VDMDBG, "VDMEnumTaskWOWEx");
	}
}

CEnumProcess::~CEnumProcess()
{
	if (PSAPI) FreeLibrary(PSAPI);
	if (VDMDBG) FreeLibrary(VDMDBG);
}

struct find_16bit_container {
	std::list<CEnumProcess::CProcessEntry> *target;
	DWORD pid;
};
BOOL CALLBACK Enum16Proc( DWORD dwThreadId, WORD hMod16, WORD hTask16, PSZ pszModName, PSZ pszFileName, LPARAM lpUserDefined )
{
	find_16bit_container *container = reinterpret_cast<find_16bit_container*>(lpUserDefined);
	CEnumProcess::CProcessEntry pEntry;
	pEntry.dwPID = container->pid;
	pEntry.command_line = pszFileName;
	std::string::size_type pos = pEntry.command_line.find_last_of("\\");
	if (pos != std::string::npos)
		pEntry.filename = pEntry.command_line.substr(++pos);
	else
		pEntry.filename = pEntry.command_line;
	container->target->push_back(pEntry);
	return FALSE;
}


void CEnumProcess::enable_token_privilege(LPTSTR privilege)
{
	HANDLE hToken;                       
	TOKEN_PRIVILEGES token_privileges;                 
	DWORD dwSize;                       
	ZeroMemory (&token_privileges, sizeof(token_privileges));
	token_privileges.PrivilegeCount = 1;
	if ( !OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
		throw process_enumeration_exception("Failed to open process token: " + error::lookup::last_error());
	if (!LookupPrivilegeValue ( NULL, privilege, &token_privileges.Privileges[0].Luid)) { 
		CloseHandle (hToken);
		throw process_enumeration_exception("Failed to lookup privilege: " + error::lookup::last_error());
	}
	token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges ( hToken, FALSE, &token_privileges, 0, NULL, &dwSize)) { 
		CloseHandle (hToken);
		throw process_enumeration_exception("Failed to adjust token privilege: " + error::lookup::last_error());
	}
	CloseHandle (hToken);
}

void CEnumProcess::disable_token_privilege(LPTSTR privilege)
{
	HANDLE hToken;                       
	TOKEN_PRIVILEGES token_privileges;                 
	DWORD dwSize;                       
	ZeroMemory (&token_privileges, sizeof (token_privileges));
	token_privileges.PrivilegeCount = 1;
	if ( !OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
		throw process_enumeration_exception("Failed to open process token: " + error::lookup::last_error());
	if (!LookupPrivilegeValue ( NULL, privilege, &token_privileges.Privileges[0].Luid)) { 
		CloseHandle (hToken);
		throw process_enumeration_exception("Failed to lookup privilege: " + error::lookup::last_error());
	}
	token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
	if (!AdjustTokenPrivileges ( hToken, FALSE, &token_privileges, 0, NULL, &dwSize)) { 
		CloseHandle (hToken);
		throw process_enumeration_exception("Failed to adjust token privilege: " + error::lookup::last_error());
	}
	CloseHandle (hToken);
}

CEnumProcess::process_list CEnumProcess::enumerate_processes(bool expand_command_line, bool find_16bit, CEnumProcess::error_reporter *error_interface, unsigned int buffer_size) {
	try {
		enable_token_privilege(SE_DEBUG_NAME);
	} catch (process_enumeration_exception &e) {
		if (error_interface!=NULL)
			error_interface->report_warning(e.reason());
	} 

	std::list<CProcessEntry> ret;
	DWORD *dwPIDs = new DWORD[buffer_size+1];
	DWORD cbNeeded = 0;
	BOOL OK = FEnumProcesses(dwPIDs, buffer_size*sizeof(DWORD), &cbNeeded);
	if (cbNeeded >= DEFAULT_BUFFER_SIZE*sizeof(DWORD)) {
		delete [] dwPIDs;
		if (error_interface!=NULL)
			error_interface->report_debug("Need larger buffer: " + strEx::s::xtos(buffer_size));
		return enumerate_processes(expand_command_line, find_16bit, error_interface, buffer_size * 10); 
	}
	if (!OK) {
		delete [] dwPIDs;
		throw process_enumeration_exception("Failed to enumerate process: " + error::lookup::last_error());
	}
	unsigned int process_count = cbNeeded/sizeof(DWORD);
	for (unsigned int i = 0;i <process_count; ++i) {
		if (dwPIDs[i] == 0)
			continue;
		CProcessEntry entry;
		entry.hung = false;
		try {
// 			if (error_interface!=NULL)
// 				error_interface->report_debug_enter(_T("describe_pid"));
			try {
				entry = describe_pid(dwPIDs[i], expand_command_line);
			} catch (process_enumeration_exception &e) {
				if (error_interface!=NULL)
					error_interface->report_debug(e.reason());
				if (expand_command_line) {
					try {
				entry = describe_pid(dwPIDs[i], false);
					} catch (process_enumeration_exception &e) {
						if (error_interface!=NULL)
							error_interface->report_debug(e.reason());
					}
				}
			}
// 			if (error_interface!=NULL)
// 				error_interface->report_debug_exit(_T("describe_pid"));
			if (VDMDBG!=NULL&&find_16bit) {
				if (error_interface!=NULL)
					error_interface->report_debug("Looking for 16bit apps");
				if(stricmp(entry.filename.substr(0,9).c_str(), "NTVDM.EXE") == 0) {
					find_16bit_container container;
					container.target = &ret;
					container.pid = entry.dwPID;
					FVDMEnumTaskWOWEx(entry.dwPID, (TASKENUMPROCEX)&Enum16Proc, (LPARAM) &container);
				}
			}
			ret.push_back(entry);
		} catch (process_enumeration_exception &e) {
			if (error_interface!=NULL)
				error_interface->report_error("Unhandeled exception describing PID: " + strEx::s::xtos(dwPIDs[i]) + ": " + e.reason());
		} catch (...) {
			if (error_interface!=NULL)
				error_interface->report_error("Unknown exception describing PID: " + strEx::s::xtos(dwPIDs[i]));
		}
	}

	std::vector<DWORD> hung_pids = find_crashed_pids(error_interface);
	for (process_list::iterator entry = ret.begin(); entry != ret.end(); ++entry) {
		if (std::find(hung_pids.begin(), hung_pids.end(), entry->dwPID) != hung_pids.end())
			(*entry).hung = true;
		else
			(*entry).hung = false;
	}

	delete [] dwPIDs;
	return ret;
}

struct enum_data {
	CEnumProcess::error_reporter * error_interface;
	std::vector<DWORD> crashed_pids;

};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam ) {
	enum_data *data = reinterpret_cast<enum_data*>(lParam);
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	if (GetWindow(hwnd, GW_OWNER) != NULL)
		return TRUE;
	PDWORD result;
	if (!SendMessageTimeout(hwnd, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 500, reinterpret_cast<PDWORD_PTR>(&result))) {
		if (data->error_interface!=NULL)
			data->error_interface->report_debug("pid: " + strEx::s::xtos(pid) + " was hung");
		data->crashed_pids.push_back(pid);
	}

// 	TCHAR *buffer = new TCHAR[1024];
// 	int len = GetWindowText(hwnd, buffer, 1023);
// 	buffer[30] = 0;
// 	if (data->error_interface!=NULL)
// 		data->error_interface->report_debug(_T("pid: ") + res + strEx::itos(pid) + _T(" - ") + strEx::itos(len) + _T(" - ") + buffer);
// 	//std::wcout << _T("pid: ") << pid << _T(" - ") << len << _T(" : ") << buffer << std::endl;
// 	delete [] buffer;
	return TRUE;
}

std::vector<DWORD> CEnumProcess::find_crashed_pids(CEnumProcess::error_reporter * error_interface) {
	enum_data data;
	data.error_interface = error_interface;
	if(!EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(&data))) {
		if (error_interface)
			error_interface->report_error("Failed to enumerate windows: " + utf8::cvt<std::string>(error::lookup::last_error()));
	}
	return data.crashed_pids;
}

CEnumProcess::CProcessEntry CEnumProcess::describe_pid(DWORD pid, bool expand_command_line) {
	CProcessEntry entry;
	entry.dwPID = pid;
	// Open process to get filename
	DWORD openArgs = PROCESS_QUERY_INFORMATION|PROCESS_VM_READ;
//	if (expand_command_line)
//		openArgs |= PROCESS_VM_OPERATION;
	HANDLE hProc = OpenProcess(openArgs, FALSE, pid);
	if (!hProc)
		throw process_enumeration_exception(GetLastError(), "Failed to open process: " + strEx::s::xtos(pid) + ": ");
	if (expand_command_line)
		entry.command_line = utf8::cvt<std::string>(GetCommandLine(hProc));
	HMODULE hMod;
	DWORD size;
	// Get the first module (the process itself)
	if( FEnumProcessModules(hProc, &hMod, sizeof(hMod), &size) ) {
		TCHAR buffer[MAX_FILENAME+1];
		if( !FGetModuleFileNameEx( hProc, hMod, reinterpret_cast<LPTSTR>(&buffer), MAX_FILENAME) ) { 
			CloseHandle(hProc);
			throw process_enumeration_exception("Failed to find name for: " + strEx::s::xtos(pid) + ": " + error::lookup::last_error());
		} else {
			std::wstring path = buffer;
			std::wstring::size_type pos = path.find_last_of(_T("\\"));
			if (pos != std::wstring::npos) {
				path = path.substr(++pos);
			}
			entry.filename = utf8::cvt<std::string>(path);
		}
	}

	CloseHandle(hProc);
	return entry;
}

typedef struct _PROCESS_BASIC_INFORMATION {
	LONG ExitStatus;
	LPVOID PebBaseAddress;
	ULONG_PTR AffinityMask;
	LONG BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

LPVOID GetPebAddress(HANDLE ProcessHandle) {
	PFNtQueryInformationProcess NtQueryInformationProcess = (PFNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess == NULL)
		throw CEnumProcess::process_enumeration_exception("Failed to load NtQueryInformationProcess");
	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);
	return pbi.PebBaseAddress;
}


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;

bool IsWow64(HANDLE hProcess, bool def = false) {
	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
	if(NULL != fnIsWow64Process) {
		if (!fnIsWow64Process(hProcess,&bIsWow64))
			return def;
	}
	return bIsWow64?true:false;
}

std::wstring CEnumProcess::GetCommandLine(HANDLE hProcess) {

	UNICODE_STRING commandLine;
#ifdef _WIN64
	LPVOID pebAddress = GetPebAddress(hProcess);
	LPVOID rtlUserProcParamsAddress;
	if (!ReadProcessMemory(hProcess, (PCHAR)pebAddress + 0x20, &rtlUserProcParamsAddress, sizeof(LPVOID), NULL))
		throw process_enumeration_exception("Could not read the address of ProcessParameters: " + error::lookup::last_error());
	if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 0x70, &commandLine, sizeof(commandLine), NULL))
		throw process_enumeration_exception("Could not read commandline: " + error::lookup::last_error());
#else
	bool osIsWin64 = IsWow64(GetCurrentProcess());
	if (!IsWow64(hProcess, !osIsWin64))
		return _T("");
	LPVOID pebAddress = GetPebAddress(hProcess);
	LPVOID rtlUserProcParamsAddress;
	if (!ReadProcessMemory(hProcess, (PCHAR)pebAddress + 0x10, &rtlUserProcParamsAddress, sizeof(LPVOID), NULL))
		throw process_enumeration_exception("Could not read the address of ProcessParameters: " + error::lookup::last_error());
	if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 0x40, &commandLine, sizeof(commandLine), NULL))
		throw process_enumeration_exception("Could not read commandline: " + error::lookup::last_error());
#endif

	/* allocate memory to hold the command line */
	wchar_t *commandLineContents = new wchar_t[commandLine.Length+2];
	memset(commandLineContents, 0, commandLine.Length);

	/* read the command line */
	if (!ReadProcessMemory(hProcess, commandLine.Buffer, commandLineContents, commandLine.Length, NULL)) {
		delete [] commandLineContents;
		throw process_enumeration_exception("Could not read commandline string: " + error::lookup::last_error());
	}

	commandLineContents[(commandLine.Length/sizeof(WCHAR))] = '\0';
	std::wstring ret = commandLineContents;
	delete [] commandLineContents;

	return ret;
}

