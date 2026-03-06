#include "ConfigMenu.h"
#include "core/display.h"
#include "core/i2c_finder.h"
#include "core/main_menu.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#ifdef HAS_RGB_LED
#include "core/led_control.h"
#endif

/*********************************************************************
**  Function: optionsMenu
**  Main Config menu entry point
**********************************************************************/
void ConfigMenu::optionsMenu() {
    returnToMenu = false;
    while (true) {
        // Check if we need to exit to Main Menu (e.g., DevMode disabled)
        if (returnToMenu) {
            returnToMenu = false; // Reset flag
            return;
        }

        std::vector<Option> localOptions = {
            {"عرض & واجهة المستخدم", [this]() { displayUIMenu(); }},
#ifdef HAS_RGB_LED
            {"إعدادات DEL",          [this]() { ledMenu(); }      },
#endif
            {"إعدادات الصوت",        [this]() { audioMenu(); }    },
            {"إعدادات النظام",       [this]() { systemMenu(); }   },
            {"إعدادات الطاقة",       [this]() { powerMenu(); }    },
        };
#if !defined(LITE_VERSION)
        if (!appStoreInstalled()) {
            localOptions.push_back({"تثبيت متجر التطبيقات", []() { installAppStoreJS(); }});
        }
#endif

        if (bruceConfig.devMode) {
            localOptions.push_back({"وضع التطوير", [this]() { devMenu(); }});
        }

        localOptions.push_back({"معلومات الجهاز", showDeviceInfo});
        localOptions.push_back({"القائمة الرئيسية", []() {}});

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "config Menu");

        // Exit to Main Menu only if user pressed Back
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Otherwise rebuild Config menu after submenu returns
    }
}

/*********************************************************************
**  Function: displayUIMenu
**  Display & UI configuration submenu with auto-rebuild
**********************************************************************/
void ConfigMenu::displayUIMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"السطوع",      [this]() { setBrightnessMenu(); }               },
            {"وقت الظلام",  [this]() { setDimmerTimeMenu(); }               },
            {"التوجه",      [this]() { lambdaHelper(gsetRotation, true)(); }},
            {"لون الواجهة", [this]() { setUIColor(); }                      },
            {"ثيم الواجهة", [this]() { setTheme(); }                        },
            {"العودة",      []() {}                                         },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Display & UI");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Otherwise loop continues and menu rebuilds
    }
}

