#include "r_ui3d.h"
#include "rb_backend.h"
#include "r_dvars.h"
#include "r_foliage.h"
#include "rb_draw3d.h"
#include "r_state_utils.h"
#include "r_state.h"
#include "rb_imagefilter.h"

GfxUI3DStack g_ui3dStack;
Ui3dTextureWindow g_ui3d_windows[6];
Ui3dStatus g_ui3dStatus;


GfxUI3DStack *__cdecl R_GetUI3DStack()
{
    return &g_ui3dStack;
}

void __cdecl R_UI3DStack_Push(GfxUI3DStack *stack, int val)
{
    if ( stack->size != 2 )
        stack->stack[stack->size++] = val;
}

void __cdecl R_UI3DStack_Pop(GfxUI3DStack *stack)
{
    if ( stack->size <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp", 73, 0, "%s", "stack->size > 0") )
    {
        __debugbreak();
    }
    --stack->size;
}

int __cdecl R_UI3DStack_Top(GfxUI3DStack *stack)
{
    int size; // [esp+0h] [ebp-4h]

    size = stack->size;
    if ( size < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp", 80, 0, "%s", "size >= 0") )
        __debugbreak();
    if ( size )
        return stack->stack[size - 1];
    else
        return -1;
}

ScreenPlacement *__cdecl R_UI3D_ScrPlaceFromTextureWindow(unsigned int windowIndex)
{
    if ( windowIndex >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp",
                    93,
                    0,
                    "windowIndex doesn't index UI3D_MAX_NUM_TEXTURE_WINDOWS\n\t%i not in [0, %i)",
                    windowIndex,
                    6) )
    {
        __debugbreak();
    }
    return &g_ui3d_windows[windowIndex].scrPlace;
}

void __cdecl R_UI3D_SetBlurRadius(float blurRadius)
{
    g_ui3dStatus.blurRadius = blurRadius;
}

void __cdecl R_UI3D_CheckRenderTarget()
{
    ScreenPlacement *scrPlace; // [esp+10h] [ebp-Ch]
    signed int i; // [esp+18h] [ebp-4h]

    if (g_ui3dStatus.useDisplaySize)
    {
        g_ui3dStatus.width = vidConfig.displayWidth;
        g_ui3dStatus.height = vidConfig.displayHeight;
    }
    if (!gfxRenderTargets[R_RENDERTARGET_UI3D].width && g_ui3dStatus.initialized)
    {
        R_InitUI3DRenderTarget(
            g_ui3dStatus.width,
            g_ui3dStatus.height,
            g_ui3dStatus.pmemLocation,
            g_ui3dStatus.hasPingPongBuffer);
        for (i = 0; i < 6; ++i)
            R_UI3D_SetupTextureWindow(
                i,
                g_ui3d_windows[i].normX,
                g_ui3d_windows[i].normY,
                g_ui3d_windows[i].normW,
                g_ui3d_windows[i].normH);
        if (r_glob.isMultiplayer)
        {
            scrPlace = R_UI3D_ScrPlaceFromTextureWindow(0);
            ScrPlace_SetupUI3DForFullscreen(scrPlace, &scrPlaceFull);
        }
    }
}

void __cdecl R_UI3D_OnetimeInit(
                unsigned __int16 width,
                unsigned __int16 height,
                bool useDisplaySize,
                int pmemLocation,
                bool allocatePingPongBuffer)
{
    signed int i; // [esp+10h] [ebp-4h]

    if ( g_ui3dStatus.initialized
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp",
                    155,
                    0,
                    "%s",
                    "!g_ui3dStatus.initialized") )
    {
        __debugbreak();
    }
    g_ui3dStatus.width = width;
    g_ui3dStatus.height = height;
    g_ui3dStatus.useDisplaySize = useDisplaySize;
    g_ui3dStatus.pmemLocation = pmemLocation;
    R_InitUI3DRenderTarget(width, height, pmemLocation, allocatePingPongBuffer);
    g_ui3dStatus.hasPingPongBuffer = allocatePingPongBuffer;
    g_ui3dStatus.blurRadius = 0.0f;
    g_ui3dStatus.initialized = 1;
    for ( i = 0; i < 6; ++i )
        R_UI3D_SetupTextureWindow(i, 0.0, 0.0, 0.0, 0.0);
}

