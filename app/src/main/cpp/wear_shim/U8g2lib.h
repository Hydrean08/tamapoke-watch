#pragma once

// Opaque font tag pointers so applyLangFont() can signal classic CP437 vs CJK UTF-8.
static const unsigned char wear_font_tag_ja = 1;
static const unsigned char wear_font_tag_ko = 2;

#define u8g2_font_unifont_t_japanese3 (&wear_font_tag_ja)
#define u8g2_font_unifont_t_korean2 (&wear_font_tag_ko)
