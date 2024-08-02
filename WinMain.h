#ifndef WinMain_H
#define WinMain_H

#include "resource.h"
#include <windows.h>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#pragma comment(lib, "Dxva2.lib")

#define ID_TRAY_ICON 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_SETTINGS 1003

#define WM_TRAYICON (WM_USER + 1)

extern HINSTANCE hInstance;
extern NOTIFYICONDATA nid;
extern WORD originalGammaArray[3][256];
extern int minBrightness;
extern int maxBrightness;

double calculateAverageBrightness(const cv::Mat& frame);
void setBrightness(int brightness);
bool getGammaRamp(WORD gammaArray[3][256]);
void restoreGammaRamp(WORD gammaArray[3][256]);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);

#endif