void __cdecl R_UI3D_Shutdown()
{
    if ( g_ui3dStatus.initialized )
    {
        g_ui3dStatus.initialized = 0;
        while ( g_ui3dStatus.rendering )
            Sleep(1u);
        R_ShutdownUI3DRenderTarget(g_ui3dStatus.hasPingPongBuffer);
        g_ui3dStatus.hasPingPongBuffer = 0;
        g_ui3dStatus.blurRadius = 0.0f;
        g_ui3dStatus.initialized = 0;
    }
}

void __cdecl R_UI3D_PerframeInit()
{
    int i; // [esp+10h] [ebp-4h]

    R_InitUI3DStack(&g_ui3dStack);
    for ( i = 0; i < 6; ++i )
        g_ui3d_windows[i].numRenderCmds = 0;
    if ( r_ui3d_use_debug_values->current.integer >= 0 )
        R_UI3D_SetupTextureWindow(
            r_ui3d_use_debug_values->current.unsignedInt,
            r_ui3d_x->current.value,
            r_ui3d_y->current.value,
            r_ui3d_w->current.value,
            r_ui3d_h->current.value);
}

void __cdecl R_InitUI3DStack(GfxUI3DStack *stack)
{
    stack->size = 0;
}

void __cdecl R_UI3D_SetupBackendData_UVConstants(GfxUI3DBackend *ui3dBackend)
{
    signed int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 6; ++i )
        R_UI3D_GetUVSetup(i, ui3dBackend->uvSetup[i]);
}

void __cdecl R_UI3D_GetUVSetup(unsigned int windowIndex, float *out)
{
    float normH; // [esp+4h] [ebp-10h]
    float normX; // [esp+8h] [ebp-Ch]
    float normY; // [esp+Ch] [ebp-8h]

    if ( windowIndex >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp",
                    100,
                    0,
                    "windowIndex doesn't index UI3D_MAX_NUM_TEXTURE_WINDOWS\n\t%i not in [0, %i)",
                    windowIndex,
                    6) )
    {
        __debugbreak();
    }
    normH = g_ui3d_windows[windowIndex].normH;
    normX = g_ui3d_windows[windowIndex].normX;
    normY = g_ui3d_windows[windowIndex].normY;
    *out = g_ui3d_windows[windowIndex].normW;
    out[1] = normH;
    out[2] = normX;
    out[3] = normY;
}

void __cdecl R_UI3D_SetupBackendData(GfxUI3DBackend *ui3dBackend)
{
    Ui3dTextureWindow *v1; // ecx
    GfxViewport *v2; // edx
    int i; // [esp+0h] [ebp-8h]
    int cmdCount; // [esp+4h] [ebp-4h]

    cmdCount = 0;
    for ( i = 0; i < 6; ++i )
    {
        cmdCount += g_ui3d_windows[i].numRenderCmds;
        ui3dBackend->renderCmdCount[i] = g_ui3d_windows[i].numRenderCmds;
        v1 = &g_ui3d_windows[i];
        v2 = &ui3dBackend->viewport[i];
        v2->x = v1->vp.x;
        v2->y = v1->vp.y;
        v2->width = v1->vp.width;
        v2->height = v1->vp.height;
    }
    ui3dBackend->totalRenderCmds = cmdCount;
    ui3dBackend->blurRadius = g_ui3dStatus.blurRadius;
    R_UI3D_SetupBackendData_UVConstants(ui3dBackend);
}

void __cdecl R_UI3D_IncrementRenderCmdCount(unsigned int wndIndex)
{
    if ( wndIndex < 6 )
        ++g_ui3d_windows[wndIndex].numRenderCmds;
}

