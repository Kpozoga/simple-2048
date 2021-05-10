// PwSG1-2019.cpp : Definiuje punkt wejścia dla aplikacji.
//

#include "pch.h"
#include "framework.h"
#include "PwSG1-2019.h"
#include "dwmapi.h"
#include <time.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <string>
using namespace std;

#pragma comment(lib,"Dwmapi.lib")
#pragma comment(lib,"Msimg32.lib") 
#define MAX_LOADSTRING 100

// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego
HWND m_hWnd[2];
int gameState = 0;
struct Score { int val = 0; int maxTile = 0; int goal = 8; };
struct Tile { int val=0; bool can_fuse = true; };
Score score;
Tile tiles[16];
static HDC offDC = NULL;
 static HBITMAP offBitmap = NULL;

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterClassTile(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool moveTiles(int);
void upadateScore(int v);
bool spawnTile();
void checkGoal();
void checkMoves();
int ij2n(int i, int j);
void SaveToFile();
bool ReadFromFile();

void GetTextInfoForKeyMsg(WPARAM wParam, const TCHAR* msgName,
     TCHAR * buf, int bufSize)
     {
      _stprintf_s(buf, bufSize, _T("%s key : %d "), msgName,
        wParam);
     }

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: W tym miejscu umieść kod.

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PWSG12019, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterClassTile(hInstance);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PWSG12019));

    MSG msg;

    // Główna pętla komunikatów:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PWSG12019));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)CreateSolidBrush(RGB(250, 247, 238));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PWSG12019);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
