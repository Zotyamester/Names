#define _CRT_SECURE_NO_WARNINGS
#include "resource.h"
#include <windows.h>
#include <string.h>
#include <ctype.h>

#include "stack.h"

const wchar_t g_szClassName[] = L"w32t";
HINSTANCE hInst;

LPSTACK g_lpsUndo = NULL;
LPSTACK g_lpsRedo = NULL;

void RepositionControl(HWND hwnd, int cx, int cy, int cw, int ch)
{
    SetWindowPos(hwnd, HWND_TOP, cx, cy, cw, ch, 0);
    RECT rect = { .left = cx, .right = cx + cw, .top = cy, .bottom = cy + ch };
    InvalidateRect(hwnd, &rect, FALSE);
}

BOOL IsWhitespace(LPCWSTR str)
{
    while (*str)
    {
        if (!isspace(*str))
        {
            return FALSE;
        }
        str++;
    }
    return TRUE;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbrBkgnd;
    switch (msg)
    {
    case WM_CREATE:
        {
            HMENU hMenu, hSubMenu;

            hMenu = CreateMenu();
            if (!hMenu)
            {
                goto err;
            }

            hSubMenu = CreatePopupMenu();
            if (!hSubMenu)
            {
                goto err;
            }
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_NEW, L"&New");
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, L"&Open");
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_SAVE, L"&Save");
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&File");

            hSubMenu = CreatePopupMenu();
            if (!hSubMenu)
            {
                goto err;
            }
            AppendMenu(hSubMenu, MF_STRING | MF_GRAYED, ID_EDIT_UNDO, L"&Undo");
            AppendMenu(hSubMenu, MF_STRING | MF_GRAYED, ID_EDIT_REDO, L"&Redo");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&Edit");

            hSubMenu = CreatePopupMenu();
            if (!hSubMenu)
            {
                goto err;
            }
            AppendMenu(hSubMenu, MF_STRING, ID_HELP_ABOUT, L"&About");
            AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&Help");

            SetMenu(hwnd, hMenu);


            HFONT hFont = CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
                OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS, L"Calibri");
            if (!hFont)
            {
                goto err;
            }


            HWND lText = CreateWindow(
                L"STATIC",
                L"Names",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                10, 10, 100, 100,
                hwnd, (HMENU)IDC_TEXT, hInst, NULL);
            if (!lText)
            {
                goto err;
            }
            SendMessage(lText, WM_SETFONT, hFont, TRUE);
            hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));

            HWND lstNames = CreateWindow(
                L"LISTBOX",
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_BORDER,
                10, 10, 100, 100,
                hwnd, (HMENU)IDC_NAMES, hInst, NULL);
            if (!lstNames)
            {
                goto err;
            }
            SendMessage(lstNames, WM_SETFONT, hFont, TRUE);

            HWND txtName = CreateWindow(
                L"EDIT",
                NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOVSCROLL | WS_BORDER,
                10, 10, 100, 100,
                hwnd, (HMENU)IDC_NAME, hInst, NULL);
            if (!txtName)
            {
               goto err;
            }
            SendMessage(txtName, WM_SETFONT, hFont, TRUE);

            HWND btnAdd = CreateWindow(
                L"BUTTON",
                L"Add",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                10, 10, 100, 100,
                hwnd, (HMENU)IDC_ADD, hInst, NULL);
            if (btnAdd) // !
            {
                SendMessage(btnAdd, WM_SETFONT, hFont, TRUE);
                return 0;
            }
        err:
            MessageBox(hwnd, L"An unexpected error has occured while creating the UI.", L"Error", MB_OK | MB_ICONERROR);
            DestroyWindow(hwnd);
            return 0;
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_NEW:
            SendDlgItemMessage(hwnd, IDC_NAMES, LB_RESETCONTENT, 0, 0);
            return 0;
        case ID_FILE_OPEN:
            {
                WCHAR szPath[MAX_PATH] = { 0 };

                OPENFILENAME ofn = { sizeof(ofn) };
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0All Files\0*.*\0";
                ofn.lpstrFile = szPath;
                ofn.nMaxFile = ARRAYSIZE(szPath);
                ofn.lpstrDefExt = L"txt";
                if (GetOpenFileName(&ofn))
                {
                    HFILE hFile = CreateFile(szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile == INVALID_HANDLE_VALUE)
                    {
                        MessageBox(hwnd, L"Could not open file.", L"Error!", MB_ICONERROR | MB_OK);
                    }
                    else
                    {
                        DWORD dwBytesToRead = GetFileSize(hFile, NULL);

                        CHAR* buf = GlobalAlloc(GPTR, (dwBytesToRead + 1) * sizeof(CHAR));
                        if (!buf)
                        {
                            MessageBox(hwnd, L"Could not allocate memory.", L"Error!", MB_ICONERROR | MB_OK);
                        }
                        else
                        {
                            DWORD dwBytesRead = 0;
                            while (dwBytesRead < dwBytesToRead)
                            {
                                if (!ReadFile(hFile, buf + dwBytesRead, dwBytesToRead - dwBytesRead, &dwBytesRead, NULL))
                                {
                                    MessageBox(hwnd, L"Could not read from file.", L"Error!", MB_ICONERROR | MB_OK);
                                    break;
                                }
                            }

                            SendDlgItemMessage(hwnd, IDC_NAMES, LB_RESETCONTENT, 0, 0);
                            StackClear(&g_lpsUndo);
                            StackClear(&g_lpsRedo);


                            WCHAR* mov = ((WCHAR*)buf) + 1;
                            while (1)
                            {
                                WCHAR* next = wcschr(mov, L'\r');
                                if (!next)
                                {
                                    break;
                                }
                                *next = L'\0';
                                if (!IsWhitespace(mov) && SendDlgItemMessage(hwnd, IDC_NAMES, LB_FINDSTRING, 0, (LPARAM)mov) == LB_ERR)
                                {
                                    SendDlgItemMessage(hwnd, IDC_NAMES, LB_ADDSTRING, 0, (LPARAM)mov);
                                }
                                mov = next + 2;
                            }
                        }

                        CloseHandle(hFile);
                    }
                }
            }
            return 0;
        case ID_FILE_SAVE:
            {
                WCHAR szPath[MAX_PATH] = { 0 };

                OPENFILENAME ofn = { sizeof(ofn) };
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0All Files\0*.*\0"; // the \0 from the double zero ending is assumed (it's the compiler's job)
                ofn.lpstrFile = szPath;
                ofn.nMaxFile = ARRAYSIZE(szPath);
                ofn.lpstrDefExt = L"txt";
                if (GetSaveFileName(&ofn))
                {
                    DWORD listCount = SendDlgItemMessage(hwnd, IDC_NAMES, LB_GETCOUNT, 0, 0);

                    DWORD dwLength = 0;

                    for (DWORD i = 0; i < listCount; i++)
                    {
                        dwLength += SendDlgItemMessage(hwnd, IDC_NAMES, LB_GETTEXTLEN, i, 0) + 2; // length + CR LF
                    }

                    dwLength++; // BOM

                    CHAR* buf = GlobalAlloc(GPTR, (dwLength + 1) * sizeof(WCHAR)); // length + NUL
                    if (!buf)
                    {
                        MessageBox(hwnd, L"Could not allocate memory.", L"Error!", MB_ICONERROR | MB_OK);
                        return 0;
                    }

                    buf[0] = 0xFF;
                    buf[1] = 0xFE;

                    WCHAR* mov = ((WCHAR*)buf) + 1;

                    for (DWORD i = 0; i < listCount; i++)
                    {
                        DWORD dwTextLength = SendDlgItemMessage(hwnd, IDC_NAMES, LB_GETTEXTLEN, i, 0);
                        SendDlgItemMessage(hwnd, IDC_NAMES, LB_GETTEXT, i, mov);
                        mov += dwTextLength;
                        *mov++ = L'\r';
                        *mov++ = L'\n';
                    }

                    HFILE hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile == INVALID_HANDLE_VALUE)
                    {
                        MessageBox(hwnd, L"Could not open file.", L"Error!", MB_ICONERROR | MB_OK);
                    }
                    else
                    {
                        DWORD dwBytesWritten = 0;
                        DWORD dwBytesToWrite = dwLength * sizeof(WCHAR);
                        while (dwBytesWritten < dwBytesToWrite)
                        {
                            if (!WriteFile(hFile, buf + dwBytesWritten, dwBytesToWrite - dwBytesWritten, &dwBytesWritten, NULL))
                            {
                                MessageBox(hwnd, L"Could not write to file.", L"Error!", MB_ICONERROR | MB_OK);
                                break;
                            }
                        }
                        CloseHandle(hFile);
                    }

                    GlobalFree(buf);
                }
            }
            return 0;
        case ID_FILE_EXIT:
            DestroyWindow(hwnd);
            return 0;
        case ID_EDIT_UNDO:
            {
                wchar_t* buf;
                OPERATION op;
                StackPop(&g_lpsUndo, &buf, &op);
                if (g_lpsUndo == NULL)
                {
                    EnableMenuItem(GetMenu(hwnd), ID_EDIT_UNDO, MF_GRAYED);
                }

                if (op == OP_ADD)
                {
                    DWORD count = SendDlgItemMessage(hwnd, IDC_NAMES, LB_GETCOUNT, 0, 0);
                    SendDlgItemMessage(hwnd, IDC_NAMES, LB_DELETESTRING, count - 1, (LPARAM)buf);
                }

                StackPush(&g_lpsRedo, buf, OP_ADD);
                EnableMenuItem(GetMenu(hwnd), ID_EDIT_REDO, MF_ENABLED);
            }
            return 0;
        case ID_EDIT_REDO:
            {
                wchar_t* buf;
                OPERATION op;
                StackPop(&g_lpsRedo, &buf, &op);
                if (g_lpsRedo == NULL)
                {
                    EnableMenuItem(GetMenu(hwnd), ID_EDIT_REDO, MF_GRAYED);
                }
                
                if (op == OP_ADD)
                {
                    SendDlgItemMessage(hwnd, IDC_NAMES, LB_ADDSTRING, 0, (LPARAM)buf);
                }

                StackPush(&g_lpsUndo, buf, OP_ADD);
                EnableMenuItem(GetMenu(hwnd), ID_EDIT_UNDO, MF_ENABLED);
            }
            return 0;
        case ID_HELP_ABOUT:
            MessageBox(hwnd, L"Thank you for using our software!", L"About", MB_OK);
            return 0;
        case IDC_ADD:
            {
                int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_NAME));
                if (len > 0)
                {
                    wchar_t* buf;

                    buf = (wchar_t*)GlobalAlloc(GPTR, (len + 1) * sizeof(wchar_t));
                    if (!buf)
                    {
                        MessageBox(hwnd, L"Could not allocate memory.", L"Error!", MB_ICONERROR | MB_OK);
                        return 0;
                    }

                    GetDlgItemText(hwnd, IDC_NAME, buf, len + 1);

                    if (!IsWhitespace(buf) && SendDlgItemMessage(hwnd, IDC_NAMES, LB_FINDSTRING, 0, (LPARAM)buf) == LB_ERR)
                    {
                        SendDlgItemMessage(hwnd, IDC_NAMES, LB_ADDSTRING, 0, (LPARAM)buf);
                        SetDlgItemText(hwnd, IDC_NAME, L"");
                    }

                    StackPush(&g_lpsUndo, buf, OP_ADD);
                    EnableMenuItem(GetMenu(hwnd), ID_EDIT_UNDO, MF_ENABLED);
                    StackClear(&g_lpsRedo);
                    EnableMenuItem(GetMenu(hwnd), ID_EDIT_REDO, MF_GRAYED);
                }
            }
            break;
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            if (GetDlgCtrlID((HWND)lParam) == IDC_TEXT)
            {
                return (INT_PTR)hbrBkgnd;
            }
        }
        break;
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 260;
            lpMMI->ptMinTrackSize.y = 153;
        }
        return 0;
    case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            // dynamic resizing
            RepositionControl(GetDlgItem(hwnd, IDC_TEXT), 0 + 10, 10, width / 2 - 20, 20);
            RepositionControl(GetDlgItem(hwnd, IDC_NAMES), 0 + 10, 35, width / 2 - 20, height - 45);
            RepositionControl(GetDlgItem(hwnd, IDC_NAME), width / 2 + 10, 35, width / 2 - 20, 20);
            RepositionControl(GetDlgItem(hwnd, IDC_ADD), width / 2 + 10, 60, width / 2 - 20, 20);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    hInst = hInstance;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Failed to register the window!", L"Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"NamesWin32",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 340,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Failed to create the window!", L"Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet < 0)
        {
            MessageBox(NULL, L"Failed to receive a message!", L"Error!", MB_ICONERROR | MB_OK);
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}
