#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <ctime>
#include <string>
#include <sstream> 
#include <fstream>
#include <amp.h>
#include <thread>
#include "hazedumpers.h"

using namespace hazedumper;
using namespace netvars;
using namespace signatures;

uintptr_t modulebaseaddr;
DWORD procId;
HWND csgowindow;
HANDLE hProcess;
bool toggle = true;
bool toggle2 = true;
int MaxPlayers = 64;


uintptr_t ModuleAddr(const char* DesiredModule) //GetModuleBaseAddr function (Speicheraddresse im RAM)
{
	HANDLE SysSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32 mEntry32 = {};
	mEntry32.dwSize = sizeof(mEntry32);

	ZeroMemory((void*)mEntry32.szModule, sizeof(mEntry32.szModule));

	for (size_t i = 0; i < mEntry32.dwSize; ++i)
	{
		Module32Next(SysSnap, &mEntry32);

		if (!strcmp(mEntry32.szModule, DesiredModule))
		{
			CloseHandle(SysSnap);
			return (uintptr_t)mEntry32.modBaseAddr;
		}
	}
	MessageBoxA(NULL, "Error finding the Address of the module. This could be because of insufficient privileges. Try running as Admin", "Error", NULL);
	system("pause");
	return true;
}


void polymorphic() // Funktion für VAC (macht nicht wirklich was)
{
	std::srand(std::time(0)); //random seed erzeugen

	int count = 0;
	for (count; count < 10; count++)
	{
		int index = rand() % (2 - 0 + 1) + 0;



		switch (index)
		{

		case 0:

			__asm __volatile
			{
				mov eax, 0
				add bp, ax
				mov eax, 0
				mov ebx, 0
				mov ax, bp
				mov bl, 7
				mul cx
				_emit 0x90


			}



		case 1:

			_asm
			{
				add al, al
				_emit 0x34
				_emit 0x35
				_emit 0x90
				_emit 0x36
				_emit 0x37
				_emit 0x90
				_emit 0x90

			}

		case 2:
			__asm __volatile
			{
			add    eax, 0;
			 dec    ecx;
			 add    eax, 0;
			 inc    ecx;
			 inc    edi;
			 _emit 0xb8
			 dec    ecx
			 inc    ecx;
			 dec    ecx;
			 inc    ecx;
			}


		}

	}
}


struct glowconfig {
	float red = 0, green = 0, blue = 0, alpha = 0.8;
	uint8_t padding[8];
	float unknown = 1;
	uint8_t padding2[4];
	BYTE renderOccluded = 1;
	BYTE renderUnoccluded = 0;
	BYTE fullBloom = 0;
}config; //GlowConfig struct


uintptr_t Read(uintptr_t addr) {
	uintptr_t val;
	Toolhelp32ReadProcessMemory(procId, (LPVOID)addr, &val, sizeof(val), NULL);
	return val;
}

auto TitleGen = [](int num) {
	std::string nameoftitle;
	for (int i = 0; i < num; i++)
		nameoftitle += rand() % 255 + 1;
	return nameoftitle;
};




void rgb()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	while (true)
	{
		for (int i = 0; i < 16; ++i)
		{
			SetConsoleTextAttribute(hConsole, i);
			std::cout << "F1 Switchen" << std::endl;
			Sleep(700);
			system("CLS");
			if (i == 7)
			{
				++i;
			}
			if (i == 15)
			{
				i = 0;
			}
		}

		Sleep(1);
	}

}



int main(int argc, char* argv[]) {

	polymorphic();
	SetConsoleTitleA(TitleGen(rand() % 100 + 30).c_str()); //Random Fensternamen

	std::thread menuthread(rgb);

	HWND hconsole = GetConsoleWindow();
	SetWindowPos(hconsole, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	std::cout << std::endl;
	std::cout << "Random Consol Fensternamen und Codesegmente weil VAC dumm ist" << std::endl;
	std::cout << "F1 für an/aus" << std::endl;
	Sleep(100);

	HWND GameWnd = FindWindow(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(GameWnd, &procId);
	uintptr_t modulebaseaddr = ModuleAddr("client.dll");
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, procId); 

	

	while (true)
	{

		if (GetAsyncKeyState(VK_F1) & 1)
		{
			toggle = !toggle;
		}
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			toggle2 = !toggle2;
		}

		if (true)
		{

			uintptr_t LocalPlayer = Read(modulebaseaddr + dwLocalPlayer); //local player Baseaddress
			

			do
			{
				uintptr_t LocalPlayer = Read(modulebaseaddr + dwLocalPlayer);
			} while (LocalPlayer == NULL); // warten auf neues Spiel wenn im Menü

//			std::cout << "Local Player : " <<LocalPlayer;
			uintptr_t dwGlowManager = Read(modulebaseaddr + dwGlowObjectManager);
			int LocalTeam = Read(LocalPlayer + m_iTeamNum);
/*			std::cout << Read(LocalPlayer + m_iHealth) << std::endl;
			std::cout << "procId: " << procId << std::endl;
			std::cout << "modulbase: " << modulebaseaddr << std::endl;*/
			Sleep(10);


			for (int i = 1; i < MaxPlayers; i++) {
				uintptr_t dwEntity = Read(modulebaseaddr + dwEntityList + i * 0x10);
				
				int GlowIndex = Read(dwEntity + m_iGlowIndex);
				int EntityHealth = Read(dwEntity + m_iHealth);
				int IsDormant = Read(dwEntity + m_bDormant);
				int EntityTeam = Read(dwEntity + m_iTeamNum);
				polymorphic();

				bool test = true;
				if (dwEntity && toggle2) {
					WriteProcessMemory(hProcess, (LPVOID)(dwEntity + m_bSpotted), &test, sizeof(test), NULL);
				}
				

				if (LocalTeam == EntityTeam && IsDormant == NULL && EntityHealth != 0 && toggle)
				{
					config.red = 0;
					config.green = 0.5;
					config.blue = 0.5;

					WriteProcessMemory(hProcess, (LPVOID)(dwGlowManager + (GlowIndex * 0x38) + 0x8), &config, sizeof(config), NULL); //nach update wurde aus offset 0x4 -> 0x8

				}
				else if (LocalTeam != EntityTeam && IsDormant == NULL && EntityHealth != 0 && toggle)
				{
					config.red = 0.9;
					config.green = 0.2;
					config.blue = 0;
					WriteProcessMemory(hProcess, (LPVOID)(dwGlowManager + (GlowIndex * 0x38) + 0x8), &config, sizeof(config), NULL);

				}

			}
		}
		else
		{
			Sleep(20);
		}

	}
}