void __cdecl R_UI3D_SetupTextureWindow(
                unsigned int windowIndex,
                float normX,
                float normY,
                float normWidth,
                float normHeight)
{
    float v5; // [esp+0h] [ebp-38h]
    float v6; // [esp+4h] [ebp-34h]
    float v7; // [esp+8h] [ebp-30h]
    float v8; // [esp+Ch] [ebp-2Ch]
    float v9; // [esp+10h] [ebp-28h]
    float v10; // [esp+18h] [ebp-20h]
    float v11; // [esp+20h] [ebp-18h]
    float v12; // [esp+24h] [ebp-14h]
    int windowPixelWidth; // [esp+28h] [ebp-10h]
    Ui3dTextureWindow *uiWnd; // [esp+30h] [ebp-8h]
    int windowPixelHeight; // [esp+34h] [ebp-4h]

    if ( !g_ui3dStatus.initialized
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_ui3d.cpp",
                    298,
                    0,
                    "%s",
                    "g_ui3dStatus.initialized") )
    {
        __debugbreak();
    }
    if ( windowIndex < 6 )
    {
        if ( (float)(normX - 1.0) < 0.0 )
            v12 = normX;
        else
            v12 = 1.0f;
        if ( (float)(0.0 - normX) < 0.0 )
            v8 = v12;
        else
            v8 = 0.0f;
        if ( (float)(normY - 1.0) < 0.0 )
            v11 = normY;
        else
            v11 = 1.0f;
        if ( (float)(0.0 - normY) < 0.0 )
            v7 = v11;
        else
            v7 = 0.0f;
        if ( (float)((float)(1.0 - v8) - normWidth) < 0.0 )
            v10 = 1.0 - v8;
        else
            v10 = normWidth;
        if ( (float)(v10 - 0.0) < 0.0 )
            v6 = 0.0f;
        else
            v6 = v10;
        if ( (float)((float)(1.0 - v7) - normHeight) < 0.0 )
            v9 = 1.0 - v7;
        else
            v9 = normHeight;
        if ( (float)(v9 - 0.0) < 0.0 )
            v5 = 0.0f;
        else
            v5 = v9;
        windowPixelWidth = (int)(float)((float)g_ui3dStatus.width * v6);
        windowPixelHeight = (int)(float)((float)g_ui3dStatus.height * v5);
        uiWnd = &g_ui3d_windows[windowIndex];
        ScrPlace_SetupUI3D(&uiWnd->scrPlace, windowPixelWidth, windowPixelHeight);
        uiWnd->vp.x = (int)(float)((float)g_ui3dStatus.width * v8);
        uiWnd->vp.y = (int)(float)((float)g_ui3dStatus.height * v7);
        uiWnd->vp.width = windowPixelWidth;
        uiWnd->vp.height = windowPixelHeight;
        uiWnd->normX = v8;
        uiWnd->normY = v7;
        uiWnd->normW = v6;
        uiWnd->normH = v5;
    }
}

void __cdecl ScrPlace_SetupUI3D(ScreenPlacement *scrPlace, int viewportWidth, int viewportHeight)
{
    memset(scrPlace, 0xB0u, sizeof(ScreenPlacement));
    scrPlace->realViewportBase[0] = 0.0f;
    scrPlace->realViewportBase[1] = 0.0f;
    scrPlace->virtualViewableMin[0] = 0.0f;
    scrPlace->virtualViewableMin[1] = 0.0f;
    scrPlace->virtualViewableMax[0] = (float)viewportWidth;
    scrPlace->virtualViewableMax[1] = (float)viewportHeight;
    scrPlace->realViewportSize[0] = (float)viewportWidth;
    scrPlace->realViewportSize[1] = (float)viewportHeight;
    scrPlace->realViewableMin[0] = 0.0f;
    scrPlace->realViewableMin[1] = 0.0f;
    scrPlace->realViewableMax[0] = (float)viewportWidth;
    scrPlace->realViewableMax[1] = (float)viewportHeight;
    scrPlace->subScreen[0] = 0.0f;
    scrPlace->subScreen[1] = 0.0f;
    scrPlace->scaleVirtualToReal[0] = (float)viewportWidth / 640.0;
    scrPlace->scaleVirtualToReal[1] = (float)viewportHeight / 480.0;
    scrPlace->scaleVirtualToFull[0] = (float)viewportWidth / 640.0;
    scrPlace->scaleVirtualToFull[1] = (float)viewportHeight / 480.0;
    scrPlace->scaleRealToVirtual[0] = 640.0 / (float)viewportWidth;
    scrPlace->scaleRealToVirtual[1] = 480.0 / (float)viewportHeight;
}

