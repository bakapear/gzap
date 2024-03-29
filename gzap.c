// cc gzap.c -o gzap -ldwmapi -s -mwindows
#include <dwmapi.h>
#include <stdio.h>
#include <windows.h>

RECT workarea;
int padding = 5;
int keep = 0;

BOOL IsOpenedWindow(HWND hwnd) {
  HWND hwndTry, hwndWalk = NULL;
  if (!IsWindowVisible(hwnd)) return 0;

  if (IsIconic(hwnd)) return 0;

  hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
  while (hwndTry != hwndWalk) {
    hwndWalk = hwndTry;
    hwndTry = GetLastActivePopup(hwndWalk);
    if (IsWindowVisible(hwndTry)) break;
  }
  if (hwndWalk != hwnd) return 0;
  TITLEBARINFO ti;
  ti.cbSize = sizeof(ti);
  GetTitleBarInfo(hwnd, &ti);
  if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) return 0;

  if (GetWindowLongA(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) return 0;

  wchar_t cls[32];
  GetClassNameW(hwnd, cls, sizeof(cls));

  if (wcscmp(cls, L"ApplicationFrameWindow") == 0) {
    DWORD cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked != 0) return 0;
  }

  return 1;
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
  if (IsOpenedWindow(hWnd)) {
    RECT rect;
    RECT rect2;
    DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
    GetWindowRect(hWnd, &rect2);

    int x = rect.left + padding;
    int y = rect.top + padding;
    int w = rect.right - x - padding;
    int h = rect.bottom - y - padding;

    x += (workarea.left == rect.left) ? padding : 0;
    y += (workarea.top == rect.top) ? padding : 0;
    w -= ((workarea.right == rect.right) ? padding : 0) + ((workarea.left == rect.left) ? padding : 0);
    h -= ((workarea.bottom == rect.bottom) ? padding : 0) + ((workarea.top == rect.top) ? padding : 0);

    // windows is amazing
    x -= (rect.left - rect2.left);
    y -= (rect.top - rect2.top);
    w -= (rect.right - rect2.right) * 2;
    h -= (rect.bottom - rect2.bottom);

    if (!keep) ShowWindow(hWnd, SW_NORMAL);
    SetWindowPos(hWnd, NULL, x, y, w, h, 0x0200);
  }

  return TRUE;
}

int main(int argc, char *argv[]) {
  if (argc > 1) padding = atoi(argv[1]);
  if (argc > 2 && atoi(argv[2])) keep = 1;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

  EnumWindows(enumWindowCallback, 0);

  return 0;
}
