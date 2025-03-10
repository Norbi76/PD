#include <iostream>
#include <windows.h>
//#include <winreg.h>
#include <tchar.h>
// why this works without 

using namespace std;

#define STRING_SIZE 256

int main() {
    HKEY hKey;
    LPCWSTR registryPath = L"SYSTEM\\CurrentControlSet\\Services";


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD l_dwIndex = 0;
        WCHAR l_wcSubKeyName[STRING_SIZE];
        DWORD l_dwSubKeyNameSize;
        WCHAR l_wcValueName[] = L"ImagePath";

        wcout << L"Subkeys in " << registryPath << L":" << endl;

        while (true) {
            l_dwSubKeyNameSize = sizeof(l_wcSubKeyName) / sizeof(l_wcSubKeyName[0]);
            //cout << "subKeyName Size: " << l_dwSubKeyNameSize << endl;
            if (RegEnumKeyEx(hKey, l_dwIndex, l_wcSubKeyName, &l_dwSubKeyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) { //why do i have to calculate the size of subKeyName, why can t i use (LPDWORD)256 instead
                break;
            }

            wcout << L" - " << l_wcSubKeyName << endl;

            HKEY hSubKey;
            if (RegOpenKeyEx(hKey, l_wcSubKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                WCHAR l_wcImagePath[1024];
                DWORD l_dwImagePathSize = sizeof(l_wcImagePath);

                if (RegQueryValueEx(hSubKey, L"ImagePath", NULL, NULL, (LPBYTE)l_wcImagePath, &l_dwImagePathSize) == ERROR_SUCCESS) { //same question as above (line 26)
                    wcout << L"    ImagePath: " << l_wcImagePath << endl;
                }
                else {
                    wcout << L"    ImagePath: (not found)" << endl;
                }
            }

            RegCloseKey(hSubKey);
            l_dwIndex++;
        }

        RegCloseKey(hKey);
    }
    else {
        cerr << "Failed to open registry key." << endl;
    }

    return 0;
}