ATOM MyRegisterClassTile(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc =nullptr;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PWSG12019));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 255, 0)); //(HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PWSG12019);
    wcex.lpszClassName = L"Tile";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej
   RECT crc = { 0,0,290,360 };
   AdjustWindowRect(&crc, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, true);
   m_hWnd[0] = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, crc.right-crc.left, crc.bottom-crc.top,nullptr,nullptr, hInstance, nullptr);
   m_hWnd[1]= CreateWindowW(szWindowClass, szTitle, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE ,
       1000, 400, crc.right - crc.left, crc.bottom - crc.top, m_hWnd[0], nullptr, hInstance, nullptr);

 /*for(int i=0;i<5;i++)
     CreateWindowW(L"Tile", nullptr, WS_CHILD | WS_VISIBLE,
       10, (i + 1) * 10 + i * 60, 60, 60, m_hWnd[0], nullptr, hInstance, nullptr);*/

   if (!m_hWnd[0]||!m_hWnd[1])
   {
      return FALSE;
   }
   
   ShowWindow(m_hWnd[0], nCmdShow);
   UpdateWindow(m_hWnd[0]);

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const int bufSize = 256;
    TCHAR buf[bufSize];
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_GAME_NEWGAME:
            {
                for (int i = 0; i < 16; i++)
                {
                    tiles[i].val = 0;
                }
                score.val = 0;
                score.maxTile = 0;
                gameState = 0;
                tiles[rand() % 16].val = 2;
                InvalidateRect(m_hWnd[0], NULL, TRUE);
                InvalidateRect(m_hWnd[1], NULL, TRUE);
                UpdateWindow(m_hWnd[0]);
                UpdateWindow(m_hWnd[1]);
            }
                break;
            case ID_GOAL_8:
            {
                HMENU menu = GetMenu(hWnd);
                HMENU menu2 = GetMenu(m_hWnd[hWnd == m_hWnd[0]]);
                CheckMenuItem(menu, ID_GOAL_8, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_8, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_64, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_64, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                score.goal = 8;
                checkGoal();
                InvalidateRect(m_hWnd[0], NULL, TRUE);
                InvalidateRect(m_hWnd[1], NULL, TRUE);
                UpdateWindow(m_hWnd[0]);
                UpdateWindow(m_hWnd[1]);
            }
                break;
            case ID_GOAL_16:
            {
                HMENU menu = GetMenu(hWnd);
                HMENU menu2 = GetMenu(m_hWnd[hWnd == m_hWnd[0]]);
                CheckMenuItem(menu, ID_GOAL_16, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_16, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_8 , MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_64 , MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu,  ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_8 , MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2,  ID_GOAL_64 , MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2,  ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                score.goal = 16;  
                checkGoal();
                InvalidateRect(m_hWnd[0], NULL, TRUE);
                InvalidateRect(m_hWnd[1], NULL, TRUE);
                UpdateWindow(m_hWnd[0]);
                UpdateWindow(m_hWnd[1]);
            }
                break;
            case ID_GOAL_64:
            {
                HMENU menu = GetMenu(hWnd);
                HMENU menu2 = GetMenu(m_hWnd[hWnd == m_hWnd[0]]);
                CheckMenuItem(menu, ID_GOAL_64, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_64, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_8, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_8, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_2048, MF_UNCHECKED | MF_BYCOMMAND);
                score.goal = 64;
                checkGoal();
                InvalidateRect(m_hWnd[0], NULL, TRUE);
                InvalidateRect(m_hWnd[1], NULL, TRUE);
                UpdateWindow(m_hWnd[0]);
                UpdateWindow(m_hWnd[1]);
            }
                break;
            case ID_GOAL_2048:
            {
                HMENU menu = GetMenu(hWnd);
                HMENU menu2 = GetMenu(m_hWnd[hWnd == m_hWnd[0]]);
                CheckMenuItem(menu, ID_GOAL_2048, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_2048, MF_CHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_8, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_64, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_8, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_64, MF_UNCHECKED | MF_BYCOMMAND);
                CheckMenuItem(menu2, ID_GOAL_16, MF_UNCHECKED | MF_BYCOMMAND);
                score.goal = 2048;
                checkGoal();
                InvalidateRect(m_hWnd[0], NULL, TRUE);
                InvalidateRect(m_hWnd[1], NULL, TRUE);
                UpdateWindow(m_hWnd[0]);
                UpdateWindow(m_hWnd[1]);
            }
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
       

        }
        break;
    /*case WM_LBUTTONDOWN:
         {
            HDC hdc = GetDC(hWnd);
             HBRUSH brush = CreateSolidBrush(RGB(128, 128, 0));
             HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
             short x = (short)LOWORD(lParam);
             short y = (short)HIWORD(lParam);
             const int rad = 5;
             Ellipse(hdc, x - rad, y - rad, x + rad, y + rad);
             SelectObject(hdc, oldBrush);
             DeleteObject(brush);
             ReleaseDC(hWnd, hdc);
             czypaint = true;
             InvalidateRect(hWnd, NULL, TRUE);
             }
         break;*/
    /*case WM_KEYDOWN:
    {
        GetTextInfoForKeyMsg(wParam, _T(" KEYDOWN "), buf, bufSize);
         if(wParam==84)SetWindowText(hWnd, buf);
    }
        break;*/
    case WM_CHAR:
    {
        if (gameState != 0) break;
        bool moved=false;
        switch ((TCHAR)wParam)
        {
        case 'W':
        case 'w':
            moved=moveTiles(0);
        break;
        case 'A':
        case 'a':
            moved=moveTiles(1);
        break;
        case 'S':
        case 's':
            moved=moveTiles(2);
        break;
        case 'D':
        case 'd':
            moved=moveTiles(3);
        break;
        }
        
        if(!moved){}
        else if (moved)
        {
            checkGoal();
            if (!spawnTile())
                checkMoves();

                
        }
        InvalidateRect(m_hWnd[0], NULL, TRUE);
        InvalidateRect(m_hWnd[1], NULL, TRUE);
        UpdateWindow(m_hWnd[0]);
        UpdateWindow(m_hWnd[1]);
    }
         break;
    case WM_MOVING:
    {      
        RECT rc;
        GetWindowRect(hWnd, &rc);       
        //DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS , &rc, sizeof(RECT));
        int w = (rc.right - rc.left);
        int h = (rc.bottom - rc.top);
        int x = rc.right - w / 2;
        int y = rc.bottom - h / 2;
        RECT rcS;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcS, 0);
        int xc = (rcS.right - rcS.left) / 2;
        int yc = (rcS.bottom - rcS.top) / 2;

        MoveWindow(m_hWnd[hWnd==m_hWnd[0]], xc + xc - x - w / 2, yc + yc - y - h / 2, w, h, TRUE);
        if (xc - rc.right <= 0 && xc - rc.left >= 0 && yc - rc.bottom <= 0 && yc - rc.top >= 0)
        {
            SetWindowLong(m_hWnd[1], GWL_EXSTYLE, GetWindowLong(m_hWnd[1], GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(m_hWnd[1], 0, (255 * 50) / 100, LWA_ALPHA);
        }
        else
            SetLayeredWindowAttributes(m_hWnd[1], 0, (255 * 100) / 100, LWA_ALPHA);

    }break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(250, 247, 238));
             HPEN oldPen = (HPEN)SelectObject(offDC, pen);             
             HBRUSH oldBrush = (HBRUSH)SelectObject(offDC, CreateSolidBrush(RGB(250, 247, 238)));
             Rectangle(offDC, 0, 0, 290, 360);
             HBRUSH brush = CreateSolidBrush(RGB(204, 192, 174));
             SelectObject(offDC, brush);
             RoundRect(offDC, 10, 10, 280, 70, 20, 20);
             RECT rc = { 10, 10, 280, 70 };
             HFONT font = CreateFont(
                  - MulDiv(20, GetDeviceCaps(offDC, LOGPIXELSY), 72), // Height
                  0, // Width
                  0, // Escapement
                  0, // Orientation
                  FW_BOLD, // Weight
                  false, // Italic
                  FALSE, // Underline
                  0, // StrikeOut
                  EASTEUROPE_CHARSET, // CharSet
                  OUT_DEFAULT_PRECIS, // OutPrecision
                  CLIP_DEFAULT_PRECIS, // ClipPrecision
                  DEFAULT_QUALITY, // Quality
                  DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
                  _T(" Verdana ")); // Facename
              HFONT oldFont = (HFONT)SelectObject(offDC, font);
              _stprintf_s(buf, bufSize, _T("%d"),score.val);
             DrawText(offDC, buf/*&itoa(score.val) */, -1, &rc, DT_CENTER| DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);
             
             for(int i=0;i<4;i++)
                 for (int j = 0; j < 4; j++)
                 {
                     switch (tiles[ij2n(i, j)].val)
                     {
                     case 0: 
                            SelectObject(offDC, CreateSolidBrush(RGB(204, 192, 174)));    
                       break;
                     case 2: SelectObject(offDC, CreateSolidBrush(RGB(238, 228, 198)));
                         break;
                     case 4: SelectObject(offDC, CreateSolidBrush(RGB(239, 225, 218)));
                         break;
                     case 8: SelectObject(offDC, CreateSolidBrush(RGB(243, 179, 124)));
                         break;
                     case 16: SelectObject(offDC, CreateSolidBrush(RGB(246, 153, 100)));
                         break;
                     case 32: SelectObject(offDC, CreateSolidBrush(RGB(246, 125, 98)));
                         break;
                     case 64: SelectObject(offDC, CreateSolidBrush(RGB(247, 93, 60)));
                         break;
                     case 128: SelectObject(offDC, CreateSolidBrush(RGB(237, 206, 116)));
                         break;
                     case 256: SelectObject(offDC, CreateSolidBrush(RGB(239, 204, 98)));
                         break;
                     case 512: SelectObject(offDC, CreateSolidBrush(RGB(243, 201, 85)));
                         break;
                     case 1024: SelectObject(offDC, CreateSolidBrush(RGB(238, 200, 72)));
                         break;
                     case 2048: SelectObject(offDC, CreateSolidBrush(RGB(239, 192, 47)));
                         break;
                     default:
                         throw EXCEPTION_BREAKPOINT;
                     }
                     RoundRect(offDC, 70*j+10, 70 * i + 80, 70+j*70, 70*i+140, 20, 20);
                     if(tiles[ij2n(i,j)].val!=0)
                     {_stprintf_s(buf, bufSize, _T("%d"), tiles[ij2n(i, j)].val);
                     rc = { 70 * j + 10, 70 * i + 80, 70 + j * 70, 70 * i + 140 };
                     DrawText(offDC, buf, -1, &rc, DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE); }
                 }
             if (gameState != 0)
             {
                
                 HDC memDC = CreateCompatibleDC(offDC);
                 HBITMAP bitmap = CreateCompatibleBitmap(offDC, 290, 360); //LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1));
                 HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
                 SelectObject(memDC, CreateSolidBrush(RGB(0,255,0)));
                 Rectangle(memDC, 0, 0,290 ,360);
                 BLENDFUNCTION blend = { AC_SRC_OVER,0,255,AC_SRC_ALPHA };
                  AlphaBlend(offDC, 0,0, 290,360, memDC,0, 0,48,48, blend);

                  /*SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
                  SendMessage(m_hWnd[hWnd == m_hWnd[0]], WM_SETREDRAW, FALSE, 0);*/

                /* HWND hwtmp=CreateWindowW(L"Tile", nullptr, WS_CHILD | WS_VISIBLE,
                     0, 0, 390, 360, m_hWnd[0], nullptr, hInst, nullptr); 
                 SetWindowLong(hwtmp, GWL_EXSTYLE,
                      GetWindowLong(hwtmp, GWL_EXSTYLE) | WS_EX_LAYERED);
                        SetLayeredWindowAttributes(hwtmp, 0, (255 * 50) / 100, LWA_ALPHA);*/
                  //LockWindowUpdate(hWnd);
                  //EnableWindow(m_hWnd[hWnd == m_hWnd[0]], false );
             }
             BitBlt(hdc, 0, 0, rc.right, rc.bottom, offDC, 0, 0, SRCCOPY);
             SelectObject(offDC, oldPen);
             DeleteObject(pen);
             SelectObject(offDC, oldBrush);
             DeleteObject(brush);
             SelectObject(offDC, oldFont);
             DeleteObject(font);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_ERASEBKGND:
             return 1;
    case WM_CREATE:
    {
        if (!ReadFromFile())
        {
            srand(time(NULL));
            tiles[rand() % 16].val = 2;
        }

        HDC hdc = GetDC(hWnd);
        offDC = CreateCompatibleDC(hdc);
        offBitmap = CreateCompatibleBitmap(hdc, 290, 360);
        SelectObject(offDC, offBitmap);
        ReleaseDC(hWnd, hdc);     
        HMENU menu = GetMenu(hWnd);
        HMENU menu2 = GetMenu(m_hWnd[hWnd == m_hWnd[0]]);
        switch (score.goal)
        {
        case 8:
        {
            CheckMenuItem(menu, ID_GOAL_8, MF_CHECKED | MF_BYCOMMAND);
            CheckMenuItem(menu2, ID_GOAL_8, MF_CHECKED | MF_BYCOMMAND);
        }
            break;
        case 16:
        {
            CheckMenuItem(menu, ID_GOAL_16, MF_CHECKED | MF_BYCOMMAND);
            CheckMenuItem(menu2, ID_GOAL_16, MF_CHECKED | MF_BYCOMMAND);
        }
        break;
        case 64:
        {
            CheckMenuItem(menu, ID_GOAL_64, MF_CHECKED | MF_BYCOMMAND);
            CheckMenuItem(menu2, ID_GOAL_64, MF_CHECKED | MF_BYCOMMAND);
        }
        break;
        case 2048:
        {
            CheckMenuItem(menu, ID_GOAL_2048, MF_CHECKED | MF_BYCOMMAND);
            CheckMenuItem(menu2, ID_GOAL_2048, MF_CHECKED | MF_BYCOMMAND);
        }
        break;
        }
    }

        break;
    case WM_DESTROY:
    {
        SaveToFile();
        if (offDC != NULL) 
             DeleteDC(offDC);
        if (offBitmap != NULL) 
            DeleteObject(offBitmap);
        PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool moveTiles(int k)
{
    bool moved = false;
    int lastfree;
    for (int i = 0; i < 4; i++)
    {
        switch (k)
        {
        case 3:
        {
            lastfree = 3;
            for (int j = 3; j >= 0; j--)
            {
                if (tiles[ij2n(i, j)].val == 0) continue;
                if (j != lastfree)
                {
                    tiles[ij2n(i, lastfree)].val = tiles[ij2n(i, j)].val;
                    tiles[ij2n(i, j)].val = 0;
                    moved = true;
                }
                if (lastfree + 1 <= 3)
                    if (tiles[ij2n(i, lastfree + 1)].can_fuse && tiles[ij2n(i, lastfree + 1)].val == tiles[ij2n(i, lastfree)].val)
                    {
                        tiles[ij2n(i, lastfree + 1)].val *= 2;
                        tiles[ij2n(i, lastfree + 1)].can_fuse = false;
                        upadateScore(tiles[ij2n(i,lastfree + 1)].val);
                        tiles[ij2n(i, lastfree)].val = 0;
                        lastfree++;
                        moved = true;
                    }
                lastfree--;
            }
        }
        break;
        case 1:
        {
            lastfree = 0;
            for (int j = 0; j <4; j++)
            {
                if (tiles[ij2n(i, j)].val == 0) continue;
                if (j != lastfree)
                {
                    tiles[ij2n(i, lastfree)].val = tiles[ij2n(i, j)].val;
                    tiles[ij2n(i, j)].val = 0;
                    moved = true;
                }
                if (lastfree - 1 >=0)
                    if (tiles[ij2n(i, lastfree - 1)].can_fuse && tiles[ij2n(i, lastfree - 1)].val == tiles[ij2n(i, lastfree)].val)
                    {
                        tiles[ij2n(i, lastfree - 1)].val *= 2;
                        tiles[ij2n(i, lastfree - 1)].can_fuse = false;
                        upadateScore(tiles[ij2n(i,lastfree - 1)].val);
                        tiles[ij2n(i, lastfree)].val = 0;
                        lastfree--;
                        moved = true;
                    }
                lastfree++;
            }
        }
        break;
        case 2:
        {
            lastfree = 3;
            for (int j = 3; j >= 0; j--)
            {
                if (tiles[ij2n(j, i)].val == 0) continue;
                if (j != lastfree)
                {
                    tiles[ij2n(lastfree,i)].val = tiles[ij2n(j,i)].val;
                    tiles[ij2n(j, i)].val = 0;
                    moved = true;
                }
                if (lastfree + 1 <= 3)
                    if (tiles[ij2n(lastfree + 1,i)].can_fuse && tiles[ij2n( lastfree + 1,i)].val == tiles[ij2n(lastfree,i)].val)
                    {
                        tiles[ij2n(lastfree + 1,i)].val *= 2;
                        tiles[ij2n(lastfree + 1,i)].can_fuse = false;
                        upadateScore(tiles[ij2n(lastfree + 1, i)].val);
                        tiles[ij2n( lastfree,i)].val = 0;
                        lastfree++;
                        moved = true;
                    }
                lastfree--;
            }
        }
        break;
        case 0:
        {
            lastfree = 0;
            for (int j = 0; j <4; j++)
            {
                if (tiles[ij2n(j, i)].val == 0) continue;
                if (j != lastfree)
                {
                    tiles[ij2n(lastfree, i)].val = tiles[ij2n(j, i)].val;
                    tiles[ij2n(j, i)].val = 0;
                    moved = true;
                }
                if (lastfree -1>=0)
                    if (tiles[ij2n(lastfree -1, i)].can_fuse && tiles[ij2n(lastfree - 1, i)].val == tiles[ij2n(lastfree, i)].val)
                    {
                        tiles[ij2n(lastfree - 1, i)].val *= 2;
                        tiles[ij2n(lastfree - 1, i)].can_fuse = false;
                        upadateScore(tiles[ij2n(lastfree - 1, i)].val);
                        tiles[ij2n(lastfree, i)].val = 0;
                        lastfree--;
                        moved = true;
                    }
                lastfree++;
            }
        }
        break;
        }
    }
    for (int i = 0; i < 16; i++)
        tiles[i].can_fuse = true;
    
    return moved;
}

void upadateScore(int v)
{
    score.val += v;
    if (v > score.maxTile)score.maxTile = v;
}

void checkGoal()
{
    if (score.maxTile >= score.goal) gameState = 1;
}

int ij2n(int i, int j) 
{
    return i * 4 + j; 
}

bool spawnTile()
{
    int freeTiles = 0;
    for (int i = 0; i < 16; i++)
        if (tiles[i].val == 0) freeTiles++;
    if (freeTiles == 0) return false;
    int tile = rand() % freeTiles;
    int k = 0;
    for (int i = 0; i < 16; i++)       
        if (tiles[i].val == 0)
        {
            if (k == tile)
            {
                tiles[i].val = 2;
                break;
            }
            else
                k++;
        }
    if (freeTiles > 1) return true;
    return false;
}

void checkMoves()
{
    for(int i=0;i<4;i++)
        for (int j = 0; j < 4; j++)
        {
            if (j > 0)
                if (tiles[ij2n(i, j)].val == tiles[ij2n(i, j - 1)].val)
                    return;
            if (i > 0)
                if (tiles[ij2n(i, j)].val == tiles[ij2n(i-1, j)].val)
                    return;
        }
    gameState = 2;
}

void SaveToFile()
{
    ofstream myfile("2048.txt", ios::trunc);
    myfile << gameState << endl;
    myfile << score.val << endl;
    myfile << score.goal << endl;
    for (int i = 0; i < 16;i++) {
        myfile << tiles[i].val << endl;
    }
    myfile.close();
}

bool ReadFromFile() {
    ifstream myfile("2048.txt", ios::in);
    if (!myfile) return false;
    string str;
    int i = 0;
    getline(myfile, str); gameState= stoi(str);
    getline(myfile, str); score.val = stoi(str);
    getline(myfile, str); score.goal = stoi(str);
    while (getline(myfile,str))
    {
        tiles[i].val = stoi(str);
        i++;
    }
    myfile.close();
    return true;
}