/*********************************************************************
**  Function: ledMenu
**  LED configuration submenu with auto-rebuild for toggles
**********************************************************************/
#ifdef HAS_RGB_LED
void ConfigMenu::ledMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"لون DEL",
             [this]() {
                 beginLed();
                 setLedColorConfig();
             }                                                                                },
            {" تأثير DEL",
             [this]() {
                 beginLed();
                 setLedEffectConfig();
             }                                                                                },
            {"سطوع DEL",
             [this]() {
                 beginLed();
                 setLedBrightnessConfig();
             }                                                                                },
            {String("وميض DEL: ") + (bruceConfig.ledBlinkEnabled ? "تشغيل" : "إيقاف"),
             [this]() {
                 // Toggle LED blink setting
                 bruceConfig.ledBlinkEnabled = !bruceConfig.ledBlinkEnabled;
                 bruceConfig.saveFile();
             }                                                                                },
            {"العودة",                                                                 []() {}},
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "LED Config");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds to update toggle label
    }
}
#endif
/*********************************************************************
**  Function: audioMenu
**  Audio configuration submenu with auto-rebuild for toggles
**********************************************************************/
void ConfigMenu::audioMenu() {
    while (true) {
        std::vector<Option> localOptions = {
#if !defined(LITE_VERSION)
#if defined(BUZZ_PIN) || defined(HAS_NS4168_SPKR)

            {String("الصوت: ") + (bruceConfig.soundEnabled ? "تشغيل" : "إيقاف"),
                                                             [this]() {
                 // Toggle sound setting
                 bruceConfig.soundEnabled = !bruceConfig.soundEnabled;
                 bruceConfig.saveFile();
             }                                                                                                                                                 },
#if defined(HAS_NS4168_SPKR)
            {"مستوى الصوت",                                                      [this]() { setSoundVolume(); }},
#endif  // BUZZ_PIN || HAS_NS4168_SPKR
#endif  //  HAS_NS4168_SPKR
#endif  //  LITE_VERSION
            {"العودة",                                                           []() {}                       },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Audio Config");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds to update toggle label
    }
}

/*********************************************************************
**  Function: systemMenu
**  System configuration submenu with auto-rebuild for toggles
**********************************************************************/
void ConfigMenu::systemMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {String("التشغيل السريع: ") + (bruceConfig.instantBoot ? "تشغيل" : "إيقاف"),
             [this]() {
                 // Toggle InstaBoot setting
                 bruceConfig.instantBoot = !bruceConfig.instantBoot;
                 bruceConfig.saveFile();
             }                                                                                                                       },
            {String("واي فاي عند التشغيل: ") + (bruceConfig.wifiAtStartup ? "تشغيل" : "إيقاف"),
             [this]() {
                 // Toggle WiFi at startup setting
                 bruceConfig.wifiAtStartup = !bruceConfig.wifiAtStartup;
                 bruceConfig.saveFile();
             }                                                                                                                       },
            {"تطبيق البدء",                                                                     [this]() { setStartupApp(); }        },
            {"إخفاء/إظهار التطبيقات",                                                           [this]() { mainMenu.hideAppsMenu(); }},
            {"الساعة",                                                                          [this]() { setClock(); }             },
            {"متقدم",                                                                           [this]() { advancedMenu(); }         },
            {"العودة",                                                                          []() {}                              },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "system Config");

        // Exit only if user pressed Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds to update toggle labels
    }
}

/*********************************************************************
**  Function: advancedMenu
**  Advanced settings submenu (nested under System Config)
**********************************************************************/
void ConfigMenu::advancedMenu() {
    while (true) {
        std::vector<Option> localOptions = {
#if !defined(LITE_VERSION)
            {"تفعيل بلوتوث IPA",     [this]() { enableBLEAPI(); }       },
            {"يو اس بي قاتل/بلوتوث", [this]() { setBadUSBBLEMenu(); }   },
#endif
            {"بيانات الشبكة",        [this]() { setNetworkCredsMenu(); }},
            {"إعادة ضبط المصنع",
                                      []() {
                 // Confirmation dialog for destructive action
                 drawMainBorder(true);
                 int8_t choice = displayMessage(
                     "هل أنت متأكد من رغبتك في إعادة ضبط المصنع?\nسيتم فقد جميع البيانات!",
                     "لا",
                     nullptr,
                     "نعم",
                     TFT_RED
                 );

                 if (choice == 1) {
                     // User confirmed - perform factory reset
                     bruceConfigPins.factoryReset();
                     bruceConfig.factoryReset(); // Restarts ESP
                 }
                 // If cancelled, loop continues and menu rebuilds
             }                                                                                   },
            {"العودة",               []() {}                            },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Advanced");

        // Exit to System Config menu
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds after each action
    }
}
/*********************************************************************
**  Function: powerMenu
**  Power management submenu with auto-rebuild
**********************************************************************/
void ConfigMenu::powerMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"سبات عميق",   goToDeepSleep          },
            {"سبات",        setSleepMode           },
            {"إعادة تشغيل", []() { ESP.restart(); }},
            {"إيقاف تشغيل",
             []() {
                 // Confirmation dialog for power off
                 drawMainBorder(true);
                 int8_t choice = displayMessage("هل تريد إيقاف تشغيل الجهاز?", "لا", nullptr, "نعم", TFT_RED);

                 if (choice == 1) { powerOff(); }
             }                                     },
            {"العودة",      []() {}                },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "Power Menu");

        // Exit to Config menu
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds after each action
    }
}

