
/**
 *  \file   win_bkend.c
 *  \brief  WorldSens Graphical Windows UI definition 
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arch/common/hardware.h"
#include "devices/devices.h"
#include "src/mgetopt.h"
#include "src/options.h"

#include "ui.h"
#include "ui_bkend.h"

/**************************************************/
/**************************************************/
/**************************************************/

#define UNUSED __attribute__((unused))  

#include <windows.h>

struct win_display_t {
  HINSTANCE   handle;
  HWND        hWnd;
  HBITMAP     hbitmap;
  HDC         MemDC;
  int         width,height,depth;
};

static struct win_display_t win_display;

/**************************************************/
/**************************************************/
/**************************************************/

LPCTSTR ClsName = "WSim";
LPCTSTR WndName = "WSim application";


LRESULT CALLBACK WndProcedure(HWND hWnd, UINT uMsg,
			      WPARAM wParam, LPARAM lParam);

void ClientResize(HWND hWnd, int nWidth, int nHeight)
{
  RECT rcClient, rcWindow;
  POINT ptDiff;
  GetClientRect(hWnd, &rcClient);
  GetWindowRect(hWnd, &rcWindow);
  ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
  ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
  MoveWindow(hWnd,rcWindow.left, rcWindow.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}

void* ui_backend_create(int w, int h, char *title, int *mustlock)
{
  WNDCLASSEX WndClsEx;
  static struct win_display_t *win = &win_display;

  *mustlock = 0;

  win->handle  = GetModuleHandle(NULL);
  win->hWnd    = 0;
  win->hbitmap = 0;
  
  // Create the application window
  WndClsEx.cbSize        = sizeof(WNDCLASSEX);
  WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
  WndClsEx.lpfnWndProc   = WndProcedure;
  WndClsEx.cbClsExtra    = 0;
  WndClsEx.cbWndExtra    = 0;
  WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
  WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  WndClsEx.lpszMenuName  = NULL;
  WndClsEx.lpszClassName = ClsName;
  WndClsEx.hInstance     = win->handle;
  WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
  
  // Register the application
  RegisterClassEx(&WndClsEx);
  
  // Create the window object
  win->hWnd = CreateWindow(ClsName,
		      title, // WndName,
		      WS_OVERLAPPEDWINDOW,
		      CW_USEDEFAULT, // x
		      CW_USEDEFAULT, // y
		      CW_USEDEFAULT, // w
		      CW_USEDEFAULT, // h
		      NULL,
		      NULL,
		      win->handle,
		      NULL);
  
  // Find out if the window was created
  if( !win->hWnd ) // If the window was not created,
    {
      return NULL; // stop the application
    }

  ClientResize(win->hWnd, w, h);

  // Display the window to the user
  ShowWindow(win->hWnd, SW_SHOWNORMAL);
  UpdateWindow(win->hWnd);

  /* Image */
  win->width   = w;
  win->height  = h;
  win->depth   = 24;

  /* Bimap */
  {
    HDC    hDC;
    BITMAP bm;

    hDC = GetDC(win->hWnd);

    win->MemDC = CreateCompatibleDC(hDC);
    win->hbitmap = CreateCompatibleBitmap(hDC, win->width, win->height); 
    SelectObject(win->MemDC, win->hbitmap);
    GetObject(win->hbitmap,sizeof(BITMAP),(LPSTR)&bm);

    ReleaseDC(win->hWnd, hDC);

    /*
    printf("type %d w:%d h:%d wb:%d p:%d bpp:%d\n",
	   bm.bmType, bm.bmWidth, bm.bmHeight, bm.bmWidthBytes, 
	   bm.bmPlanes, bm.bmBitsPixel);
    */
  }

  return win;
}


/**************************************************/
/**************************************************/
/**************************************************/

void ui_backend_delete(void *ptr)
{
  struct win_display_t *win = (struct win_display_t*)ptr;
  if (win->hbitmap)
    {
      DeleteObject(win->hbitmap);
      win->hbitmap = 0;
    }
  if (win->MemDC)
    {
      DeleteDC(win->MemDC);
      win->MemDC = 0;
    }
  if (win->hWnd)
    {
      DestroyWindow(win->hWnd);
      win->hWnd = 0;
    }
  PostQuitMessage(WM_QUIT);
}

/**************************************************/
/**************************************************/
/**************************************************/
 
int ui_backend_framebuffer_blit(void *ptr, uint8_t *fb)
{
  int w,h;
  int idx_buff;
  struct win_display_t *win = (struct win_display_t*)ptr;

  for(h=0; h < win->height; h++)
    {
      idx_buff =  h * win->width * 3;
      for(w=0; w < win->width; w++)
	{
	  COLORREF rgbval = RGB(
			    (fb[idx_buff + 2]),
			    (fb[idx_buff + 1]),
			    (fb[idx_buff + 0])
			    );

	  SetPixel(win->MemDC, w, h, rgbval);
	  idx_buff += 3;
	}
    }
  return UI_OK;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_lock(void UNUSED *ptr)
{
  // struct win_display_t *win = (struct win_display_t*)ptr;
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_unlock(void UNUSED *ptr)
{
  // struct win_display_t *win = (struct win_display_t*)ptr;
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_update(void *ptr)
{
  struct win_display_t *win = (struct win_display_t*)ptr;

  InvalidateRect (win->hWnd,0,0);
  UpdateWindow   (win->hWnd    );
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

int ui_backend_getevent(void *ptr, uint32_t *b_up, uint32_t* b_down)
{
  int ret = UI_EVENT_NONE;
  MSG  Msg;

  struct win_display_t *win = (struct win_display_t*)ptr;

  while (PeekMessage(&Msg, win->hWnd, 0, 0, PM_REMOVE))
    {
      switch (Msg.message)
	{
	case WM_KEYDOWN:
	  {
	    ret |= UI_EVENT_USER;
	    *b_up   = 0;
	    *b_down = 0;
	    
	    switch (Msg.wParam)
	      {
	      case '1': *b_down |= UI_BUTTON_1; break;
	      case '2': *b_down |= UI_BUTTON_2; break;
	      case '3': *b_down |= UI_BUTTON_3; break;
	      case '4': *b_down |= UI_BUTTON_4; break;
	      case '5': *b_down |= UI_BUTTON_5; break;
	      case '6': *b_down |= UI_BUTTON_6; break;
	      case '7': *b_down |= UI_BUTTON_7; break;
	      case '8': *b_down |= UI_BUTTON_8; break;
	      case 'Q': ret = UI_EVENT_QUIT;    break;
	      default:
		ret = UI_EVENT_NONE;
		break;
	      }
	  }
	  return ret;

	case WM_KEYUP:
	  {
	    ret |= UI_EVENT_USER;
	    *b_up   = 0;
	    *b_down = 0;

	    switch (Msg.wParam)
	      {
	      case '1': *b_up |= UI_BUTTON_1; break;
	      case '2': *b_up |= UI_BUTTON_2; break;
	      case '3': *b_up |= UI_BUTTON_3; break;
	      case '4': *b_up |= UI_BUTTON_4; break;
	      case '5': *b_up |= UI_BUTTON_5; break;
	      case '6': *b_up |= UI_BUTTON_6; break;
	      case '7': *b_up |= UI_BUTTON_7; break;
	      case '8': *b_up |= UI_BUTTON_8; break;
	      case 'Q': ret = UI_EVENT_QUIT;  break;
	      default:
		ret = UI_EVENT_NONE;
		break;
	      }
	  }
	  return ret;

	default:
	  TranslateMessage(&Msg);
	  DispatchMessage(&Msg);
	  break;
	}
    }

  return ret;
}

/**************************************************/
/**************************************************/
/**************************************************/

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  HDC hDC;
  PAINTSTRUCT Ps;

  switch(Msg)
    {
    case WM_PAINT:
      {
	hDC = BeginPaint(hWnd, &Ps);
	BitBlt(hDC, 0, 0, win_display.width, win_display.height, 
	       win_display.MemDC, 0, 0, SRCCOPY);
	EndPaint(hWnd, &Ps);
      }
      break;

    case WM_CLOSE:
      mcu_signal_add(SIG_HOST | SIGTERM);
      break;

    case WM_DESTROY:
      mcu_signal_add(SIG_HOST | SIGTERM);
      break;

    default:
      return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/

INT WINAPI WinMain(HINSTANCE UNUSED hInstance, HINSTANCE UNUSED hPrevInstance,
               LPSTR UNUSED lpCmdLine, int UNUSED nCmdShow)
{
  /* put a warning messagebox and exit */
  
  MessageBox(NULL,
	     "Although This program has a graphical interface,\n\r"
	     "it can only be started using a console command line.",
	     "WSim: error",
	     MB_APPLMODAL | MB_OK
	     );
  
  return 0;
}

/**************************************************/
/**************************************************/
/**************************************************/
