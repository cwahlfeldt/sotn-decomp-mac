// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef PC_H
#define PC_H

#define LEN(x) ((s32)(sizeof(x) / sizeof(*(x))))

#ifndef VERSION_PC
#define VERSION_PC
#endif
#define VERSION_US
#ifdef VERSION
#undef VERSION
#endif
#define VERSION "us"

#include <stdio.h>
#include <log.h>

// native size of the PSX display area; the window scales this freely
#define DISP_WIDTH 400
#define DISP_HEIGHT 256

#define VRAM_W 1024
#define VRAM_H 512
#define VRAM_STRIDE 2048

enum TestMode {
    NO_TEST,
    TEST_SNDLIB,
};
struct InitGameParams {
    const char* diskPath;
    enum TestMode testMode;
    int stage;
    int player;
    int scale;
    int windowW; // initial window size; 0 = DISP_WIDTH/HEIGHT * scale
    int windowH;
    int fullscreen;
};

struct FileOpenRead {
    const char* filename;
    FILE* file;
    size_t length;
    void* param;
};

struct FileAsString {
    const char* filename;
    const char* content;
    size_t length;
    void* param;
};

typedef struct FileUseContent {
    const char* filename;
    const void* content;
    size_t length;
    void* param;
} FileLoad;

extern struct InitGameParams g_GameParams;
bool InitGame(struct InitGameParams* params);
void MainGame(void);
void ResetGame(void);

bool FileOpenRead(
    bool (*cb)(const struct FileOpenRead*), const char* filename, void* param);
int FileReadToBuf(const char* filename, void* dst, int offset, size_t maxlen);
bool FileAsString(bool (*cb)(const struct FileAsString* file),
                  const char* filename, void* param);
bool FileUseContent(bool (*cb)(const struct FileUseContent* file, void* param),
                    const char* filename, void* param);

const char* AnsiToSotnMenuString(const char* str);

#endif