/*********************************************************************
**  Function: devMenu
**  Developer mode menu for advanced hardware configuration
**********************************************************************/
void ConfigMenu::devMenu() {
    while (true) {
        std::vector<Option> localOptions = {
            {"باحث C2I",         [this]() { find_i2c_addresses(); }                      },
            {"اطراف 1011CC ",    [this]() { setSPIPinsMenu(bruceConfigPins.CC1101_bus); }},
            {"أطراف 42FRN",      [this]() { setSPIPinsMenu(bruceConfigPins.NRF24_bus); } },
#if !defined(LITE_VERSION)
            {"أطراف لورا",       [this]() { setSPIPinsMenu(bruceConfigPins.LoRa_bus); }  },
            {"أطراف 0055W",      [this]() { setSPIPinsMenu(bruceConfigPins.W5500_bus); } },
#endif
            {"أطراف DS",         [this]() { setSPIPinsMenu(bruceConfigPins.SDCARD_bus); }},
            {"أطراف C2I",        [this]() { setI2CPinsMenu(bruceConfigPins.i2c_bus); }   },
            {"أطراف TRAU",       [this]() { setUARTPinsMenu(bruceConfigPins.uart_bus); } },
            {"أطراف SPG",        [this]() { setUARTPinsMenu(bruceConfigPins.gps_bus); }  },
            {"سيريل BSU",        [this]() { switchToUSBSerial(); }                       },
            {"سيريل TRAU",       [this]() { switchToUARTSerial(); }                      },
            {"تعطيل وضع المطور", [this]() { bruceConfig.setDevMode(false); }             },
            {"العودة",           []() {}                                                 },
        };

        int selected = loopOptions(localOptions, MENU_TYPE_SUBMENU, "DevMode");

        // Check if "Disable DevMode" was pressed (second-to-last option)
        if (selected == localOptions.size() - 2) {
            returnToMenu = true; // Signal to exit all Config menus
            return;
        }

        // Exit to Config menu on Back or ESC
        if (selected == -1 || selected == localOptions.size() - 1) { return; }
        // Menu rebuilds after each action
    }
}

/*********************************************************************
**  Function: switchToUSBSerial
**  Switch serial output to USB Serial
**********************************************************************/
void ConfigMenu::switchToUSBSerial() {
    USBserial.setSerialOutput(&Serial);
    Serial1.end();
}

/*********************************************************************
**  Function: switchToUARTSerial
**  Switch serial output to UART (handles pin conflicts)
**********************************************************************/
void ConfigMenu::switchToUARTSerial() {
    // Check and resolve SD card pin conflicts
    if (bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.SDCARD_bus.checkConflict(bruceConfigPins.uart_bus.tx)) {
        sdcardSPI.end();
    }

    // Check and resolve CC1101/NRF24 pin conflicts
    if (bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.CC1101_bus.checkConflict(bruceConfigPins.uart_bus.tx) ||
        bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.uart_bus.rx) ||
        bruceConfigPins.NRF24_bus.checkConflict(bruceConfigPins.uart_bus.tx)) {
        CC_NRF_SPI.end();
    }

    // Configure UART pins and switch serial output
    pinMode(bruceConfigPins.uart_bus.rx, INPUT);
    pinMode(bruceConfigPins.uart_bus.tx, OUTPUT);
    Serial1.begin(115200, SERIAL_8N1, bruceConfigPins.uart_bus.rx, bruceConfigPins.uart_bus.tx);
    USBserial.setSerialOutput(&Serial1);
}
/*********************************************************************
**  Function: drawIcon
**  Draw config gear icon
**********************************************************************/
void ConfigMenu::drawIcon(float scale) {
    clearIconArea();
    int radius = scale * 9;

    // Draw 6 gear teeth segments
    for (int i = 0; i < 6; i++) {
        tft.drawArc(
            iconCenterX,
            iconCenterY,
            3.5 * radius,
            2 * radius,
            15 + 60 * i,
            45 + 60 * i,
            bruceConfig.priColor,
            bruceConfig.bgColor,
            true
        );
    }

    // Draw inner circle
    tft.drawArc(
        iconCenterX,
        iconCenterY,
        2.5 * radius,
        radius,
        0,
        360,
        bruceConfig.priColor,
        bruceConfig.bgColor,
        false
    );
}
