#include "WinMain.h"
#include <csignal>

HINSTANCE hInstance;
NOTIFYICONDATA nid;
WORD originalGammaArray[3][256];
int minBrightness = 15;
int maxBrightness = 100;

/**
 * @brief Calculates the average brightness of a given frame.
 *
 * @param frame The input frame from the webcam.
 * @return The average brightness value.
 */
double calculateAverageBrightness(const cv::Mat& frame) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Scalar meanBrightness = cv::mean(gray);
    return meanBrightness[0];
}

/**
 * @brief Sets the brightness of the display.
 *
 * @param brightness The desired brightness level (0-100).
 */
void setBrightness(int brightness) {
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);
  
    for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); i++) {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            HDC hDC = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (!hDC) {
                break;
            }
            if (brightness < minBrightness) {
                brightness = minBrightness;
            }
            else if (brightness > maxBrightness) {
                brightness = maxBrightness;
            }

            WORD gammaArray[3][256];
            for (int i = 0; i < 256; i++) {
                int value = (i * brightness / 100);
                if (value > 255) {
                    value = 255;
                }
                gammaArray[0][i] = gammaArray[1][i] = gammaArray[2][i] = (WORD)((value << 8) | value);
            }

            SetDeviceGammaRamp(hDC, gammaArray);
            DeleteDC(hDC);
            break;
        }
    }
}

/**
 * @brief Retrieves the current gamma ramp settings.
 *
 * @param gammaArray Array to store the current gamma ramp values.
 * @return True if successful, false otherwise.
 */
bool getGammaRamp(WORD gammaArray[3][256]) {
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);
    for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); i++) {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            HDC hDC = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (!hDC) {
                break;
            }

            BOOL result = GetDeviceGammaRamp(hDC, gammaArray);
            DeleteDC(hDC);
            return result == TRUE;
        }
    }
    return false;
}


/**
 * @brief Restores the gamma ramp to the original settings.
 *
 * @param gammaArray Array containing the original gamma ramp values.
 */
void restoreGammaRamp(WORD gammaArray[3][256]) {
    DISPLAY_DEVICE dd;
    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    for (int i = 0; EnumDisplayDevices(NULL, i, &dd, 0); i++) {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            HDC hDC = CreateDC(NULL, dd.DeviceName, NULL, NULL);
            if (!hDC) {
                break;
            }

            SetDeviceGammaRamp(hDC, gammaArray);
            DeleteDC(hDC);
            break; 
        }
    }
}

/**
 * @brief Initializes the NOTIFYICONDATA structure.
 *
 * @param hInstance Handle to the current instance.
 * @param hwnd Handle to the window.
 * @param nid Reference to the NOTIFYICONDATA structure.
 */
void InitializeNotifyIconData(HINSTANCE hInstance, HWND hwnd, NOTIFYICONDATA& nid) {
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
    wcscpy_s(nid.szTip, L"Dynamic Brightness Adjuster");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

/**
 * @brief Window procedure for handling messages.
 * Creates a context menu, gets the position of the tray icon, shows the context menu.
 * Destroys the menu, Removes the tray icon and exits.
 *
 *
 * @param hwnd Handle to the window.
 * @param uMsg Message identifier.
 * @param wParam Additional message information.
 * @param lParam Additional message information.
 * @return Result of the message processing.
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONDOWN) {
            HMENU hMenu = CreatePopupMenu();
            InsertMenu(hMenu, 0, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

            POINT trayIconPosition;
            GetCursorPos(&trayIconPosition);
            SetForegroundWindow(hwnd);

            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, trayIconPosition.x, trayIconPosition.y, 0, hwnd, NULL);

            DestroyMenu(hMenu);
        }
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
        }
        break;
    }
    case WM_DESTROY: {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/**
 * @brief Main function of the program.
 * Stores the original gamma ramp.
 * Opens the default camera and Capture a new frame from the camera, sets the brightness.
 * Processes Windows messages.
 *
 * @param hInstance Handle to the current instance.
 * @param hPrevInstance Handle to the previous instance.
 * @param lpCmdLine Command line for the application.
 * @param nCmdShow Controls how the window is to be shown.
 * @return Exit code of the application.
 */
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayApp";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"TrayApp", L"Tray Application", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    InitializeNotifyIconData(hInstance, hwnd, nid);
    Shell_NotifyIcon(NIM_ADD, &nid);

    if (!getGammaRamp(originalGammaArray)) {
        MessageBox(NULL, L"Error: Could not get original gamma ramp. Try again!", L"Gamma Ramp Error", MB_OK | MB_ICONERROR);

        return -1;
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        MessageBox(NULL, L"Error: Could not open camera. Please check if the webcam is connected.", L"Camera Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            MessageBox(NULL, L"Error: Could not capture frame. Try again!", L"Frame Error", MB_OK | MB_ICONERROR);
            break;
        }

        double avgBrightness = calculateAverageBrightness(frame);
        avgBrightness = avgBrightness + 20;
        int brightness = static_cast<int>((avgBrightness / 255.0) * 100);
        setBrightness(brightness);

        MSG WindowsMessages;
        while (PeekMessage(&WindowsMessages, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&WindowsMessages);
            DispatchMessage(&WindowsMessages);
            if (WindowsMessages.message == WM_QUIT) {
                goto cleanup;
            }
        }
        Sleep(30);
    }

cleanup:
    cap.release();
    restoreGammaRamp(originalGammaArray);
    cv::destroyAllWindows();
    return 0;
}