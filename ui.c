#define NCURSES_WIDECHAR 1
#define _XOPEN_SOURCE 700
#include "ui.h"
#include "beep.h"
#include <ncursesw/ncurses.h>
#include <wchar.h>
#include <locale.h>

void gui_main(struct chip8 *chip)
{
    const int deltatime = 1; // 1ms per redraw
    int rows, cols;
    bool ui_running = true;
    const wchar_t *tb = L"\u2588",
                  *t_ = L"\u2580",
                  *_b = L"\u2584";
    
    // Setup the terminal such that we can get all characters and reading a char doesn't block
    setlocale(LC_ALL, "");
    initscr();
    set_escdelay(0);
    raw();
    keypad(stdscr, TRUE);
    noecho();
    nodelay(stdscr, TRUE);

    beep_h(440, 1000);

    // Go away debug!
    chip->debug = false;

    // Mainloop
    while (ui_running) {
        curs_set(0); // Hide cursor

        u16 key_mask = gui_get_key_mask(deltatime);
        if (key_mask == 0xFFFF)
            ui_running = false;
        
        bool screen_update = chip_cycle(chip, key_mask, deltatime);
        if (screen_update) {
            getmaxyx(stdscr, rows, cols);
            if (rows > DISPLAY_H) rows = DISPLAY_H;
            if (cols > DISPLAY_W) cols = DISPLAY_W;
            erase();
            move(0, 0);

            printw("O------------------------------------------------------------------O\n");
            for (int y = 0; y < DISPLAY_H; y+=2) {
                printw("| ");
                for (int x = 0; x < cols; x++) {
                    bool top = DISPLAY_GET(chip->display, x, y);
                    bool bottom = DISPLAY_GET(chip->display, x, y+1);
                    if (top && bottom)
                        addwstr(tb);
                    else if (top && !bottom)
                        addwstr(t_);
                    else if (!top && bottom)
                        addwstr(_b);
                    else
                        printw(" ");
                }
                printw(" |\n");
            }
            printw("O------------------------------------------------------------------O");
        }

        refresh();
        napms(deltatime); // 1ms delay to throttle, also gives a consistent deltatime
    }

    endwin();
}

u16 gui_get_key_mask(u16 dt)
{
    static u64 t = 0;
    static u16 mask = 0;
    static u16 last_seen[16];

    // Read pressed keys
    int ch, key;
    while ((ch = getch()) != ERR) {
        // Hand-mapping the keys, I can't be bothered to attempt something cleverer
        switch (ch) {
            case '1': key = 0x1; break;
            case '2': key = 0x2; break;
            case '3': key = 0x3; break;
            case '4': key = 0xC; break;

            case 'q': key = 0x4; break;
            case 'w': key = 0x5; break;
            case 'e': key = 0x6; break;
            case 'r': key = 0xD; break;

            case 'a': key = 0x7; break;
            case 's': key = 0x8; break;
            case 'd': key = 0x9; break;
            case 'f': key = 0xE; break;

            case 'z': key = 0xA; break;
            case 'x': key = 0x0; break;
            case 'c': key = 0xB; break;
            case 'v': key = 0xF; break;

            default: return 0xFFFF;
        }

        if (key < 0)
            continue;

        mask |= (1 << key);
        last_seen[key] = t;
    }

    // Remove keys that haven't been pressed for a while
    for (int i = 0; i < 16; i++)
        if ((mask & (1 << i)) && (t - last_seen[i] > 40))
            mask &= ~(1 << i);

    t += dt;
    return mask;
}
