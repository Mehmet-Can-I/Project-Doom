/* wad_access.c - WAD dosyasını dosya sistemi olmadan okuma
 *
 * doomgeneric WAD'ı w_file_stdc.c üzerinden fopen/fread ile açar.
 * Gömülü sistemde dosya sistemi yok; bunun yerine w_file.h arayüzüne
 * uyan, WAD'ı doğrudan bellek adresinden okuyan bir sınıf sağlıyoruz.
 *
 * Kurulum: w_file.c içindeki wad_file_classes[] dizisine &memory_wad_file ekleyin
 * ve w_file_stdc.c'yi derlemeden çıkarın.
 *
 * Daha da iyisi: W_CacheLumpNum'u, lump'ı RAM'e kopyalamak yerine doğrudan
 * flash adresini döndürecek şekilde değiştirin (zero-copy). Bu, RAM tasarrufunun
 * en büyük kalemidir. mapped != NULL ise motor bunu kısmen zaten yapar.
 */
#include <string.h>
#include "w_file.h"
#include "z_zone.h"
#include "board_config.h"

#ifdef WAD_SOURCE_QSPI
  #define WAD_BASE  ((const uint8_t *)WAD_QSPI_BASE_ADDR)
#else
  extern const uint8_t wad_data[];               /* objcopy ile gömülü */
  #define WAD_BASE  (wad_data)
#endif

extern wad_file_class_t memory_wad_file;

/* WAD başlığı: "IWAD", numlumps, infotableofs; boyutu dizin sonundan hesaplanır */
static uint32_t wad_size_from_header(const uint8_t *base)
{
    uint32_t numlumps, infoofs;
    memcpy(&numlumps, base + 4, 4);
    memcpy(&infoofs,  base + 8, 4);
    return infoofs + numlumps * 16;
}

static wad_file_t *mem_OpenFile(char *path)
{
    (void)path;                                  /* tek WAD var, yol yok sayılır */
    if (memcmp(WAD_BASE, "IWAD", 4) != 0 && memcmp(WAD_BASE, "PWAD", 4) != 0)
        return NULL;

    wad_file_t *f = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    f->file_class = &memory_wad_file;
    f->mapped     = (byte *)WAD_BASE;            /* memory-mapped: kopya yok */
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
