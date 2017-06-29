#include <windows.h>
#include <stdio.h>

#define BORDER_WIDTH        1

typedef struct instance_desc
{

} instance_t;

inline POINT CPOINT(LONG x, LONG y)
{
    POINT p;

    p.x = x; p.y = y;

    return p;
};

static LRESULT CALLBACK WndProcWrap(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    fprintf(stderr, "%s:%d:\n", __FUNCTION__, __LINE__);

    switch (msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            {
                HDC hdc;
                PAINTSTRUCT ps;
                RECT rectCapture;

                hdc = BeginPaint(hwnd, &ps);

                //create a new red paint brush
                HPEN hPen = CreatePen(PS_INSIDEFRAME | PS_DASH, BORDER_WIDTH, RGB(255, 0, 0));

                SetROP2(hdc, R2_COPYPEN);
                HPEN hpenOld = (HPEN)SelectObject(hdc, hPen);

                //get the rectangle of the window relating to the current mouse cursor
                ::GetWindowRect(hwnd, &rectCapture);
                long W = rectCapture.right - rectCapture.left, H = rectCapture.bottom - rectCapture.top;

                //draw a red rectangle around the window relating to the current mouse cursor to friendly prompt the user
                POINT pt[5];
                pt[0] = CPOINT(BORDER_WIDTH, BORDER_WIDTH);
                pt[1] = CPOINT(W - BORDER_WIDTH, BORDER_WIDTH);
                pt[2] = CPOINT(W - BORDER_WIDTH, H - BORDER_WIDTH);
                pt[3] = CPOINT(BORDER_WIDTH, H - BORDER_WIDTH);
                pt[4] = pt[0];
                ::Polyline(hdc, pt, 5);
#if 0
                //first sleep 100 milliseconds and then redraw the red rectangle so as not to destroy the original content
                Sleep(100);
                ::Polyline(hdc, pt, 5);
#endif
                //seemly verbose while effective
                ::SelectObject(hdc, hpenOld);
                ::ReleaseDC(NULL, hdc);

                EndPaint(hwnd, &ps);
            }
            break;

        case WM_ERASEBKGND:
            return 0;

        case WM_SIZE:
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static const char* vga_window_name = "map2vga";

static HWND create_window(instance_t* instance, HINSTANCE hInstance, HWND hParent)
{
    // Register the windows class
    WNDCLASSEX wndClass;
    HWND hWnd;

    if (!GetClassInfoEx(hInstance, vga_window_name, &wndClass))
    {
        wndClass.cbSize = sizeof(wndClass);
        wndClass.style = 0; // CS_HREDRAW | CS_VREDRAW; // CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wndClass.lpfnWndProc = WndProcWrap;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = NULL;
        wndClass.hCursor = NULL;
        wndClass.hbrBackground = NULL; // (HBRUSH)(COLOR_WINDOW + 1); // NULL
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = vga_window_name;
        wndClass.hIconSm = NULL;

        if (!RegisterClassEx(&wndClass))
        {
            int r = GetLastError();
            fprintf(stderr, "%s: RegisterClassEx failed: GetLastError=%d", __FUNCTION__, r);
            return NULL;
        };
    };

    // Create the render window ; https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms632680(v=vs.85).aspx
    hWnd = CreateWindowEx
    (
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        vga_window_name,
        vga_window_name,
        WS_POPUP | WS_VISIBLE, // | WS_CAPTION | WS_BORDER,
        100 /*CW_USEDEFAULT*/,
        100 /*CW_USEDEFAULT*/,
        200 /*CW_USEDEFAULT*/,
        200 /*CW_USEDEFAULT*/,
        hParent,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        int r = GetLastError();
        fprintf(stderr, "%s: CreateWindowEx failed: GetLastError=%d", __FUNCTION__, r);
    };

    SetLayeredWindowAttributes(hWnd, 0, 100, /*LWA_ALPHA*/ LWA_COLORKEY);

    return hWnd;
};


int main(int argc, char** argv)
{
    HRESULT hr;
    HWND hWnd;
    MSG msg;
    //	HACCEL hAccelTable;
    //	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLY_SPLIT));

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    hWnd = create_window(NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        fprintf(stderr, "%s:%d\n", __FUNCTION__, __LINE__);
        if (!TranslateAccelerator(hWnd, NULL /*hAccelTable*/, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    fprintf(stdout, "%s: press <enter> to continue...\n", __FUNCTION__);
    getc(stdin);
    fprintf(stdout, "%s: exiting...", __FUNCTION__);

    return 0;
}