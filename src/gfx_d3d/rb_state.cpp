#include "rb_state.h"
#include "r_rendercmds.h"
#include "r_state_utils.h"
#include "r_init.h"
#include "r_state.h"
#include "r_rendertarget.h"
#include "rb_backend.h"

GfxCmdBufState gfxCmdBufState;
GfxCmdBufInput gfxCmdBufInput;
GfxCmdBufSourceState gfxCmdBufSourceState;

GfxCmdBufContext gfxCmdBufContext=
{ &gfxCmdBufSourceState, &gfxCmdBufState };


void __cdecl RB_SetInitialState()
{
  memset(&gfxCmdBufInput, 0, sizeof(gfxCmdBufInput));
  R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
  gfxCmdBufState.prim.device = dx.device;

  R_SetInitialContextState(dx.device);
  R_SetTexFilter();
  R_InitCmdBufState(&gfxCmdBufState);
  RB_InitSceneViewport();

  iassert(gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color);
  if (!g_allocateMinimalResources)
  {
      iassert(gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.depthStencil);
  }

  iassert(gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].width);
  iassert(gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].height);
  iassert(gfxCmdBufSourceState.renderTargetWidth == 0);
  iassert(gfxCmdBufSourceState.renderTargetHeight == 0);
  iassert(gfxCmdBufSourceState.viewportBehavior == 0);

  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);

  iassert(gfxCmdBufState.prim.indexBuffer == NULL);
  iassert(gfxCmdBufState.prim.streams[0].vb == NULL);
  iassert(gfxCmdBufState.prim.streams[0].offset == 0);
  iassert(gfxCmdBufState.prim.streams[0].stride == 0);
  iassert(gfxCmdBufState.prim.streams[1].vb == NULL);
  iassert(gfxCmdBufState.prim.streams[1].offset == 0);
  iassert(gfxCmdBufState.prim.streams[1].stride == 0);
  iassert(gfxCmdBufState.vertexShader == NULL);
  iassert(gfxCmdBufState.material == NULL);
  iassert(gfxCmdBufState.technique == NULL);
  iassert(gfxCmdBufState.alphaRef == 0);
}

void __cdecl RB_UnbindAllImages()
{
    unsigned int samplerIndex; // [esp+0h] [ebp-8h]

    if ( dx.device )
    {
        for ( samplerIndex = 0; samplerIndex < vidConfig.maxTextureMaps; ++samplerIndex )
            R_DisableSampler(&gfxCmdBufState, samplerIndex);
    }
}

void __cdecl RB_InitSceneViewport()
{
    gfxCmdBufSourceState.sceneViewport.x = 0;
    gfxCmdBufSourceState.sceneViewport.y = 0;
    gfxCmdBufSourceState.sceneViewport.width = vidConfig.displayWidth;
    gfxCmdBufSourceState.sceneViewport.height = vidConfig.displayHeight;
}

void __cdecl RB_BindDefaultImages()
{
  GfxCmdBufContext context; // [esp+0h] [ebp-10h]
  unsigned int samplerIndex; // [esp+8h] [ebp-8h]

  context.source = &gfxCmdBufSourceState;
  context.state = &gfxCmdBufState;
  for ( samplerIndex = 0; samplerIndex < 0x10; ++samplerIndex )
    R_SetSampler(context, samplerIndex, 1u, rgp.whiteImage);
}

void __cdecl RB_InitImages()
{

    RB_InitCodeImages();
    RB_BindDefaultImages();
}

