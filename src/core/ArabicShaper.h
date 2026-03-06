#pragma once
#include <Arduino.h>
#include <vector>

static const uint16_t arabicForms[][4] = {
    // ا  alef
    {0xFE8D, 0xFE8E, 0xFE8E, 0xFE8D},
    // ب  ba
    {0xFE8F, 0xFE90, 0xFE92, 0xFE91},
    // ت  ta
    {0xFE95, 0xFE96, 0xFE98, 0xFE97},
    // ث  tha
    {0xFE99, 0xFE9A, 0xFE9C, 0xFE9B},
    // ج  jeem
    {0xFE9D, 0xFE9E, 0xFEA0, 0xFE9F},
    // ح  ha
    {0xFEA1, 0xFEA2, 0xFEA4, 0xFEA3},
    // خ  kha
    {0xFEA5, 0xFEA6, 0xFEA8, 0xFEA7},
    // د  dal
    {0xFEA9, 0xFEAA, 0xFEAA, 0xFEA9},
    // ذ  dhal
    {0xFEAB, 0xFEAC, 0xFEAC, 0xFEAB},
    // ر  ra
    {0xFEAD, 0xFEAE, 0xFEAE, 0xFEAD},
    // ز  zain
    {0xFEAF, 0xFEB0, 0xFEB0, 0xFEAF},
    // س  seen
    {0xFEB1, 0xFEB2, 0xFEB4, 0xFEB3},
    // ش  sheen
    {0xFEB5, 0xFEB6, 0xFEB8, 0xFEB7},
    // ص  sad
    {0xFEB9, 0xFEBA, 0xFEBC, 0xFEBB},
    // ض  dad
    {0xFEBD, 0xFEBE, 0xFEC0, 0xFEBF},
    // ط  ta
    {0xFEC1, 0xFEC2, 0xFEC4, 0xFEC3},
    // ظ  zha
    {0xFEC5, 0xFEC6, 0xFEC8, 0xFEC7},
    // ع  ain
    {0xFEC9, 0xFECA, 0xFECC, 0xFECB},
    // غ  ghain
    {0xFECD, 0xFECE, 0xFED0, 0xFECF},
    // ف  fa
    {0xFED1, 0xFED2, 0xFED4, 0xFED3},
    // ق  qaf
    {0xFED5, 0xFED6, 0xFED8, 0xFED7},
    // ك  kaf
    {0xFED9, 0xFEDA, 0xFEDC, 0xFEDB},
    // ل  lam
    {0xFEDD, 0xFEDE, 0xFEE0, 0xFEDF},
    // م  meem
    {0xFEE1, 0xFEE2, 0xFEE4, 0xFEE3},
    // ن  noon
    {0xFEE5, 0xFEE6, 0xFEE8, 0xFEE7},
    // ه  ha
    {0xFEE9, 0xFEEA, 0xFEEC, 0xFEEB},
    // و  waw
    {0xFEED, 0xFEEE, 0xFEEE, 0xFEED},
    // ي  ya
    {0xFEF1, 0xFEF2, 0xFEF4, 0xFEF3},
    // ى  alef maqsura
    {0xFEEF, 0xFEF0, 0xFEF0, 0xFEEF},
    // ة  ta marbuta
    {0xFE93, 0xFE94, 0xFE94, 0xFE93},
    // لا  lam-alef (ligature)
    {0xFEFB, 0xFEFC, 0xFEFC, 0xFEFB},
};

