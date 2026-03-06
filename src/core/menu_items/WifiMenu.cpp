#include "WifiMenu.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "core/wifi/webInterface.h"
#include "core/wifi/wg.h"
#include "core/wifi/wifi_common.h"
#include "core/wifi/wifi_mac.h"
#include "modules/ethernet/ARPScanner.h"
#include "modules/wifi/ap_info.h"
#include "modules/wifi/clients.h"
#include "modules/wifi/evil_portal.h"
#include "modules/wifi/karma_attack.h"
#include "modules/wifi/responder.h"
#include "modules/wifi/scan_hosts.h"
#include "modules/wifi/sniffer.h"
#include "modules/wifi/wifi_atks.h"

#ifndef LITE_VERSION
#include "modules/pwnagotchi/pwnagotchi.h"
#include "modules/wifi/wifi_recover.h"
#endif

// #include "modules/reverseShell/reverseShell.h"
//  Developed by Fourier (github.com/9dl)
//  Use BruceC2 to interact with the reverse shell server
//  BruceC2: https://github.com/9dl/Bruce-C2
//  To use BruceC2:
//  1. Start Reverse Shell Mode in Bruce
//  2. Start BruceC2 and wait.
//  3. Visit 192.168.4.1 in your browser to access the web interface for shell executing.

// 32bit: https://github.com/9dl/Bruce-C2/releases/download/v1.0/BruceC2_windows_386.exe
// 64bit: https://github.com/9dl/Bruce-C2/releases/download/v1.0/BruceC2_windows_amd64.exe
#include "modules/wifi/tcp_utils.h"

// global toggle - controls whether scanNetworks includes hidden SSIDs
bool showHiddenNetworks = false;

void WifiMenu::optionsMenu() {
    returnToMenu = false;
    options.clear();
    if (isWebUIActive) {
        drawMainBorderWithTitle("وايفاي", true);
        padprintln("");
        padprintln("تشغيل وظيفة واي فاي قد يوقف واجهة الويب");
        padprintln("");
        padprintln("اختر : للمتابعة");
        padprintln("أي زر : للقائمة");
        while (1) {
            if (check(SelPress)) { break; }
            if (check(AnyKeyPress)) { return; }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
        options = {
            {"الاتصال بواي فاي", lambdaHelper(wifiConnectMenu, WIFI_STA)},
            {"تشغيل نقطة اتصال", [=]() {
                 wifiConnectMenu(WIFI_AP);
                 displayInfo("الرمز: " + bruceConfig.wifiAp.pwd, true);
             }},
        };
    }
    if (WiFi.getMode() != WIFI_MODE_NULL) { options.push_back({"إيقاف الواي فاي", wifiDisconnect}); }
    if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_APSTA) {
        options.push_back({"معلومات الشبكة", displayAPInfo});
    }
    options.push_back({"هجمات واي فاي", wifi_atk_menu});
    options.push_back({"البوابة الوهمية", [=]() {
                           if (isWebUIActive || server) {
                               stopWebUi();
                               wifiDisconnect();
                           }
                           EvilPortal();
                       }});
    // options.push_back({"ReverseShell", [=]()       { ReverseShell(); }});
#ifndef LITE_VERSION
    options.push_back({"استماع PCT", listenTcpPort});
    options.push_back({"عميل PCT", clientTCP});
    options.push_back({"تيل نت", telnet_setup});
    options.push_back({"بوابة HSS", lambdaHelper(ssh_setup, String(""))});
    options.push_back({"حاسة الشم", [this]() {
                           std::vector<Option> snifferOptions;
                           snifferOptions.push_back({"شم خام", sniffer_setup});
                           snifferOptions.push_back({"شم طلبات", karma_setup});
                           snifferOptions.push_back({"رجوع", [this]() { optionsMenu(); }});

                           loopOptions(snifferOptions, MENU_TYPE_SUBMENU, "حاسة الشم");
                       }});
    options.push_back({"بحث الأجهزة", [=]() {
                           bool doScan = true;
                           if (!wifiConnected) doScan = wifiConnectMenu();

                           if (doScan) {
                               esp_netif_t *esp_netinterface =
                                   esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
                               if (esp_netinterface == nullptr) {
                                   Serial.println("Failed to get netif handle");
                                   return;
                               }
                               ARPScanner{esp_netinterface};
                           }
                       }});
    options.push_back({"في بي ان", wg_setup});
    options.push_back({"مستجيب", responder});
    options.push_back({"بروس كوتشي", brucegotchi_start});
    options.push_back({"استرداد رمز الوايفاي", wifi_recover_menu});
#endif

    options.push_back({"اعدادات", [this]() { configMenu(); }});

    addOptionToMainMenu();

    loopOptions(options, MENU_TYPE_SUBMENU, "Wifi");

    options.clear();
}

void WifiMenu::configMenu() {
    std::vector<Option> wifiOptions;

    wifiOptions.push_back({"تغيير الماك ادريس", wifiMACMenu});
    wifiOptions.push_back({"اضافة الوايفاي شرير", addEvilWifiMenu});
    wifiOptions.push_back({"ازالة الوايفاي الشرير", removeEvilWifiMenu});

    // Evil Wifi Settings submenu (unchanged)
    wifiOptions.push_back({"اعدادات الوايفاي الشرير", [this]() {
                               std::vector<Option> evilOptions;

                               evilOptions.push_back({"وضع الرموز", setEvilPasswordMode});
                               evilOptions.push_back({"تغيير اسم / معلومات", setEvilEndpointCreds});
                               evilOptions.push_back({"قبول / تصريح معلومات ", setEvilAllowGetCreds});
                               evilOptions.push_back({"تسمية الشبكة", setEvilEndpointSsid});
                               evilOptions.push_back({"قبول / تصريح الشبكة", setEvilAllowSetSsid});
                               evilOptions.push_back({"عرض نقاط النهاية", setEvilAllowEndpointDisplay});
                               evilOptions.push_back({"رجوع", [this]() { configMenu(); }});
                               loopOptions(evilOptions, MENU_TYPE_SUBMENU, "اعدادات الوايفاي الشرير");
                           }});

    {

        String hidden__wifi_option = String("Hidden Networks:") + (showHiddenNetworks ? "ON" : "OFF");

        // construct Option explicitly using char* label
        Option opt(hidden__wifi_option.c_str(), [this]() {
            showHiddenNetworks = !showHiddenNetworks;
            displayInfo(String("الشبكات المخفية:") + (showHiddenNetworks ? "تشغيل" : "ايقاف"), true);
            configMenu();
        });

        wifiOptions.push_back(opt);
    }
    wifiOptions.push_back({"رجوع", [this]() { optionsMenu(); }});
    loopOptions(wifiOptions, MENU_TYPE_SUBMENU, "wifi Config");
}

void WifiMenu::drawIcon(float scale) {
    clearIconArea();
    int deltaY = scale * 20;
    int radius = scale * 6;

    tft.fillCircle(iconCenterX, iconCenterY + deltaY, radius, bruceConfig.priColor);
    tft.drawArc(
        iconCenterX,
        iconCenterY + deltaY,
        deltaY + radius,
        deltaY,
        130,
        230,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX,
        iconCenterY + deltaY,
        2 * deltaY + radius,
        2 * deltaY,
        130,
        230,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
