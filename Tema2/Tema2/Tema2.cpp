#define UNICODE
#define _UNICODE

#include <windows.h>
#include <setupapi.h>
#include <devguid.h> // Define GUID_DEVINTERFACE_USB_DEVICE etc.
#include <regstr.h> // For REGSTR_VAL_HARDWAREID
#include <tchar.h>  // For _tprintf etc. (UNICODE/ANSI compatibility)
#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

// Link to the SetupAPI library
#pragma comment(lib, "Setupapi.lib")

// Helper function to get a device property as a string
wstring GetDeviceProperty(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pDevInfoData, DWORD dwProperty) {
    DWORD dwType = 0;
    DWORD dwSize = 0;
    // First call to get the required size
    SetupDiGetDeviceRegistryProperty(hDevInfo, pDevInfoData, dwProperty, &dwType, NULL, 0, &dwSize);

    if (dwSize == 0) {
        return L""; // Property does not exist or is empty
    }

    vector<BYTE> buffer(dwSize);
    if (SetupDiGetDeviceRegistryProperty(hDevInfo, pDevInfoData, dwProperty, &dwType, buffer.data(), dwSize, &dwSize)) {
        if (dwType == REG_SZ) {
            return wstring(reinterpret_cast<wchar_t*>(buffer.data()));
        }
        else if (dwType == REG_MULTI_SZ) {
            // For Hardware IDs/Compatible IDs, return the first string in the list
            wstring result(reinterpret_cast<wchar_t*>(buffer.data()));
            return result; // May contain multiple strings separated by NULL, we take only the first one.
        }
    }
    return L"";
}


int main() {
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA deviceInfoData;
    DWORD i;

    _tprintf(_T("Listing USB devices \n\n"));

    // Get a handle to a set of device information
    // Enumerate ALL present devices and then filter the USB ones
    hDevInfo = SetupDiGetClassDevs(NULL, // All classes
        NULL, // Enumerator (e.g., "USB", "PCI") - NULL for all
        NULL, // HWND parent
        DIGCF_PRESENT | DIGCF_ALLCLASSES); // Only present devices

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error at SetupDiGetClassDevs: %lu\n", GetLastError());
        return 1;
    }

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData); i++) {
        wstring hardwareId = GetDeviceProperty(hDevInfo, &deviceInfoData, SPDRP_HARDWAREID);
        wstring compatibleId = GetDeviceProperty(hDevInfo, &deviceInfoData, SPDRP_COMPATIBLEIDS);
        wstring enumeratorName = GetDeviceProperty(hDevInfo, &deviceInfoData, SPDRP_ENUMERATOR_NAME);

        // Check if it is a USB device
        // Look for "USB\" in HardwareID, CompatibleID or if Enumerator is "USB"
        bool isUsbDevice = false;
        if (hardwareId.find(L"USB\\") != wstring::npos ||
            compatibleId.find(L"USB\\") != wstring::npos ||
            enumeratorName == L"USB")
        {
            isUsbDevice = true;
        }

        // Some USB Root Hub devices may have the enumerator "ROOT_HUB" etc.
        // We include these as well if they have "USB" in their IDs.
        if (!isUsbDevice && (hardwareId.find(L"USBROOT_HUB") != wstring::npos ||
            hardwareId.find(L"USBEHCI") != wstring::npos ||
            hardwareId.find(L"USBXHCI") != wstring::npos))
        {
            isUsbDevice = true;
        }


        if (isUsbDevice) {
            wstring devDesc = GetDeviceProperty(hDevInfo, &deviceInfoData, SPDRP_DEVICEDESC);
            wstring manufacturer = GetDeviceProperty(hDevInfo, &deviceInfoData, SPDRP_MFG);

            _tprintf(_T("Description: %s\n"), devDesc.empty() ? L"(Unknown)" : devDesc.c_str());
            _tprintf(_T("  Manufacturer: %s\n"), manufacturer.empty() ? L"(Unknown)" : manufacturer.c_str());
            _tprintf(_T("  Hardware ID(s): %s\n"), hardwareId.empty() ? L"(Unknown)" : hardwareId.c_str());
            _tprintf(_T("\n"));
        }
    }

    if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS) {
        fprintf(stderr, "Error enumerating devices: %lu\n", GetLastError());
    }

    // Release resources
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return 0;
}
