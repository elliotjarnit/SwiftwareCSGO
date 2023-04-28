#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "common.hpp"

namespace MemoryManager {
	template <class dataType>
	void wpm(dataType valToWrite, DWORD addressToWrite) {
		WriteProcessMemory(processHandle, (PVOID)addressToWrite, &valToWrite, sizeof(dataType), 0);
	}

	template <class dataType>
	dataType rpm(DWORD addressToRead) {
		dataType rpmbuffer;
		ReadProcessMemory(processHandle, (PVOID)addressToRead, &rpmbuffer, sizeof(dataType), 0);
		return rpmbuffer;
	}

	DWORD getProcessID(const CHAR* name) {
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(PROCESSENTRY32);
		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0UL);
		
		DWORD pid{};
		if (!snapshot) {
			return pid;
		}

		do {
			if (strcmp(name, pe.szExeFile) == 0) {
				pid = pe.th32ProcessID;
				break;
			}
		} while (Process32Next(snapshot, &pe));

		CloseHandle(snapshot);
		return pid;
	}

	DWORD getProcessAddress(const DWORD pid, const CHAR* name) {
		MODULEENTRY32 me;
		me.dwSize = sizeof(MODULEENTRY32);
		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

		DWORD address{};
		if (!snapshot) return address;

		do {
			if (strcmp(name, me.szModule) == 0) {
				address = reinterpret_cast<DWORD>(me.modBaseAddr);
				break;
			}
		} while (Module32Next(snapshot, &me));

		CloseHandle(snapshot);
		return address;
	}
}