#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "gltab/gl_interno.h"
#include "log/log.h"

#if USAR_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H

#include "arq/arquivo.h"
#endif

namespace gl {

#if !USAR_FREETYPE
// GLUTES font data.
static const GLubyte Fixed8x13_Character_032[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* blank */
static const GLubyte Fixed8x13_Character_097[] = {  8,  0,  0,116,140,132,124,  4,120,  0,  0,  0,  0,  0}; /* "a" */
static const GLubyte Fixed8x13_Character_098[] = {  8,  0,  0,184,196,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_099[] = {  8,  0,  0,120,132,128,128,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_100[] = {  8,  0,  0,116,140,132,132,140,116,  4,  4,  4,  0,  0};
static const GLubyte Fixed8x13_Character_101[] = {  8,  0,  0,120,132,128,252,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_102[] = {  8,  0,  0, 64, 64, 64, 64,248, 64, 64, 68, 56,  0,  0};
static const GLubyte Fixed8x13_Character_103[] = {  8,120,132,120,128,112,136,136,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_104[] = {  8,  0,  0,132,132,132,132,196,184,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_105[] = {  8,  0,  0,248, 32, 32, 32, 32, 96,  0, 32,  0,  0,  0};
static const GLubyte Fixed8x13_Character_106[] = {  8,112,136,136,  8,  8,  8,  8, 24,  0,  8,  0,  0,  0};
static const GLubyte Fixed8x13_Character_107[] = {  8,  0,  0,132,136,144,224,144,136,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_108[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32, 96,  0,  0};
static const GLubyte Fixed8x13_Character_109[] = {  8,  0,  0,130,146,146,146,146,236,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_110[] = {  8,  0,  0,132,132,132,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_111[] = {  8,  0,  0,120,132,132,132,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_112[] = {  8,128,128,128,184,196,132,196,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_113[] = {  8,  4,  4,  4,116,140,132,140,116,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_114[] = {  8,  0,  0, 64, 64, 64, 64, 68,184,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_115[] = {  8,  0,  0,120,132, 24, 96,132,120,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_116[] = {  8,  0,  0, 56, 68, 64, 64, 64,248, 64, 64,  0,  0,  0};
static const GLubyte Fixed8x13_Character_117[] = {  8,  0,  0,116,136,136,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_119[] = {  8,  0,  0, 68,170,146,146,130,130,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_118[] = {  8,  0,  0, 32, 80, 80,136,136,136,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_120[] = {  8,  0,  0,132, 72, 48, 48, 72,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_121[] = {  8,120,132,  4,116,140,132,132,132,  0,  0,  0,  0,  0};
static const GLubyte Fixed8x13_Character_122[] = {  8,  0,  0,252, 64, 32, 16,  8,252,  0,  0,  0,  0,  0}; /* "z" */
static const GLubyte Fixed8x13_Character_065[] = {  8,  0,  0,132,132,132,252,132,132,132, 72, 48,  0,  0}; /* "A" */
static const GLubyte Fixed8x13_Character_066[] = {  8,  0,  0,252, 66, 66, 66,124, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_067[] = {  8,  0,  0,120,132,128,128,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_068[] = {  8,  0,  0,252, 66, 66, 66, 66, 66, 66, 66,252,  0,  0};
static const GLubyte Fixed8x13_Character_069[] = {  8,  0,  0,252,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_070[] = {  8,  0,  0,128,128,128,128,240,128,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_071[] = {  8,  0,  0,116,140,132,156,128,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_072[] = {  8,  0,  0,132,132,132,132,252,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_073[] = {  8,  0,  0,248, 32, 32, 32, 32, 32, 32, 32,248,  0,  0};
static const GLubyte Fixed8x13_Character_074[] = {  8,  0,  0,112,136,  8,  8,  8,  8,  8,  8, 60,  0,  0};
static const GLubyte Fixed8x13_Character_075[] = {  8,  0,  0,132,136,144,160,192,160,144,136,132,  0,  0};
static const GLubyte Fixed8x13_Character_076[] = {  8,  0,  0,252,128,128,128,128,128,128,128,128,  0,  0};
static const GLubyte Fixed8x13_Character_077[] = {  8,  0,  0,130,130,130,146,146,170,198,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_078[] = {  8,  0,  0,132,132,132,140,148,164,196,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_079[] = {  8,  0,  0,120,132,132,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_080[] = {  8,  0,  0,128,128,128,128,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_081[] = {  8,  0,  4,120,148,164,132,132,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_082[] = {  8,  0,  0,132,136,144,160,248,132,132,132,248,  0,  0};
static const GLubyte Fixed8x13_Character_083[] = {  8,  0,  0,120,132,  4,  4,120,128,128,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_084[] = {  8,  0,  0, 16, 16, 16, 16, 16, 16, 16, 16,254,  0,  0};
static const GLubyte Fixed8x13_Character_085[] = {  8,  0,  0,120,132,132,132,132,132,132,132,132,  0,  0};
static const GLubyte Fixed8x13_Character_087[] = {  8,  0,  0, 68,170,146,146,146,130,130,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_086[] = {  8,  0,  0, 16, 40, 40, 40, 68, 68, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_088[] = {  8,  0,  0,130,130, 68, 40, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_089[] = {  8,  0,  0, 16, 16, 16, 16, 16, 40, 68,130,130,  0,  0};
static const GLubyte Fixed8x13_Character_090[] = {  8,  0,  0,252,128,128, 64, 32, 16,  8,  4,252,  0,  0}; /* "Z" */
static const GLubyte Fixed8x13_Character_048[] = {  8,  0,  0, 48, 72,132,132,132,132,132, 72, 48,  0,  0}; /* "0" */
static const GLubyte Fixed8x13_Character_049[] = {  8,  0,  0,248, 32, 32, 32, 32, 32,160, 96, 32,  0,  0};
static const GLubyte Fixed8x13_Character_050[] = {  8,  0,  0,252,128, 64, 48,  8,  4,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_051[] = {  8,  0,  0,120,132,  4,  4, 56, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_052[] = {  8,  0,  0,  8,  8,252,136,136, 72, 40, 24,  8,  0,  0};
static const GLubyte Fixed8x13_Character_053[] = {  8,  0,  0,120,132,  4,  4,196,184,128,128,252,  0,  0};
static const GLubyte Fixed8x13_Character_054[] = {  8,  0,  0,120,132,132,196,184,128,128, 64, 56,  0,  0};
static const GLubyte Fixed8x13_Character_055[] = {  8,  0,  0, 64, 64, 32, 32, 16, 16,  8,  4,252,  0,  0};
static const GLubyte Fixed8x13_Character_056[] = {  8,  0,  0,120,132,132,132,120,132,132,132,120,  0,  0};
static const GLubyte Fixed8x13_Character_057[] = {  8,  0,  0,112,  8,  4,  4,116,140,132,132,120,  0,  0}; /* "9" */
static const GLubyte Fixed8x13_Character_096[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 16, 96,224,  0,  0}; /* "`" */
static const GLubyte Fixed8x13_Character_126[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,168, 72,  0,  0}; /* "~" */
static const GLubyte Fixed8x13_Character_033[] = {  8,  0,  0,128,  0,128,128,128,128,128,128,128,  0,  0}; /* "!" */
static const GLubyte Fixed8x13_Character_064[] = {  8,  0,  0,120,128,148,172,164,156,132,132,120,  0,  0}; /* "@" */
static const GLubyte Fixed8x13_Character_035[] = {  8,  0,  0,  0, 72, 72,252, 72,252, 72, 72,  0,  0,  0}; /* "#" */
static const GLubyte Fixed8x13_Character_036[] = {  8,  0,  0,  0, 32,240, 40,112,160,120, 32,  0,  0,  0}; /* "$" */
static const GLubyte Fixed8x13_Character_037[] = {  8,  0,  0,136, 84, 72, 32, 16, 16, 72,164, 68,  0,  0}; /* "%" */
static const GLubyte Fixed8x13_Character_094[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,136, 80, 32,  0,  0}; /* "^" */
static const GLubyte Fixed8x13_Character_038[] = {  8,  0,  0,116,136,148, 96,144,144, 96,  0,  0,  0,  0}; /* "&" */
static const GLubyte Fixed8x13_Character_042[] = {  8,  0,  0,  0,  0, 72, 48,252, 48, 72,  0,  0,  0,  0}; /* "*" */
static const GLubyte Fixed8x13_Character_040[] = {  8,  0,  0, 32, 64, 64,128,128,128, 64, 64, 32,  0,  0}; /* "(" */
static const GLubyte Fixed8x13_Character_041[] = {  8,  0,  0,128, 64, 64, 32, 32, 32, 64, 64,128,  0,  0}; /* ")" */
static const GLubyte Fixed8x13_Character_045[] = {  8,  0,  0,  0,  0,  0,  0,252,  0,  0,  0,  0,  0,  0}; /* "-" */
static const GLubyte Fixed8x13_Character_095[] = {  8,  0,254,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "_" */
static const GLubyte Fixed8x13_Character_061[] = {  8,  0,  0,  0,  0,252,  0,  0,252,  0,  0,  0,  0,  0}; /* "=" */
static const GLubyte Fixed8x13_Character_043[] = {  8,  0,  0,  0,  0, 32, 32,248, 32, 32,  0,  0,  0,  0}; /* "+" */
static const GLubyte Fixed8x13_Character_091[] = {  8,  0,  0,240,128,128,128,128,128,128,128,240,  0,  0}; /* "[" */
static const GLubyte Fixed8x13_Character_123[] = {  8,  0,  0, 56, 64, 64, 32,192, 32, 64, 64, 56,  0,  0}; /* "{" */
static const GLubyte Fixed8x13_Character_125[] = {  8,  0,  0,224, 16, 16, 32, 24, 32, 16, 16,224,  0,  0}; /* "}" */
static const GLubyte Fixed8x13_Character_093[] = {  8,  0,  0,240, 16, 16, 16, 16, 16, 16, 16,240,  0,  0}; /* "]" */
static const GLubyte Fixed8x13_Character_059[] = {  8,  0,128, 96,112,  0,  0, 32,112, 32,  0,  0,  0,  0}; /* ";" */
static const GLubyte Fixed8x13_Character_058[] = {  8,  0, 64,224, 64,  0,  0, 64,224, 64,  0,  0,  0,  0}; /* ":" */
static const GLubyte Fixed8x13_Character_044[] = {  8,  0,128, 96,112,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "," */
static const GLubyte Fixed8x13_Character_046[] = {  8,  0, 64,224, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0}; /* "." */
static const GLubyte Fixed8x13_Character_060[] = {  8,  0,  0,  8, 16, 32, 64,128, 64, 32, 16,  8,  0,  0}; /* "<" */
static const GLubyte Fixed8x13_Character_062[] = {  8,  0,  0,128, 64, 32, 16,  8, 16, 32, 64,128,  0,  0}; /* ">" */
static const GLubyte Fixed8x13_Character_047[] = {  8,  0,  0,128,128, 64, 32, 16,  8,  4,  2,  2,  0,  0}; /* "/" */
static const GLubyte Fixed8x13_Character_063[] = {  8,  0,  0, 16,  0, 16, 16,  8,  4,132,132,120,  0,  0}; /* "?" */
static const GLubyte Fixed8x13_Character_092[] = {  8,  0,  0,  2,  2,  4,  8, 16, 32, 64,128,128,  0,  0}; /* "\" */
static const GLubyte Fixed8x13_Character_034[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0,144,144,144,  0,  0}; /* """ */
/* Missing Characters filled in by John Fay by hand ... */
static const GLubyte Fixed8x13_Character_039[] = {  8,  0,  0,  0,  0,  0,  0,  0,  0, 32, 32, 32,  0,  0}; /* """ */
static const GLubyte Fixed8x13_Character_124[] = {  8, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,  0,  0}; /* """ */


/* The font characters mapping: */
static const GLubyte* Fixed8x13_Character_Map[] = {
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_032,
  Fixed8x13_Character_033,
  Fixed8x13_Character_034,
  Fixed8x13_Character_035,
  Fixed8x13_Character_036,
  Fixed8x13_Character_037,
  Fixed8x13_Character_038,
  Fixed8x13_Character_039,
  Fixed8x13_Character_040,
  Fixed8x13_Character_041,
  Fixed8x13_Character_042,
  Fixed8x13_Character_043,
  Fixed8x13_Character_044,
  Fixed8x13_Character_045,
  Fixed8x13_Character_046,
  Fixed8x13_Character_047,
  Fixed8x13_Character_048,
  Fixed8x13_Character_049,
  Fixed8x13_Character_050,
  Fixed8x13_Character_051,
  Fixed8x13_Character_052,
  Fixed8x13_Character_053,
  Fixed8x13_Character_054,
  Fixed8x13_Character_055,
  Fixed8x13_Character_056,
  Fixed8x13_Character_057,
  Fixed8x13_Character_058,
  Fixed8x13_Character_059,
  Fixed8x13_Character_060,
  Fixed8x13_Character_061,
  Fixed8x13_Character_062,
  Fixed8x13_Character_063,
  Fixed8x13_Character_064,
  Fixed8x13_Character_065,
  Fixed8x13_Character_066,
  Fixed8x13_Character_067,
  Fixed8x13_Character_068,
  Fixed8x13_Character_069,
  Fixed8x13_Character_070,
  Fixed8x13_Character_071,
  Fixed8x13_Character_072,
  Fixed8x13_Character_073,
  Fixed8x13_Character_074,
  Fixed8x13_Character_075,
  Fixed8x13_Character_076,
  Fixed8x13_Character_077,
  Fixed8x13_Character_078,
  Fixed8x13_Character_079,
  Fixed8x13_Character_080,
  Fixed8x13_Character_081,
  Fixed8x13_Character_082,
  Fixed8x13_Character_083,
  Fixed8x13_Character_084,
  Fixed8x13_Character_085,
  Fixed8x13_Character_086,
  Fixed8x13_Character_087,
  Fixed8x13_Character_088,
  Fixed8x13_Character_089,
  Fixed8x13_Character_090,
  Fixed8x13_Character_091,
  Fixed8x13_Character_092,
  Fixed8x13_Character_093,
  Fixed8x13_Character_094,
  Fixed8x13_Character_095,
  Fixed8x13_Character_096,
  Fixed8x13_Character_097,
  Fixed8x13_Character_098,
  Fixed8x13_Character_099,
  Fixed8x13_Character_100,
  Fixed8x13_Character_101,
  Fixed8x13_Character_102,
  Fixed8x13_Character_103,
  Fixed8x13_Character_104,
  Fixed8x13_Character_105,
  Fixed8x13_Character_106,
  Fixed8x13_Character_107,
  Fixed8x13_Character_108,
  Fixed8x13_Character_109,
  Fixed8x13_Character_110,
  Fixed8x13_Character_111,
  Fixed8x13_Character_112,
  Fixed8x13_Character_113,
  Fixed8x13_Character_114,
  Fixed8x13_Character_115,
  Fixed8x13_Character_116,
  Fixed8x13_Character_117,
  Fixed8x13_Character_118,
  Fixed8x13_Character_119,
  Fixed8x13_Character_120,
  Fixed8x13_Character_121,
  Fixed8x13_Character_122,
  Fixed8x13_Character_123,
  Fixed8x13_Character_124,
  Fixed8x13_Character_125,
  Fixed8x13_Character_126,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  Fixed8x13_Character_042,
  NULL};
#if 0
void DesenhaCaractere(char c) {
  const auto& caractere_it = g_indices_caracteres.find(c);
  if (caractere_it == g_indices_caracteres.end()) {
    return;
  }
  gl::MatrizEscopo salva_matriz(false);
  if (g_caracteres_baixos.find(caractere_it->first) != g_caracteres_baixos.end()) {
    gl::Translada(0.0f, -0.3f, 0.0f, false);
  }
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, &g_vertices_caracteres[0]);
  gl::DesenhaElementos(GL_TRIANGLES, caractere_it->second.size(), GL_UNSIGNED_SHORT, &caractere_it->second[0]);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}
#endif
// glutes.
int BitsToIndexedShorts(const GLubyte *bits, int ssx, int ssy, GLshort *points, int startx, int starty) {
  int ssxsy = ssx * ssy; // total bit #
  int x = 0;
  int bit = 7, count = 0;

  while (ssxsy--) {
    if (bits[0] & (1 << bit)) {
      points[0] = startx + x;
      points[1] = starty;
      count++;
      points += 2;
    }
    if (--bit == -1) {
      bit = 7;
      bits++;
    }
    if (++x >= ssx) {
      x = 0;
      starty++;
    }
  }
  return count;
}

namespace {
// Cada face tem seus dados pre computados.
struct FaceInfo {
  GLushort indices[64 * 64];
  std::vector<GLshort> points;
  int nbpoints;
};
FaceInfo g_face_infos[255];

}  // namespace

namespace interno {
void IniciaChar() {
  for (int i = 0; i < 255; ++i) {
    const GLubyte* face = Fixed8x13_Character_Map[i];
    g_face_infos[i].points.resize(13 * face[0] * 2);
    g_face_infos[i].nbpoints = BitsToIndexedShorts(face + 1, face[0], 13, &g_face_infos[i].points[0], 0, 0);
    for (int j = 0; j < g_face_infos[i].nbpoints; j++) {
      g_face_infos[i].indices[j] = j;
    }
  }
}
}  // namespace interno.

// glutes.
void BitmapCharacter(int character) {
  if (!(character >= 1) && (character < 256)) return;

  const auto& face_info = g_face_infos[character - 1];

  //glEnable(GL_ALPHA_TEST);
  //glAlphaFunc(GL_NOTEQUAL, 0);
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_SHORT, 0, face_info.points.data());
  gl::DesenhaElementos(GL_POINTS, face_info.nbpoints, GL_UNSIGNED_SHORT, face_info.indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
  //glDisable(GL_ALPHA_TEST);
}

void DesenhaCaractere(char c) {
  BitmapCharacter((int)c);
}

namespace {

std::vector<VboNaoGravado> VbosTabelados() {
  std::vector<VboNaoGravado> tabela(256);
  for (int i = 0; i < 256; ++i) {
    VboNaoGravado& vbo_char = tabela[i];
		int character = (i >= 1) && (i < 256) ? i : ' ';
    const auto& face_info = g_face_infos[character - 1];
    vbo_char.AtribuiCoordenadas(/*num_dimensoes=*/2, face_info.points);
    vbo_char.AtribuiIndices(face_info.indices, face_info.nbpoints);
  }
  return tabela;
}

}  // namespace

const VboNaoGravado& VboCaractere(int character) {
  static std::vector<VboNaoGravado> vbos_tabelados = VbosTabelados();
  return vbos_tabelados[character];
}

#else  // USAR_FREETYPE

#define VERSAO_PONTOS 1
namespace {
struct FaceInfo {
  std::vector<GLshort> vertices;
  std::vector<GLushort> indices;
  GLuint textura;
};

FaceInfo g_face_infos[256];
gl::VboNaoGravado g_vbot;

#if VERSAO_PONTOS
std::vector<GLshort> BitsParaPosicoes(const GLubyte *bits, int tam_x, int tam_y, bool print, int delta_y) {
  std::vector<GLshort> pontos;
  int total = tam_x * tam_y; // total bit #
  int x = 0, y = -delta_y;
  int bit = 7;

  if (print) {
    std::cout << "0x" << std::hex << (int)*bits << ": " << std::flush;
  }

  while (total--) {
    if (bits[0] & (1 << bit)) {
      pontos.push_back(x);
      pontos.push_back(tam_y - y);
      if (print) {
        std::cout << "[" << x << ", " << (tam_y - y) << "]; " << std::flush;
      }
    }
    if (--bit == -1) {
      bit = 7;
      bits++;
      if (print && total > 0) {
        std::cout << std::endl << "0x" << std::hex << (int)*bits << ": " << std::flush;
      }
    }
    if (++x >= tam_x) {
      if (tam_x % 8 != 0) {
        bits++;
        bit = 7;
        if (print && total > 0) {
          std::cout << std::endl << "0x" << std::hex << (int)*bits << ": " << std::flush;
        }
      }
      x = 0;
      y++;
    }
  }
  if (print) {
    std::cout << std::endl;
  }

  return pontos;
}
#else
std::vector<unsigned char> BitsParaLuminancia(const GLubyte *bits, int tam_x, int tam_y) {
  std::vector<unsigned char> pontos;
  int total = tam_x * tam_y; // total bit #
  int bit = 7;

  while (total--) {
    if (bits[0] & (1 << bit)) {
      pontos.push_back(0xFF);
      pontos.push_back(0xFF);
      pontos.push_back(0xFF);
      pontos.push_back(0xFF);
    } else {
      pontos.push_back(0x00);
      pontos.push_back(0x00);
      pontos.push_back(0x00);
      pontos.push_back(0x00);
    }
    if (--bit == -1) {
      bit = 7;
      bits++;
    }
  }
  return pontos;
}

std::vector<unsigned char> ParaRgba(const GLubyte *input, int tam_x, int tam_y) {
  std::vector<unsigned char> pontos;
  for (int i = 0; i < tam_x * tam_y; ++i) {
    if (*input > 0) {
      pontos.push_back(0xFF);
      pontos.push_back(0xFF);
      pontos.push_back(0xFF);
      pontos.push_back(*input);
    } else {
      pontos.push_back(0x00);
      pontos.push_back(0x00);
      pontos.push_back(0x00);
      pontos.push_back(0x00);
    }
    ++input;
  }
  return pontos;
}
#endif
}  // namespace

namespace interno {
void IniciaChar() {
  //float tam_y = 13.0f;
#if !VERSAO_PONTOS
  float tam_x = 8.0f;

  float x_sobre_y = tam_x / tam_y;
  const unsigned short indices[] = { 0, 1, 2, 3, 4, 5 };
  const float coordenadas[] = {
    0,     0,
    tam_x, 0,
    tam_x, tam_y,
    0,     0,
    tam_x, tam_y,
    0,     tam_y,
  };
  const float coordenadas_texel[] = {
    0.0f, 1.0f,  // x1, y1
    x_sobre_y, 1.0f,  // x2, y1
    x_sobre_y, 0.0f,  // x2, y2
    0.0f, 1.0f,  // x1, y1
    x_sobre_y, 0.0f,  // x2, y2
    0.0f, 0.0f,  // x1, y2
  };
  g_vbot.AtribuiCoordenadas(2, coordenadas, sizeof(coordenadas) / sizeof(float));
  g_vbot.AtribuiTexturas(coordenadas_texel);
  g_vbot.AtribuiIndices(indices, sizeof(indices) / sizeof(unsigned short));
  g_vbot.Nomeia("chartex");
#endif

  FT_Error error = FT_Err_Ok;
  FT_Face m_face = 0;
  FT_Library m_library = 0;

  // For simplicity, the error handling is very rudimentary.
  error = FT_Init_FreeType(&m_library);
  if (error) {
    LOG(ERROR) << "Falha iniciando freetype";
    return;
  }
  std::string caminho_fonte = arq::Diretorio(arq::TIPO_FONTES) + "/mono.ttf";
  error = FT_New_Face(m_library, caminho_fonte.c_str(), 0, &m_face);
  if (error) {
    LOG(ERROR) << "Falha iniciando mono";
    return;
  }

#if VERSAO_PONTOS
  const int tam_text = 16;
#else
  const int tam_text = tam_y;
#endif
  error = FT_Set_Pixel_Sizes(m_face, tam_text, tam_text);
  if (error) {
    LOG(ERROR) << "Erro FT_Set_Pixel_Sizes";
    return;
  }

  for (int c = 0; c < 256; ++c) {
    auto glyph_index = FT_Get_Char_Index(m_face, c);
    error = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
    if (error) {
      LOG(ERROR) << "Erro com caractere " << c;
      continue;
    }
#if VERSAO_PONTOS
    int pitch = (tam_text / 8 + (tam_text % 8 ? 1 : 0));
    std::vector<unsigned char> buffer(tam_text * pitch);
    FT_Bitmap bitmap;
    bitmap.rows = tam_text;
    bitmap.width = tam_text;
    bitmap.pitch = pitch;
    bitmap.buffer = buffer.data();
    bitmap.pixel_mode = FT_PIXEL_MODE_MONO;
    LOG(INFO) << "char: " << (char)c
              << "height: " << m_face->glyph->metrics.height
              << ", hby: " <<  m_face->glyph->metrics.horiBearingY;
    int delta_y = m_face->glyph->metrics.height > m_face->glyph->metrics.horiBearingY
        ? m_face->glyph->metrics.height - m_face->glyph->metrics.horiBearingY : 0;
    float delta_percentage = delta_y / static_cast<float>(m_face->glyph->metrics.height);
    FT_Outline_Translate(&m_face->glyph->outline, 0, delta_y);
    error = FT_Outline_Get_Bitmap(m_library, &m_face->glyph->outline, &bitmap);
    if (error) {
      LOG(ERROR) << "Erro lendo bitmap de caractere " << c;
      continue;
    }
    if (c == 'y') {
      LOG(INFO) << "bitmap rows: " << bitmap.rows;
      LOG(INFO) << "bitmap width: " << bitmap.width;
      LOG(INFO) << "bitmap pitch: " << bitmap.pitch;
    }
    // Versao pontos.
    auto& face = g_face_infos[c];
    face.vertices = BitsParaPosicoes(buffer.data(), tam_text, tam_text, c == 'y', -tam_text * delta_percentage);
    for (unsigned int i = 0; i < face.vertices.size() / 2; ++i) {
      face.indices.push_back(i);
    }
#else
    std::vector<unsigned char> buffer(tam_text * tam_text);
    FT_Bitmap bitmap;
    bitmap.rows = tam_text;
    bitmap.width = tam_text;
    bitmap.pitch = tam_text;
    bitmap.buffer = buffer.data();
    bitmap.num_grays = 256;
    bitmap.pixel_mode = FT_PIXEL_MODE_GRAY;
    error = FT_Outline_Get_Bitmap(m_library, &m_face->glyph->outline, &bitmap);
    if (error) {
      LOG(ERROR) << "Erro lendo bitmap de caractere " << c;
      continue;
    }

    // Versao textura
    gl::GeraTexturas(1, &face.textura);
    gl::LigacaoComTextura(GL_TEXTURE_2D, face.textura);
    gl::HabilitaMipmapAniso(GL_TEXTURE_2D);
    V_ERRO("Ligacao");
    // Carrega a textura.
    //std::vector<unsigned char> dados = BitsParaLuminancia(buffer.data(), tam_x, tam_y);
    std::vector<unsigned char> dados = ParaRgba(buffer.data(), tam_text, tam_text);
    gl::ImagemTextura2d(GL_TEXTURE_2D,
                        0, GL_RGBA,
                        tam_text, tam_text,
                        0, GL_RGBA, GL_UNSIGNED_BYTE,
                        dados.data());
    V_ERRO("Imagem");
#if WIN32 || MAC_OSX || TARGET_OS_IPHONE || (__linux__ && !ANDROID)
    // TODO wrapper para outros...
    gl::GeraMipmap(GL_TEXTURE_2D);
#endif
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::DesabilitaMipmapAniso(GL_TEXTURE_2D);
    V_ERRO("CriaTexturaOpenGl id: " << face.textura);
#endif
  }
  FT_Done_Face(m_face);
  LOG(INFO) << "Fonte carregada";
}

void FinalizaChar() {}

}  // namespace interno.

void DesenhaCaractere(char character) {
  if (character < 1) return;
  const auto& face = g_face_infos[(int)character];
#if VERSAO_PONTOS
  if (face.vertices.empty()) return;
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_SHORT, 0, face.vertices.data());
  gl::DesenhaElementos(GL_POINTS, face.indices.size(), GL_UNSIGNED_SHORT, face.indices.data());
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
#else
  //gl::HabilitaEscopo blend_escopo(GL_BLEND);
  //gl::DesabilitaEscopo depth_escopo(GL_DEPTH_TEST);

  gl::HabilitaEscopo textura(GL_TEXTURE_2D);
  gl::LigacaoComTextura(GL_TEXTURE_2D, face.textura);
  //auto* c = interno::BuscaContexto();
  //gl::MudaCor(1.0f, 0.0f, 0.0f, 1.0f);
  gl::DesenhaVbo(g_vbot);
  //gl::Retangulo(g_vbot);
  //gl::Retangulo(100, 100, 500, 500);
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
#endif
}
#endif

}
