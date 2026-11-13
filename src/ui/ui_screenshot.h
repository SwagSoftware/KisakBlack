#pragma once
#include "ui_utils.h"

struct FileShareReadFileTask;

void __cdecl UI_Screenshot_Success(FileShareReadFileTask *task);
void __cdecl UI_Screenshot_Failure(FileShareReadFileTask *task);
void __cdecl UI_ScreenshotAlloc();
void __cdecl UI_ScreenshotShutdown();
void __cdecl UI_ScreenshotDecompressCallback(void *data);
void __cdecl UI_ScreenshotUpdate(int localClientNum, int contextIndex);
char __cdecl UI_ScreenshotDecompress(int localClientNum, int contextIndex);
void __cdecl UI_ScreenshotHandleInput(int localClientNum);
void __cdecl UI_ScreenshotDownload(int localClientNum, int context, unsigned __int64 fileID, unsigned int fileSize);
void __cdecl UI_ScreenshotDraw(int localClientNum, int contextIndex, const rectDef_s *rect, const float *color);
void __cdecl UI_ScreenshotInit();
void __cdecl UI_ScreenshotDownload_f();
void __cdecl UI_ScreenshotAbortDownload_f();
void __cdecl UI_ScreenshotZoom_f();
