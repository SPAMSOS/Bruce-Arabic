#ifndef __NRF24_MENU_H__
#define __NRF24_MENU_H__

#include <MenuItemInterface.h>

class NRF24Menu : public MenuItemInterface {
public:
    NRF24Menu() : MenuItemInterface("جهاز تشويش") {}

    void optionsMenu(void);
    void configMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.nrf; }
    String themePath() { return bruceConfig.theme.paths.nrf; }
};

#endif
