/* wad_access.c - Reading the WAD file without a filesystem
 *
 * doomgeneric opens the WAD through w_file_stdc.c using fopen/fread.
 * An embedded system has no filesystem, so instead we provide a class that
 * implements the w_file.h interface and reads the WAD directly from a memory address.
 *
 * Setup: add &memory_wad_file to the wad_file_classes[] array in w_file.c
 * and exclude w_file_stdc.c from the build.
 *
 * Even better: modify W_CacheLumpNum to return the flash address directly
 * instead of copying the lump into RAM (zero-copy). This is the single biggest
 * RAM saving. The engine already partly does this when mapped != NULL.
 */
#include <string.h>
#include "w_file.h"
#include "z_zone.h"
#include "board_config.h"

#ifdef WAD_SOURCE_QSPI
  #define WAD_BASE  ((const uint8_t *)WAD_QSPI_BASE_ADDR)
#else
  extern const uint8_t wad_data[];               /* embedded via objcopy */
  #define WAD_BASE  (wad_data)
#endif

extern wad_file_class_t memory_wad_file;

/* WAD header: "IWAD", numlumps, infotableofs; size is computed from the end of the directory */
static uint32_t wad_size_from_header(const uint8_t *base)
{
    uint32_t numlumps, infoofs;
    memcpy(&numlumps, base + 4, 4);
    memcpy(&infoofs,  base + 8, 4);
    return infoofs + numlumps * 16;
}

static wad_file_t *mem_OpenFile(char *path)
{
    (void)path;                                  /* single WAD, path ignored */
    if (memcmp(WAD_BASE, "IWAD", 4) != 0 && memcmp(WAD_BASE, "PWAD", 4) != 0)
        return NULL;

    wad_file_t *f = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    f->file_class = &memory_wad_file;
    f->mapped     = (byte *)WAD_BASE;            /* memory-mapped: no copy */
    f->length     = wad_size_from_header(WAD_BASE);
    return f;
}

static void mem_CloseFile(wad_file_t *wad)
{
    Z_Free(wad);
}

static size_t mem_Read(wad_file_t *wad, unsigned int offset, void *buffer, size_t len)
{
    if (offset >= wad->length) return 0;
    if (offset + len > wad->length) len = wad->length - offset;
    memcpy(buffer, WAD_BASE + offset, len);
    return len;
}

wad_file_class_t memory_wad_file = {
    mem_OpenFile,
    mem_CloseFile,
    mem_Read,
};
