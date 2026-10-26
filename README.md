# NUCLEO-L476RG Doom — Proje İskeleti

## Klasör yapısı

```
nucleo_doom/
├── Core/
│   ├── Inc/
│   │   ├── board_config.h      Pin atamaları, tek yerden değişir
│   │   ├── keyboard.h
│   │   └── lcd_ili9341.h
│   └── Src/
│       ├── main.c              CubeMX main.c'ye eklenecek kısımlar
│       ├── keyboard.c          4x4 matris tarama + debounce + olay kuyruğu
│       ├── lcd_ili9341.c       SPI + DMA ekran sürücüsü
│       ├── doomgeneric_stm32.c doomgeneric platform katmanı (DG_* fonksiyonları)
│       └── wad_access.c        WAD'ı dosya sistemi olmadan, bellekten okuma
└── Doom/                       github.com/ozkl/doomgeneric kaynakları buraya
```

## Kurulum adımları

1. STM32CubeIDE'de NUCLEO-L476RG için yeni proje açın.
2. CubeMX ayarları:
   - **Saat:** HSI16 → PLL → SYSCLK **80 MHz**
   - **SPI1:** Transmit Only Master, 8 bit, prescaler /2, DMA: SPI1_TX (Normal, byte)
   - **USART2:** 115200 baud (ST-LINK VCP, printf için)
   - **TIM6:** Prescaler 79, Period 999 → 1 kHz, NVIC kesmesi açık
   - **GPIO:** `board_config.h`'daki LCD CS/DC/RST çıkışları
3. `doomgeneric/doomgeneric/` içindeki `.c/.h` dosyalarını `Doom/` klasörüne kopyalayın.
   Şunları derlemeden çıkarın: `doomgeneric_*.c` (SDL, X11, win vb. portlar),
   `w_file_stdc.c`, ses dosyaları (`i_sdlsound.c`, `i_sdlmusic.c`).
4. `w_file.c` içindeki `wad_file_classes[]` dizisine `&memory_wad_file` ekleyin.
5. `i_video.c` içindeki `I_SetPalette()` fonksiyonundan `dg_set_palette()` çağırın.

## Bellek bütçesi — asıl zorluk burası

STM32L476RG: **1 MB flash, 128 KB RAM** (96 KB SRAM1 + 32 KB SRAM2).
Masaüstü Doom birkaç MB RAM kullanır; olduğu gibi **sığmaz**. Yapılması gerekenler:

| Kalem | Varsayılan | Hedef | Nasıl |
|---|---|---|---|
| Ekran tamponu | 256 KB (32-bit) | 64 KB | 8-bit paletli, DG_ScreenBuffer'ı kaldır |
| Zone heap (`z_zone.c`) | 6 MB | ~40–50 KB | `I_ZoneBase` boyutunu küçült |
| WAD lump'ları | RAM'e kopyalanır | 0 KB | Zero-copy: flash adresini doğrudan döndür |
| Ses | — | 0 KB | İlk aşamada tamamen kapalı |

64 KB'lık ekran tamponu SRAM1'in büyük kısmını yer; çok sıkışırsanız seçenekler:
**düşük detay modu** (`detailLevel = 1`, yatay yarım çözünürlük → 32 KB) veya
pencere boyutunu küçültmek (`screenblocks`).

**WAD:** Shareware `doom1.wad` ~4 MB, dahili 1 MB flash'a sığmaz. İki yol:
- Harici **QSPI NOR flash** (ör. W25Q64, 8 MB) → memory-mapped okuma, `WAD_SOURCE_QSPI`
- Sadece birkaç bölüm içeren küçültülmüş WAD → dahili flash, `WAD_SOURCE_INTERNAL`

Not: NUCLEO-L476RG'nin 64 pinli kılıfında FMC yok, yani harici SRAM eklenemez.
RAM tavanı 128 KB'dır; bu yüzden zero-copy WAD erişimi zorunlu.

Referans olarak next-hack'in 256 KB RAM ile nRF52840'a yaptığı Doom portu, bellek
optimizasyonu tekniklerini iyi belgeler; yaklaşımını incelemenizi öneririm.

## Klavye

4x4 matris, `keyboard.c` içindeki `keymap[][]` tablosundan eşlenir:

| | C0 | C1 | C2 | C3 |
|---|---|---|---|---|
| **R0** | İleri | Ateş | Kullan | ESC |
| **R1** | Sola dön | Geri | Sağa dön | Enter |
| **R2** | Strafe sol | Koş | Strafe sağ | Harita |
| **R3** | Silah 1 | Silah 2 | Silah 3 | Silah 4 |

Her tuşa seri **1N4148 diyot** koyun; Doom'da aynı anda 3–4 tuşa basılır,
diyotsuz matriste hayalet tuşlar oluşur.

## Önerilen geliştirme sırası

1. LCD'ye renk doldur (`lcd_fill`) → ekran + SPI + DMA çalışıyor mu
2. Klavye olaylarını UART'a yazdır → tarama + debounce doğru mu
3. QSPI'den WAD başlığını oku, "IWAD" imzasını UART'a yaz
4. Zone heap'i küçültüp `doomgeneric_Create` → ilk menü ekranı
5. Oyun içi FPS ölçümü ve optimizasyon (`-O2`, kritik döngüleri SRAM'e/ITCM'e taşıma)