static const uint16_t arabicBase[] = {
    0x0627, // ا  index 0
    0x0628, // ب  index 1
    0x062A, // ت  index 2
    0x062B, // ث  index 3
    0x062C, // ج  index 4
    0x062D, // ح  index 5
    0x062E, // خ  index 6
    0x062F, // د  index 7
    0x0630, // ذ  index 8
    0x0631, // ر  index 9
    0x0632, // ز  index 10
    0x0633, // س  index 11
    0x0634, // ش  index 12
    0x0635, // ص  index 13
    0x0636, // ض  index 14
    0x0637, // ط  index 15
    0x0638, // ظ  index 16
    0x0639, // ع  index 17
    0x063A, // غ  index 18
    0x0641, // ف  index 19
    0x0642, // ق  index 20
    0x0643, // ك  index 21
    0x0644, // ل  index 22
    0x0645, // م  index 23
    0x0646, // ن  index 24
    0x0647, // ه  index 25
    0x0648, // و  index 26
    0x064A, // ي  index 27
    0x0649, // ى  index 28
    0x0629, // ة  index 29
};

static const uint16_t nonJoining[] = {
    0x0627, // ا
    0x062F, // د
    0x0630, // ذ
    0x0631, // ر
    0x0632, // ز
    0x0648, // و
    0x0624, // ؤ
    0x0625, // إ
    0x0623, // أ
    0x0622, // آ
    0x0626, // ئ
    0x0629, // ة
    0x0649, // ى
};

inline int getArabicIndex(uint16_t ch) {
    for (int i = 0; i < sizeof(arabicBase) / sizeof(arabicBase[0]); i++) {
        if (arabicBase[i] == ch) return i;
    }
    return -1;
}

inline bool isNonJoining(uint16_t ch) {
    for (int i = 0; i < sizeof(nonJoining) / sizeof(nonJoining[0]); i++) {
        if (nonJoining[i] == ch) return true;
    }
    return false;
}

inline bool isArabic(uint16_t ch) { return (ch >= 0x0600 && ch <= 0x06FF); }

inline std::vector<uint16_t> utf8ToUnicode(const String &str) {
    std::vector<uint16_t> result;
    const uint8_t *p = (const uint8_t *)str.c_str();
    while (*p) {
        uint16_t ch;
        if (*p < 0x80) {
            ch = *p++;
        } else if ((*p & 0xE0) == 0xC0) {
            ch = (*p++ & 0x1F) << 6;
            ch |= (*p++ & 0x3F);
        } else if ((*p & 0xF0) == 0xE0) {
            ch = (*p++ & 0x0F) << 12;
            ch |= (*p++ & 0x3F) << 6;
            ch |= (*p++ & 0x3F);
        } else {
            p++;
            continue;
        }
        result.push_back(ch);
    }
    return result;
}

inline String unicodeToUtf8(uint16_t cp) {
    String s;
    if (cp < 0x80) {
        s += (char)cp;
    } else if (cp < 0x800) {
        s += (char)(0xC0 | (cp >> 6));
        s += (char)(0x80 | (cp & 0x3F));
    } else {
        s += (char)(0xE0 | (cp >> 12));
        s += (char)(0x80 | ((cp >> 6) & 0x3F));
        s += (char)(0x80 | (cp & 0x3F));
    }
    return s;
}

inline String shapeArabic(const String &input) {

    std::vector<uint16_t> chars = utf8ToUnicode(input);
    int n = chars.size();

    std::vector<uint16_t> shaped(n);
    for (int i = 0; i < n; i++) {
        uint16_t ch = chars[i];
        int idx = getArabicIndex(ch);
        if (idx < 0) {
            shaped[i] = ch;
            continue;
        }

        bool prevJoins = (i > 0) && isArabic(chars[i - 1]) && !isNonJoining(chars[i - 1]);
        bool nextJoins = (i < n - 1) && isArabic(chars[i + 1]);

        int form;
        if (prevJoins && nextJoins && !isNonJoining(ch)) form = 2;
        else if (prevJoins && !isNonJoining(ch)) form = 1;
        else if (nextJoins) form = 3;
        else form = 0;

        shaped[i] = arabicForms[idx][form];
    }

    String result;
    for (int i = n - 1; i >= 0; i--) { result += unicodeToUtf8(shaped[i]); }
    return result;
}