void __cdecl RB_UI3D_SetShaderConstants(GfxCmdBufSourceState *source, const GfxUI3DBackend *rbUI3D)
{
    int j; // [esp+8h] [ebp-18h]
    float uvsetup[4]; // [esp+Ch] [ebp-14h] BYREF
    int i; // [esp+1Ch] [ebp-4h]

    if ( rbUI3D )
    {
        for ( i = 0; i < 6; ++i )
            R_SetCodeConstantFromVec4(source, (CodeConstant)(i + 173), (float*)rbUI3D->uvSetup[i]);
    }
    else
    {
        uvsetup[0] = 0.0f;
        uvsetup[1] = 0.0f;
        uvsetup[2] = 0.01f;
        uvsetup[3] = 0.01f;
        for ( j = 0; j < 6; ++j )
            R_SetCodeConstantFromVec4(source, (CodeConstant)(j + 173), uvsetup);
    }
}

void __cdecl RB_SetUI3DSamplerAndConstants(GfxCmdBufSourceState *cmdBufSrcState, const GfxUI3DBackend *rbUI3D)
{
    if (gfxRenderTargets[R_RENDERTARGET_UI3D].image)
        R_SetCodeImageTexture(cmdBufSrcState, 0x27u, gfxRenderTargets[R_RENDERTARGET_UI3D].image);
    if (rbUI3D)
        RB_UI3D_SetShaderConstants(cmdBufSrcState, rbUI3D);
}

