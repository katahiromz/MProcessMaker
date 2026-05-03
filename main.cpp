// main.cpp --- MProcessMaker のサンプルプログラム
// Author: katahiromz
// License: MIT
#include "MProcessMaker.hpp"
#include <windowsx.h>
#include <commctrl.h>

// グローバル変数でプロセスとリダイレクト用ファイルを保持
MProcessMaker g_maker;
MFile g_hInputWrite;
MFile g_hOutputRead;

// タイマーID
#define IDT_TIMER_OUTPUT 100

// タイマー間隔（ミリ秒）
#define TIMER_INTERVAL 100

// エディットボックス（edt1）に出力を追加する関数
void AppendTextA(HWND hwndStatic, LPCSTR pszText)
{
    INT len = GetWindowTextLengthA(hwndStatic);
    SendMessageA(hwndStatic, EM_SETSEL, len, len);
    SendMessageA(hwndStatic, EM_REPLACESEL, 0, (LPARAM)pszText);
}

// WM_INITDIALOG
BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    // 入出力をリダイレクトするための準備
    if (g_maker.PrepareForRedirect(&g_hInputWrite, &g_hOutputRead))
    {
        g_maker.SetShowWindow(SW_HIDE); // cmd.exeの黒い画面を出さない
        if (g_maker.CreateProcessDx(NULL, TEXT("cmd.exe")))
        {
            // 出力を監視するためのタイマーを開始
            SetTimer(hwnd, IDT_TIMER_OUTPUT, TIMER_INTERVAL, NULL);
            return TRUE;
        }
    }

    MessageBox(hwnd, TEXT("cmd.exe の起動に失敗しました。"), NULL, MB_ICONERROR);
    return TRUE;
}

// WM_COMMAND
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
        KillTimer(hwnd, IDT_TIMER_OUTPUT);
        EndDialog(hwnd, id);
        break;
    case psh1: // Enterボタンが押された時
        {
            CHAR szInput[256];
            GetDlgItemTextA(hwnd, edt2, szInput, _countof(szInput)); // edt2からコマンド取得

            MStringA strInput = szInput;
            strInput += "\r\n"; // 改行を付加して実行

            DWORD cbWritten;
            g_hInputWrite.WriteFile(strInput.c_str(), (DWORD)strInput.length(), &cbWritten);

            // 入力欄をクリア
            SetDlgItemText(hwnd, edt2, _T(""));
        }
        break;
    }
}

// WM_TIMER
void OnTimer(HWND hwnd, UINT id)
{
    if (id != IDT_TIMER_OUTPUT)
        return;

    DWORD cbAvail = 0;
    // 読み取れるデータがあるか確認
    if (g_hOutputRead.PeekNamedPipe(NULL, 0, NULL, &cbAvail) && cbAvail > 0)
    {
        std::string strOutput;
        char szBuf[1024];
        DWORD cbRead;

        // 利用可能なデータを読み取る
        if (g_hOutputRead.ReadFile(szBuf, sizeof(szBuf) - 1, &cbRead) && cbRead > 0)
        {
            szBuf[cbRead] = '\0';
            // cmd.exeの出力(ANSI/OEM)をTCHARに簡易変換して表示
            AppendTextA(GetDlgItem(hwnd, edt1), szBuf);
        }
    }

    // プロセスが終了していたらタイマーを止める
    if (!g_maker.IsRunning() && cbAvail == 0)
    {
        KillTimer(hwnd, IDT_TIMER_OUTPUT);
        AppendTextA(GetDlgItem(hwnd, edt1), "\r\n[Process Terminated]\r\n");
    }
}

// WM_CTLCOLORSTATIC
HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    // 背景を黒、文字色を白にする
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkColor(hdc, RGB(0, 0, 0));
    return GetStockBrush(BLACK_BRUSH);
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC, OnCtlColor);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    InitCommonControls();
    DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
    return 0;
}
