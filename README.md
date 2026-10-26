# NUCLEO-L476RG Doom — Project Skeleton

## Folder structure

```
nucleo_doom/
├── Core/
│   ├── Inc/
│   │   ├── board_config.h      Pin assignments, changed in one place
│   │   ├── keyboard.h
│   │   └── lcd_ili9341.h
│   └── Src/
│       ├── main.c              Parts to add to the CubeMX-generated main.c
│       ├── keyboard.c          4x4 matrix scanning + debounce + event queue
│       ├── lcd_ili9341.c       SPI + DMA display driver
│       ├── doomgeneric_stm32.c doomgeneric platform layer (DG_* functions)
│       └── wad_access.c        Reading the WAD from memory, no filesystem
└── Doom/                       Sources from github.com/ozkl/doomgeneric go here
```

## Setup steps

1. Create a new STM32CubeIDE project for the NUCLEO-L476RG.
2. CubeMX settings:
   - **Clock:** HSI16 → PLL → SYSCLK **80 MHz**
   - **SPI1:** Transmit Only Master, 8 bit, prescaler /2, DMA: SPI1_TX (Normal, byte)
   - **USART2:** 115200 baud (ST-LINK VCP, for printf)
   - **TIM6:** Prescaler 79, Period 999 → 1 kHz, NVIC interrupt enabled
   - **GPIO:** LCD CS/DC/RST outputs as listed in `board_config.h`
3. Copy the `.c/.h` files from `doomgeneric/doomgeneric/` into the `Doom/` folder.
   Exclude from the build: `doomgeneric_*.c` (SDL, X11, Windows etc. ports),
   `w_file_stdc.c`, and the sound files (`i_sdlsound.c`, `i_sdlmusic.c`).
4. Add `&memory_wad_file` to the `wad_file_classes[]` array in `w_file.c`.
5. Call `dg_set_palette()` from `I_SetPalette()` in `i_video.c`.

## Memory budget — the real challenge

STM32L476RG: **1 MB flash, 128 KB RAM** (96 KB SRAM1 + 32 KB SRAM2).
Desktop Doom uses several MB of RAM; as-is it **will not fit**. What needs to be done:

| Item | Default | Target | How |
|---|---|---|---|
| Screen buffer | 256 KB (32-bit) | 64 KB | 8-bit palettized, remove DG_ScreenBuffer |
| Zone heap (`z_zone.c`) | 6 MB | ~40–50 KB | Shrink the `I_ZoneBase` size |
| WAD lumps | Copied to RAM | 0 KB | Zero-copy: return the flash address directly |
| Sound | — | 0 KB | Fully disabled in the first phase |

The 64 KB screen buffer takes up most of SRAM1; if things get too tight, options are
**low detail mode** (`detailLevel = 1`, half horizontal resolution → 32 KB) or
shrinking the view window (`screenblocks`).

**WAD:** The shareware `doom1.wad` is ~4 MB and does not fit in the internal 1 MB flash. Two options:
- External **QSPI NOR flash** (e.g. W25Q64, 8 MB) → memory-mapped reads, `WAD_SOURCE_QSPI`
- A trimmed WAD containing only a few levels → internal flash, `WAD_SOURCE_INTERNAL`

Note: The NUCLEO-L476RG's 64-pin package has no FMC, so external SRAM cannot be added.
The RAM ceiling is 128 KB, which is why zero-copy WAD access is mandatory.

As a reference, next-hack's Doom port to the nRF52840 (256 KB RAM) documents
memory optimization techniques well; studying its approach is recommended.

## Keyboard

4x4 matrix, mapped via the `keymap[][]` table in `keyboard.c`:

| | C0 | C1 | C2 | C3 |
|---|---|---|---|---|
| **R0** | Forward | Fire | Use | ESC |
| **R1** | Turn left | Back | Turn right | Enter |
| **R2** | Strafe left | Run | Strafe right | Automap |
| **R3** | Weapon 1 | Weapon 2 | Weapon 3 | Weapon 4 |

Put a **1N4148 diode** in series with every key; in Doom 3–4 keys are held at once,
and a diode-less matrix will produce ghost keys.

## Suggested development order

1. Fill the LCD with a color (`lcd_fill`) → are the display, SPI and DMA working?
2. Print keyboard events to UART → are scanning and debouncing correct?
3. Read the WAD header from QSPI and print the "IWAD" signature to UART
4. Shrink the zone heap and run `doomgeneric_Create` → first menu screen
5. In-game FPS measurement and optimization (`-O2`, moving hot loops to SRAM)
