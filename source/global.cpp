#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <stdio.h>
#include <string>

#include "global.h"
#include "date.h"
#include "sys.h"
#include "menu.h"
#include "cart.h"
#include "sd_cia.h"
#include "titledata.h"
#include "backupmenu.h"
#include "savemenu.h"
#include "extmenu.h"
#include "titles.h"
#include "nand.h"
#include "hbfilter.h"
#include "extra.h"
#include "shared.h"

int state = states::STATE_MAINMENU, prevState = states::STATE_MAINMENU;
titleData *curTitle = NULL;

unsigned buff_size = 0x10000;
unsigned frameCount = 0;

sftd_font * font;

sf2d_texture * bar;

FS_Archive sdArch;

bool useTouch, _date, hbl = false;

bool devMode = false;

bool kill = false;

//default colors
u8 clearColor[3] = {0, 0, 0};
u8 selColor[3] = {0, 255, 0};
u8 unSelColor[3] = {128, 128, 128};

std::u32string nameEnterString = U"Enter a folder name. Press A when finished.";

//draws the bar shown up top
void drawTopBar(const std::u32string nfo)
{
    //the bar
    sf2d_draw_texture_scale(bar, 0, 0, 25, 1);

    //nfo is wide text, UTF32
    sftd_draw_wtext(font, 0, 0, RGBA8(0, 0, 0, 255), 12, (wchar_t *)nfo.data());

    //time
    sftd_draw_text(font, 360, 0, RGBA8(0, 0, 0, 255), 12, RetTime().c_str());
}

void handleState()
{
    switch(state)
    {
        case states::STATE_MAINMENU:
            mainMenu();
            break;
        case states::STATE_CARTMENU:
            cartManager();
            break;
        case states::STATE_CIAMENU:
            sdStartSelect();
            break;
        case states::STATE_BACKUPMENU:
            backupMenu();
            break;
        case states::STATE_SAVEMENU:
            showSaveMenu();
            break;
        case states::STATE_EXTMENU:
            showExtMenu();
            break;
        case states::STATE_NANDSELECT:
            nandStartSelect();
            break;
        case states::STATE_NANDBACKUP:
            nandBackup();
            break;
        case states::STATE_EXTRAS:
            extrasMenu();
            break;
        case states::STATE_SHARED:
            sharedExtManager();
            break;
        case states::STATE_SHAREDBACKUP:
            sharedBackupMenu();
            break;
    }
}

void killApp(u32 up)
{
    if(up & KEY_START)
        kill = true;
}

enum mMenuOpts
{
    cart,
    cia,
    sys,
    shared,
    ref,
    filter,
    extra,
    exit
};

static menu mMenu(136, 80, false, true);

void prepMain()
{
    mMenu.addItem("Cartridge");
    mMenu.addItem("SD/CIA");
    mMenu.addItem("System Titles");
    mMenu.addItem("Shared ExtData");
    mMenu.addItem("Refresh Games");
    mMenu.addItem("Download Filter");
    mMenu.addItem("Extras");
    mMenu.addItem("Exit");

    mMenu.autoVert();
}

void mainMenu()
{

    hidScanInput();

    u32 down = hidKeysDown();

    mMenu.handleInput(down, 0);
    if(down & KEY_A)
    {
        switch(mMenu.getSelected())
        {
            case mMenuOpts::cart:
                state = states::STATE_CARTMENU;
                break;
            case mMenuOpts::cia:
                state = states::STATE_CIAMENU;
                break;
            case mMenuOpts::sys:
                state = states::STATE_NANDSELECT;
                break;
            case mMenuOpts::shared:
                state = states::STATE_SHARED;
                break;
            case mMenuOpts::ref:
                remove("titles");
                sdTitlesInit();
                refreshed = true;
                break;
            case mMenuOpts::filter:
                remove("filter.txt");
                loadFilterList();
                break;
            case mMenuOpts::extra:
                state = STATE_EXTRAS;
                break;
            case mMenuOpts::exit:
                kill = true;
                break;
        }
    }

    killApp(down);

    sf2d_start_frame(GFX_TOP, GFX_LEFT);
        drawTopBar(U"JKSM - 7/23/2016");
        mMenu.draw();
    sf2d_end_frame();

    sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
    sf2d_end_frame();

    sf2d_swapbuffers();
}