void __cdecl RB_UI3D_RenderToTexture(const void *cmds, const GfxUI3DBackend *rbUI3D, const GfxCmdBufInput *input)
{
  int v3; // [esp+10h] [ebp-CCh]
  int v4; // [esp+14h] [ebp-C8h]
  int v5; // [esp+18h] [ebp-C4h]
  int v6; // [esp+1Ch] [ebp-C0h]
  int v7; // [esp+20h] [ebp-BCh]
  int y; // [esp+24h] [ebp-B8h]
  int v9; // [esp+28h] [ebp-B4h]
  int x; // [esp+2Ch] [ebp-B0h]
  int ui3dTextureWindow; // [esp+70h] [ebp-6Ch] BYREF
  const GfxViewport *viewport; // [esp+74h] [ebp-68h]
  int i; // [esp+78h] [ebp-64h]
  int wIdx; // [esp+7Ch] [ebp-60h]
  const GfxViewport *vp; // [esp+80h] [ebp-5Ch]
  int vindex; // [esp+84h] [ebp-58h]
  int scissor_min_y; // [esp+8Ch] [ebp-50h]
  int scissor_max_y; // [esp+90h] [ebp-4Ch]
  unsigned __int8 rt; // [esp+97h] [ebp-45h]
  GfxViewport vpBounds; // [esp+98h] [ebp-44h] BYREF
  float clearColor[4]; // [esp+A8h] [ebp-34h] BYREF
  int validWindows[6]; // [esp+B8h] [ebp-24h] BYREF
  int scissor_min_x; // [esp+D0h] [ebp-Ch]
  int validWindowsCount; // [esp+D4h] [ebp-8h]
  int scissor_max_x; // [esp+D8h] [ebp-4h]

  rt = 20;
  if ( rbUI3D->totalRenderCmds >= 1 )
  {
    if ( cmds )
    {
      validWindowsCount = RB_UI3D_GetValidWindows(rbUI3D, validWindows);
      if ( validWindowsCount )
      {
        g_ui3dStatus.rendering = 1;
        if ( g_ui3dStatus.initialized )
        {
          PROF_SCOPED("RB_UI3D_RenderToTexture");
          R_InitCmdBufSourceState(&gfxCmdBufSourceState, input, 0);
          gfxCmdBufSourceState.input.data = backEndData;
          R_InitLocalCmdBufState(&gfxCmdBufState);
          R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_UI3D);
          R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_UI3D);
          scissor_min_x = 4095;
          scissor_max_x = 0;
          scissor_min_y = 4095;
          scissor_max_y = 0;
          for ( vindex = 0; vindex < validWindowsCount; ++vindex )
          {
            wIdx = validWindows[vindex];
            vp = &rbUI3D->viewport[wIdx];
            if ( vp->x < scissor_min_x )
              x = rbUI3D->viewport[wIdx].x;
            else
              x = scissor_min_x;
            scissor_min_x = x;
            if ( scissor_max_x < vp->width + vp->x )
              v9 = vp->width + vp->x;
            else
              v9 = scissor_max_x;
            scissor_max_x = v9;
            if ( vp->y < scissor_min_y )
              y = vp->y;
            else
              y = scissor_min_y;
            scissor_min_y = y;
            if ( scissor_max_y < vp->height + vp->y )
              v7 = vp->height + vp->y;
            else
              v7 = scissor_max_y;
            scissor_max_y = v7;
          }
          if ( scissor_max_x < scissor_min_x )
            v6 = scissor_max_x;
          else
            v6 = scissor_min_x;
          vpBounds.x = v6;
          if ( scissor_min_x < scissor_max_x )
            v5 = scissor_max_x;
          else
            v5 = scissor_min_x;
          vpBounds.width = v5 - vpBounds.x;
          if ( scissor_max_y < scissor_min_y )
            v4 = scissor_max_y;
          else
            v4 = scissor_min_y;
          vpBounds.y = v4;
          if ( scissor_min_y < scissor_max_y )
            v3 = scissor_max_y;
          else
            v3 = scissor_min_y;
          vpBounds.height = v3 - vpBounds.y;
          memset(clearColor, 0, sizeof(clearColor));
          R_ClearScreen(gfxCmdBufState.prim.device, 1u, clearColor, 1.0, 0, &vpBounds);
          for ( i = 0; i < validWindowsCount; ++i )
          {
            ui3dTextureWindow = validWindows[i];
            if ( rgp.heatMapImage )
              R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.heatMapImage);
            else
              R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.whiteImage);
            viewport = &rbUI3D->viewport[ui3dTextureWindow];
            R_HW_EnableScissor(
              gfxCmdBufContext.state->prim.device,
              viewport->x,
              viewport->y,
              viewport->width,
              viewport->height);
            R_SetViewportStruct(&gfxCmdBufSourceState, viewport);
            R_Set2D(&gfxCmdBufSourceState);
            RB_ExecuteRenderCommandsLoop(cmds, &ui3dTextureWindow);
          }
          R_HW_DisableScissor(gfxCmdBufContext.state->prim.device);
          memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
          if (rbUI3D->blurRadius > 0.0
              && gfxRenderTargets[R_RENDERTARGET_UI3D_PING_PONG].image
              && gfxRenderTargets[R_RENDERTARGET_UI3D_PING_PONG].width == gfxRenderTargets[R_RENDERTARGET_UI3D].width
              && gfxRenderTargets[R_RENDERTARGET_UI3D_PING_PONG].height == gfxRenderTargets[R_RENDERTARGET_UI3D].height)
          {
            RB_GaussianFilterImage(rbUI3D->blurRadius, R_RENDERTARGET_UI3D, R_RENDERTARGET_UI3D_PING_PONG);
            RB_GaussianFilterImage(rbUI3D->blurRadius, R_RENDERTARGET_UI3D_PING_PONG, R_RENDERTARGET_UI3D);
          }
          g_ui3dStatus.rendering = 0;
        }
        else
        {
          g_ui3dStatus.rendering = 0;
        }
      }
    }
  }
}

int __cdecl RB_UI3D_GetValidWindows(const GfxUI3DBackend *rbUI3D, int *validWindowsArray)
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    for ( i = 0; i < 6; ++i )
    {
        if ( rbUI3D->renderCmdCount[i] >= 1 && RB_UI3D_ViewportIsValid(&rbUI3D->viewport[i]) )
            validWindowsArray[count++] = i;
    }
    return count;
}

bool __cdecl RB_UI3D_ViewportIsValid(const GfxViewport *vp)
{
    return vp->height > 0 && vp->width > 0;
}

