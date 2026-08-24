#include "Gui.hpp"

#include <SDL.h>
#include <GL/gl.h>
#include <d3d8.h>
#include <stdio.h>
#include <string.h>

// WebGL has no attribute stack. These probe-only definitions let us determine
// whether the remaining fixed-function compatibility path can create a device
// and present a frame. A production Web backend must restore explicit state.
extern "C" void glPushAttrib(GLbitfield) {}
extern "C" void glPopAttrib() {}

// Surface DC access is not exercised by this renderer-only probe.
extern "C" HDC CreateCompatibleDC(HDC) { return reinterpret_cast<HDC>(1); }
extern "C" BOOL DeleteDC(HDC) { return TRUE; }

namespace th08
{
// The production renderer queries this only to preserve dialogue snapshots.
// The renderer probe has no game state, so its isolated Gui is never active.
Gui g_Gui;
i32 Gui::IsDialogPresent() { return 0; }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_Window *window = SDL_CreateWindow(
        "TH08 Web renderer probe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    IDirect3D8 *d3d = Direct3DCreate8(D3D_SDK_VERSION);
    D3DPRESENT_PARAMETERS parameters;
    memset(&parameters, 0, sizeof(parameters));
    parameters.BackBufferWidth = 640;
    parameters.BackBufferHeight = 480;
    parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    parameters.BackBufferCount = 1;
    parameters.SwapEffect = D3DSWAPEFFECT_COPY;
    parameters.hDeviceWindow = reinterpret_cast<HWND>(window);
    parameters.Windowed = TRUE;
    parameters.EnableAutoDepthStencil = TRUE;
    parameters.AutoDepthStencilFormat = D3DFMT_D16;
    parameters.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    IDirect3DDevice8 *device = NULL;
    if (d3d == NULL || d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                                         parameters.hDeviceWindow,
                                         D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                         &parameters, &device) != D3D_OK)
    {
        fprintf(stderr, "Direct3D8 compatibility device creation failed\n");
        return 1;
    }

    device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff271c47, 1.0f, 0);
    device->BeginScene();
    device->EndScene();
    device->Present(NULL, NULL, NULL, NULL);
    printf("renderer-probe=ok\n");
    return 0;
}
