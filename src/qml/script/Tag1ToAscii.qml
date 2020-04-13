/**
 * \file Tag1ToAscii.qml
 * Transliterate contents of ID3v1 tag from Extended Latin to ASCII.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Apr 2020
 *
 * Copyright (C) 2020  Urs Fleisch
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import Kid3 1.1

Kid3Script {
  onRun: {
    /**
     * ASCII replacements for set of extended latin characters.
     * Extended using some Perl modules by Martin Mareš.
     * $conv = Text::Iconv->new('UTF-8', 'US-ASCII//TRANSLIT');
     * $t = $conv->convert(Encode::encode('utf-8', chr $code));
     */
    var replacements = [
          ' ',   // 160   NO-BREAK SPACE
          '!',   // 161 ¡ INVERTED EXCLAMATION MARK
          'c',   // 162 ¢ CENT SIGN
          'L',   // 163 £ POUND SIGN
          'o',   // 164 ¤ CURRENCY SIGN
          'Y',   // 165 ¥ YEN SIGN
          '|',   // 166 ¦ BROKEN BAR
          'S',   // 167 § SECTION SIGN
          '`',   // 168 ¨ DIAERESIS
          'c',   // 169 © COPYRIGHT SIGN
          'a',   // 170 ª FEMININE ORDINAL INDICATOR
          '<<',  // 171 « LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
          '-',   // 172 ¬ NOT SIGN
          '-',   // 173   SOFT HYPHEN
          'R',   // 174 ® REGISTERED SIGN
          '-',   // 175 ¯ MACRON
          'o',   // 176 ° DEGREE SIGN
          '+-',  // 177 ± PLUS-MINUS SIGN
          '2',   // 178 ² SUPERSCRIPT TWO
          '3',   // 179 ³ SUPERSCRIPT THREE
          '\'',  // 180 ´ ACUTE ACCENT
          'u',   // 181 µ MICRO SIGN
          'P',   // 182 ¶ PILCROW SIGN
          '.',   // 183 · MIDDLE DOT
          ',',   // 184 ¸ CEDILLA
          '1',   // 185 ¹ SUPERSCRIPT ONE
          'o',   // 186 º MASCULINE ORDINAL INDICATOR
          '>>',  // 187 » RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
          '1/4', // 188 ¼ VULGAR FRACTION ONE QUARTER
          '1/2', // 189 ½ VULGAR FRACTION ONE HALF
          '3/4', // 190 ¾ VULGAR FRACTION THREE QUARTERS
          '?',   // 191 ¿ INVERTED QUESTION MARK
          'A',   // 192 À LATIN CAPITAL LETTER A WITH GRAVE
          'A',   // 193 Á LATIN CAPITAL LETTER A WITH ACUTE
          'A',   // 194 Â LATIN CAPITAL LETTER A WITH CIRCUMFLEX
          'A',   // 195 Ã LATIN CAPITAL LETTER A WITH TILDE
          'Ae',  // 196 Ä LATIN CAPITAL LETTER A WITH DIAERESIS
          'A',   // 197 Å LATIN CAPITAL LETTER A WITH RING ABOVE
          'AE',  // 198 Æ LATIN CAPITAL LETTER AE
          'C',   // 199 Ç LATIN CAPITAL LETTER C WITH CEDILLA
          'E',   // 200 È LATIN CAPITAL LETTER E WITH GRAVE
          'E',   // 201 É LATIN CAPITAL LETTER E WITH ACUTE
          'E',   // 202 Ê LATIN CAPITAL LETTER E WITH CIRCUMFLEX
          'E',   // 203 Ë LATIN CAPITAL LETTER E WITH DIAERESIS
          'I',   // 204 Ì LATIN CAPITAL LETTER I WITH GRAVE
          'I',   // 205 Í LATIN CAPITAL LETTER I WITH ACUTE
          'I',   // 206 Î LATIN CAPITAL LETTER I WITH CIRCUMFLEX
          'I',   // 207 Ï LATIN CAPITAL LETTER I WITH DIAERESIS
          'D',   // 208 Ð LATIN CAPITAL LETTER ETH
          'N',   // 209 Ñ LATIN CAPITAL LETTER N WITH TILDE
          'O',   // 210 Ò LATIN CAPITAL LETTER O WITH GRAVE
          'O',   // 211 Ó LATIN CAPITAL LETTER O WITH ACUTE
          'O',   // 212 Ô LATIN CAPITAL LETTER O WITH CIRCUMFLEX
          'O',   // 213 Õ LATIN CAPITAL LETTER O WITH TILDE
          'Oe',  // 214 Ö LATIN CAPITAL LETTER O WITH DIAERESIS
          'x',   // 215 × MULTIPLICATION SIGN
          'O',   // 216 Ø LATIN CAPITAL LETTER O WITH STROKE
          'U',   // 217 Ù LATIN CAPITAL LETTER U WITH GRAVE
          'U',   // 218 Ú LATIN CAPITAL LETTER U WITH ACUTE
          'U',   // 219 Û LATIN CAPITAL LETTER U WITH CIRCUMFLEX
          'Ue',  // 220 Ü LATIN CAPITAL LETTER U WITH DIAERESIS
          'Y',   // 221 Ý LATIN CAPITAL LETTER Y WITH ACUTE
          'p',   // 222 Þ LATIN CAPITAL LETTER THORN
          'ss',  // 223 ß LATIN SMALL LETTER SHARP S
          'a',   // 224 à LATIN SMALL LETTER A WITH GRAVE
          'a',   // 225 á LATIN SMALL LETTER A WITH ACUTE
          'a',   // 226 â LATIN SMALL LETTER A WITH CIRCUMFLEX
          'a',   // 227 ã LATIN SMALL LETTER A WITH TILDE
          'ae',  // 228 ä LATIN SMALL LETTER A WITH DIAERESIS
          'a',   // 229 å LATIN SMALL LETTER A WITH RING ABOVE
          'ae',  // 230 æ LATIN SMALL LETTER AE
          'c',   // 231 ç LATIN SMALL LETTER C WITH CEDILLA
          'e',   // 232 è LATIN SMALL LETTER E WITH GRAVE
          'e',   // 233 é LATIN SMALL LETTER E WITH ACUTE
          'e',   // 234 ê LATIN SMALL LETTER E WITH CIRCUMFLEX
          'e',   // 235 ë LATIN SMALL LETTER E WITH DIAERESIS
          'i',   // 236 ì LATIN SMALL LETTER I WITH GRAVE
          'i',   // 237 í LATIN SMALL LETTER I WITH ACUTE
          'i',   // 238 î LATIN SMALL LETTER I WITH CIRCUMFLEX
          'i',   // 239 ï LATIN SMALL LETTER I WITH DIAERESIS
          'o',   // 240 ð LATIN SMALL LETTER ETH
          'n',   // 241 ñ LATIN SMALL LETTER N WITH TILDE
          'o',   // 242 ò LATIN SMALL LETTER O WITH GRAVE
          'o',   // 243 ó LATIN SMALL LETTER O WITH ACUTE
          'o',   // 244 ô LATIN SMALL LETTER O WITH CIRCUMFLEX
          'o',   // 245 õ LATIN SMALL LETTER O WITH TILDE
          'oe',  // 246 ö LATIN SMALL LETTER O WITH DIAERESIS
          '/',   // 247 ÷ DIVISION SIGN
          'o',   // 248 ø LATIN SMALL LETTER O WITH STROKE
          'u',   // 249 ù LATIN SMALL LETTER U WITH GRAVE
          'u',   // 250 ú LATIN SMALL LETTER U WITH ACUTE
          'u',   // 251 û LATIN SMALL LETTER U WITH CIRCUMFLEX
          'ue',  // 252 ü LATIN SMALL LETTER U WITH DIAERESIS
          'y',   // 253 ý LATIN SMALL LETTER Y WITH ACUTE
          'p',   // 254 þ LATIN SMALL LETTER THORN
          'y',   // 255 ÿ LATIN SMALL LETTER Y WITH DIAERESIS
          'A',   // 256 Ā LATIN CAPITAL LETTER A WITH MACRON
          'a',   // 257 ā LATIN SMALL LETTER A WITH MACRON
          'A',   // 258 Ă LATIN CAPITAL LETTER A WITH BREVE
          'a',   // 259 ă LATIN SMALL LETTER A WITH BREVE
          'A',   // 260 Ą LATIN CAPITAL LETTER A WITH OGONEK
          'a',   // 261 ą LATIN SMALL LETTER A WITH OGONEK
          'C',   // 262 Ć LATIN CAPITAL LETTER C WITH ACUTE
          'c',   // 263 ć LATIN SMALL LETTER C WITH ACUTE
          'C',   // 264 Ĉ LATIN CAPITAL LETTER C WITH CIRCUMFLEX
          'c',   // 265 ĉ LATIN SMALL LETTER C WITH CIRCUMFLEX
          'C',   // 266 Ċ LATIN CAPITAL LETTER C WITH DOT ABOVE
          'c',   // 267 ċ LATIN SMALL LETTER C WITH DOT ABOVE
          'C',   // 268 Č LATIN CAPITAL LETTER C WITH CARON
          'c',   // 269 č LATIN SMALL LETTER C WITH CARON
          'D',   // 270 Ď LATIN CAPITAL LETTER D WITH CARON
          'd',   // 271 ď LATIN SMALL LETTER D WITH CARON
          'D',   // 272 Đ LATIN CAPITAL LETTER D WITH STROKE
          'd',   // 273 đ LATIN SMALL LETTER D WITH STROKE
          'E',   // 274 Ē LATIN CAPITAL LETTER E WITH MACRON
          'e',   // 275 ē LATIN SMALL LETTER E WITH MACRON
          'E',   // 276 Ĕ LATIN CAPITAL LETTER E WITH BREVE
          'e',   // 277 ĕ LATIN SMALL LETTER E WITH BREVE
          'E',   // 278 Ė LATIN CAPITAL LETTER E WITH DOT ABOVE
          'e',   // 279 ė LATIN SMALL LETTER E WITH DOT ABOVE
          'E',   // 280 Ę LATIN CAPITAL LETTER E WITH OGONEK
          'e',   // 281 ę LATIN SMALL LETTER E WITH OGONEK
          'E',   // 282 Ě LATIN CAPITAL LETTER E WITH CARON
          'e',   // 283 ě LATIN SMALL LETTER E WITH CARON
          'G',   // 284 Ĝ LATIN CAPITAL LETTER G WITH CIRCUMFLEX
          'g',   // 285 ĝ LATIN SMALL LETTER G WITH CIRCUMFLEX
          'G',   // 286 Ğ LATIN CAPITAL LETTER G WITH BREVE
          'g',   // 287 ğ LATIN SMALL LETTER G WITH BREVE
          'G',   // 288 Ġ LATIN CAPITAL LETTER G WITH DOT ABOVE
          'g',   // 289 ġ LATIN SMALL LETTER G WITH DOT ABOVE
          'G',   // 290 Ģ LATIN CAPITAL LETTER G WITH CEDILLA
          'g',   // 291 ģ LATIN SMALL LETTER G WITH CEDILLA
          'H',   // 292 Ĥ LATIN CAPITAL LETTER H WITH CIRCUMFLEX
          'h',   // 293 ĥ LATIN SMALL LETTER H WITH CIRCUMFLEX
          'H',   // 294 Ħ LATIN CAPITAL LETTER H WITH STROKE
          'h',   // 295 ħ LATIN SMALL LETTER H WITH STROKE
          'I',   // 296 Ĩ LATIN CAPITAL LETTER I WITH TILDE
          'i',   // 297 ĩ LATIN SMALL LETTER I WITH TILDE
          'I',   // 298 Ī LATIN CAPITAL LETTER I WITH MACRON
          'i',   // 299 ī LATIN SMALL LETTER I WITH MACRON
          'I',   // 300 Ĭ LATIN CAPITAL LETTER I WITH BREVE
          'i',   // 301 ĭ LATIN SMALL LETTER I WITH BREVE
          'I',   // 302 Į LATIN CAPITAL LETTER I WITH OGONEK
          'i',   // 303 į LATIN SMALL LETTER I WITH OGONEK
          'I',   // 304 İ LATIN CAPITAL LETTER I WITH DOT ABOVE
          'i',   // 305 ı LATIN SMALL LETTER DOTLESS I
          'IJ',  // 306 Ĳ LATIN CAPITAL LIGATURE IJ
          'ij',  // 307 ĳ LATIN SMALL LIGATURE IJ
          'J',   // 308 Ĵ LATIN CAPITAL LETTER J WITH CIRCUMFLEX
          'j',   // 309 ĵ LATIN SMALL LETTER J WITH CIRCUMFLEX
          'K',   // 310 Ķ LATIN CAPITAL LETTER K WITH CEDILLA
          'k',   // 311 ķ LATIN SMALL LETTER K WITH CEDILLA
          'q',   // 312 ĸ LATIN SMALL LETTER KRA
          'L',   // 313 Ĺ LATIN CAPITAL LETTER L WITH ACUTE
          'l',   // 314 ĺ LATIN SMALL LETTER L WITH ACUTE
          'L',   // 315 Ļ LATIN CAPITAL LETTER L WITH CEDILLA
          'l',   // 316 ļ LATIN SMALL LETTER L WITH CEDILLA
          'L',   // 317 Ľ LATIN CAPITAL LETTER L WITH CARON
          'l',   // 318 ľ LATIN SMALL LETTER L WITH CARON
          'L',   // 319 Ŀ LATIN CAPITAL LETTER L WITH MIDDLE DOT
          'l',   // 320 ŀ LATIN SMALL LETTER L WITH MIDDLE DOT
          'L',   // 321 Ł LATIN CAPITAL LETTER L WITH STROKE
          'l',   // 322 ł LATIN SMALL LETTER L WITH STROKE
          'N',   // 323 Ń LATIN CAPITAL LETTER N WITH ACUTE
          'n',   // 324 ń LATIN SMALL LETTER N WITH ACUTE
          'N',   // 325 Ņ LATIN CAPITAL LETTER N WITH CEDILLA
          'n',   // 326 ņ LATIN SMALL LETTER N WITH CEDILLA
          'N',   // 327 Ň LATIN CAPITAL LETTER N WITH CARON
          'n',   // 328 ň LATIN SMALL LETTER N WITH CARON
          '\'n', // 329 ŉ LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
          'N',   // 330 Ŋ LATIN CAPITAL LETTER ENG
          'n',   // 331 ŋ LATIN SMALL LETTER ENG
          'O',   // 332 Ō LATIN CAPITAL LETTER O WITH MACRON
          'o',   // 333 ō LATIN SMALL LETTER O WITH MACRON
          'O',   // 334 Ŏ LATIN CAPITAL LETTER O WITH BREVE
          'o',   // 335 ŏ LATIN SMALL LETTER O WITH BREVE
          'O',   // 336 Ő LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
          'o',   // 337 ő LATIN SMALL LETTER O WITH DOUBLE ACUTE
          'OE',  // 338 Œ LATIN CAPITAL LIGATURE OE
          'oe',  // 339 œ LATIN SMALL LIGATURE OE
          'R',   // 340 Ŕ LATIN CAPITAL LETTER R WITH ACUTE
          'r',   // 341 ŕ LATIN SMALL LETTER R WITH ACUTE
          'R',   // 342 Ŗ LATIN CAPITAL LETTER R WITH CEDILLA
          'r',   // 343 ŗ LATIN SMALL LETTER R WITH CEDILLA
          'R',   // 344 Ř LATIN CAPITAL LETTER R WITH CARON
          'r',   // 345 ř LATIN SMALL LETTER R WITH CARON
          'S',   // 346 Ś LATIN CAPITAL LETTER S WITH ACUTE
          's',   // 347 ś LATIN SMALL LETTER S WITH ACUTE
          'S',   // 348 Ŝ LATIN CAPITAL LETTER S WITH CIRCUMFLEX
          's',   // 349 ŝ LATIN SMALL LETTER S WITH CIRCUMFLEX
          'S',   // 350 Ş LATIN CAPITAL LETTER S WITH CEDILLA
          's',   // 351 ş LATIN SMALL LETTER S WITH CEDILLA
          'S',   // 352 Š LATIN CAPITAL LETTER S WITH CARON
          's',   // 353 š LATIN SMALL LETTER S WITH CARON
          'T',   // 354 Ţ LATIN CAPITAL LETTER T WITH CEDILLA
          't',   // 355 ţ LATIN SMALL LETTER T WITH CEDILLA
          'T',   // 356 Ť LATIN CAPITAL LETTER T WITH CARON
          't',   // 357 ť LATIN SMALL LETTER T WITH CARON
          'T',   // 358 Ŧ LATIN CAPITAL LETTER T WITH STROKE
          't',   // 359 ŧ LATIN SMALL LETTER T WITH STROKE
          'U',   // 360 Ũ LATIN CAPITAL LETTER U WITH TILDE
          'u',   // 361 ũ LATIN SMALL LETTER U WITH TILDE
          'U',   // 362 Ū LATIN CAPITAL LETTER U WITH MACRON
          'u',   // 363 ū LATIN SMALL LETTER U WITH MACRON
          'U',   // 364 Ŭ LATIN CAPITAL LETTER U WITH BREVE
          'u',   // 365 ŭ LATIN SMALL LETTER U WITH BREVE
          'U',   // 366 Ů LATIN CAPITAL LETTER U WITH RING ABOVE
          'u',   // 367 ů LATIN SMALL LETTER U WITH RING ABOVE
          'U',   // 368 Ű LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
          'u',   // 369 ű LATIN SMALL LETTER U WITH DOUBLE ACUTE
          'U',   // 370 Ų LATIN CAPITAL LETTER U WITH OGONEK
          'u',   // 371 ų LATIN SMALL LETTER U WITH OGONEK
          'W',   // 372 Ŵ LATIN CAPITAL LETTER W WITH CIRCUMFLEX
          'w',   // 373 ŵ LATIN SMALL LETTER W WITH CIRCUMFLEX
          'Y',   // 374 Ŷ LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
          'y',   // 375 ŷ LATIN SMALL LETTER Y WITH CIRCUMFLEX
          'Y',   // 376 Ÿ LATIN CAPITAL LETTER Y WITH DIAERESIS
          'Z',   // 377 Ź LATIN CAPITAL LETTER Z WITH ACUTE
          'z',   // 378 ź LATIN SMALL LETTER Z WITH ACUTE
          'Z',   // 379 Ż LATIN CAPITAL LETTER Z WITH DOT ABOVE
          'z',   // 380 ż LATIN SMALL LETTER Z WITH DOT ABOVE
          'Z',   // 381 Ž LATIN CAPITAL LETTER Z WITH CARON
          'z',   // 382 ž LATIN SMALL LETTER Z WITH CARON
          's',   // 383 ſ LATIN SMALL LETTER LONG S
          'b',   // 384 ƀ LATIN SMALL LETTER B WITH STROKE
          'B',   // 385 Ɓ LATIN CAPITAL LETTER B WITH HOOK
          'B',   // 386 Ƃ LATIN CAPITAL LETTER B WITH TOPBAR
          'b',   // 387 ƃ LATIN SMALL LETTER B WITH TOPBAR
          '6',   // 388 Ƅ LATIN CAPITAL LETTER TONE SIX
          '6',   // 389 ƅ LATIN SMALL LETTER TONE SIX
          'O',   // 390 Ɔ LATIN CAPITAL LETTER OPEN O
          'C',   // 391 Ƈ LATIN CAPITAL LETTER C WITH HOOK
          'c',   // 392 ƈ LATIN SMALL LETTER C WITH HOOK
          'D',   // 393 Ɖ LATIN CAPITAL LETTER AFRICAN D
          'D',   // 394 Ɗ LATIN CAPITAL LETTER D WITH HOOK
          'D',   // 395 Ƌ LATIN CAPITAL LETTER D WITH TOPBAR
          'd',   // 396 ƌ LATIN SMALL LETTER D WITH TOPBAR
          'd',   // 397 ƍ LATIN SMALL LETTER TURNED DELTA
          'E',   // 398 Ǝ LATIN CAPITAL LETTER REVERSED E
          'd',   // 399 Ə LATIN CAPITAL LETTER SCHWA
          'E',   // 400 Ɛ LATIN CAPITAL LETTER OPEN E
          'F',   // 401 Ƒ LATIN CAPITAL LETTER F WITH HOOK
          'f',   // 402 ƒ LATIN SMALL LETTER F WITH HOOK
          'G',   // 403 Ɠ LATIN CAPITAL LETTER G WITH HOOK
          'Y',   // 404 Ɣ LATIN CAPITAL LETTER GAMMA
          'hv',  // 405 ƕ LATIN SMALL LETTER HV
          'I',   // 406 Ɩ LATIN CAPITAL LETTER IOTA
          'I',   // 407 Ɨ LATIN CAPITAL LETTER I WITH STROKE
          'K',   // 408 Ƙ LATIN CAPITAL LETTER K WITH HOOK
          'k',   // 409 ƙ LATIN SMALL LETTER K WITH HOOK
          'l',   // 410 ƚ LATIN SMALL LETTER L WITH BAR
          'L',   // 411 ƛ LATIN SMALL LETTER LAMBDA WITH STROKE
          'W',   // 412 Ɯ LATIN CAPITAL LETTER TURNED M
          'N',   // 413 Ɲ LATIN CAPITAL LETTER N WITH LEFT HOOK
          'n',   // 414 ƞ LATIN SMALL LETTER N WITH LONG RIGHT LEG
          'O',   // 415 Ɵ LATIN CAPITAL LETTER O WITH MIDDLE TILDE
          'O',   // 416 Ơ LATIN CAPITAL LETTER O WITH HORN
          'o',   // 417 ơ LATIN SMALL LETTER O WITH HORN
          'OI',  // 418 Ƣ LATIN CAPITAL LETTER OI
          'oi',  // 419 ƣ LATIN SMALL LETTER OI
          'P',   // 420 Ƥ LATIN CAPITAL LETTER P WITH HOOK
          'p',   // 421 ƥ LATIN SMALL LETTER P WITH HOOK
          'R',   // 422 Ʀ LATIN LETTER YR
          '2',   // 423 Ƨ LATIN CAPITAL LETTER TONE TWO
          '2',   // 424 ƨ LATIN SMALL LETTER TONE TWO
          'E',   // 425 Ʃ LATIN CAPITAL LETTER ESH
          'E',   // 426 ƪ LATIN LETTER REVERSED ESH LOOP
          't',   // 427 ƫ LATIN SMALL LETTER T WITH PALATAL HOOK
          'T',   // 428 Ƭ LATIN CAPITAL LETTER T WITH HOOK
          't',   // 429 ƭ LATIN SMALL LETTER T WITH HOOK
          'T',   // 430 Ʈ LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
          'U',   // 431 Ư LATIN CAPITAL LETTER U WITH HORN
          'u',   // 432 ư LATIN SMALL LETTER U WITH HORN
          'U',   // 433 Ʊ LATIN CAPITAL LETTER UPSILON
          'V',   // 434 Ʋ LATIN CAPITAL LETTER V WITH HOOK
          'Y',   // 435 Ƴ LATIN CAPITAL LETTER Y WITH HOOK
          'y',   // 436 ƴ LATIN SMALL LETTER Y WITH HOOK
          'Z',   // 437 Ƶ LATIN CAPITAL LETTER Z WITH STROKE
          'z',   // 438 ƶ LATIN SMALL LETTER Z WITH STROKE
          '3',   // 439 Ʒ LATIN CAPITAL LETTER EZH
          '3',   // 440 Ƹ LATIN CAPITAL LETTER EZH REVERSED
          '3',   // 441 ƹ LATIN SMALL LETTER EZH REVERSED
          '3',   // 442 ƺ LATIN SMALL LETTER EZH WITH TAIL
          '2',   // 443 ƻ LATIN LETTER TWO WITH STROKE
          '5',   // 444 Ƽ LATIN CAPITAL LETTER TONE FIVE
          '5',   // 445 ƽ LATIN SMALL LETTER TONE FIVE
          's',   // 446 ƾ LATIN LETTER INVERTED GLOTTAL STOP WITH STROKE
          'p',   // 447 ƿ LATIN LETTER WYNN
          '|',   // 448 ǀ LATIN LETTER DENTAL CLICK
          '||',  // 449 ǁ LATIN LETTER LATERAL CLICK
          '#',   // 450 ǂ LATIN LETTER ALVEOLAR CLICK
          '!',   // 451 ǃ LATIN LETTER RETROFLEX CLICK
          'DZ',  // 452 Ǆ LATIN CAPITAL LETTER DZ WITH CARON
          'Dz',  // 453 ǅ LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
          'dz',  // 454 ǆ LATIN SMALL LETTER DZ WITH CARON
          'LJ',  // 455 Ǉ LATIN CAPITAL LETTER LJ
          'Lj',  // 456 ǈ LATIN CAPITAL LETTER L WITH SMALL LETTER J
          'lj',  // 457 ǉ LATIN SMALL LETTER LJ
          'NJ',  // 458 Ǌ LATIN CAPITAL LETTER NJ
          'Nj',  // 459 ǋ LATIN CAPITAL LETTER N WITH SMALL LETTER J
          'nj',  // 460 ǌ LATIN SMALL LETTER NJ
          'A',   // 461 Ǎ LATIN CAPITAL LETTER A WITH CARON
          'a',   // 462 ǎ LATIN SMALL LETTER A WITH CARON
          'I',   // 463 Ǐ LATIN CAPITAL LETTER I WITH CARON
          'i',   // 464 ǐ LATIN SMALL LETTER I WITH CARON
          'O',   // 465 Ǒ LATIN CAPITAL LETTER O WITH CARON
          'o',   // 466 ǒ LATIN SMALL LETTER O WITH CARON
          'U',   // 467 Ǔ LATIN CAPITAL LETTER U WITH CARON
          'u',   // 468 ǔ LATIN SMALL LETTER U WITH CARON
          'U',   // 469 Ǖ LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
          'u',   // 470 ǖ LATIN SMALL LETTER U WITH DIAERESIS AND MACRON
          'U',   // 471 Ǘ LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
          'u',   // 472 ǘ LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE
          'U',   // 473 Ǚ LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
          'u',   // 474 ǚ LATIN SMALL LETTER U WITH DIAERESIS AND CARON
          'U',   // 475 Ǜ LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
          'u',   // 476 ǜ LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE
          'e',   // 477 ǝ LATIN SMALL LETTER TURNED E
          'A',   // 478 Ǟ LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
          'a',   // 479 ǟ LATIN SMALL LETTER A WITH DIAERESIS AND MACRON
          'A',   // 480 Ǡ LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
          'a',   // 481 ǡ LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON
          'AE',  // 482 Ǣ LATIN CAPITAL LETTER AE WITH MACRON
          'ae',  // 483 ǣ LATIN SMALL LETTER AE WITH MACRON
          'G',   // 484 Ǥ LATIN CAPITAL LETTER G WITH STROKE
          'g',   // 485 ǥ LATIN SMALL LETTER G WITH STROKE
          'G',   // 486 Ǧ LATIN CAPITAL LETTER G WITH CARON
          'g',   // 487 ǧ LATIN SMALL LETTER G WITH CARON
          'K',   // 488 Ǩ LATIN CAPITAL LETTER K WITH CARON
          'k',   // 489 ǩ LATIN SMALL LETTER K WITH CARON
          'O',   // 490 Ǫ LATIN CAPITAL LETTER O WITH OGONEK
          'o',   // 491 ǫ LATIN SMALL LETTER O WITH OGONEK
          'O',   // 492 Ǭ LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
          'o',   // 493 ǭ LATIN SMALL LETTER O WITH OGONEK AND MACRON
          '3',   // 494 Ǯ LATIN CAPITAL LETTER EZH WITH CARON
          '3',   // 495 ǯ LATIN SMALL LETTER EZH WITH CARON
          'j',   // 496 ǰ LATIN SMALL LETTER J WITH CARON
          'DZ',  // 497 Ǳ LATIN CAPITAL LETTER DZ
          'Dz',  // 498 ǲ LATIN CAPITAL LETTER D WITH SMALL LETTER Z
          'dz',  // 499 ǳ LATIN SMALL LETTER DZ
          'G',   // 500 Ǵ LATIN CAPITAL LETTER G WITH ACUTE
          'g',   // 501 ǵ LATIN SMALL LETTER G WITH ACUTE
          'H',   // 502 Ƕ LATIN CAPITAL LETTER HWAIR
          'P',   // 503 Ƿ LATIN CAPITAL LETTER WYNN
          'N',   // 504 Ǹ LATIN CAPITAL LETTER N WITH GRAVE
          'n',   // 505 ǹ LATIN SMALL LETTER N WITH GRAVE
          'A',   // 506 Ǻ LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
          'a',   // 507 ǻ LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE
          'AE',  // 508 Ǽ LATIN CAPITAL LETTER AE WITH ACUTE
          'ae',  // 509 ǽ LATIN SMALL LETTER AE WITH ACUTE
          'O',   // 510 Ǿ LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
          'o',   // 511 ǿ LATIN SMALL LETTER O WITH STROKE AND ACUTE
          'A',   // 512 Ȁ LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
          'a',   // 513 ȁ LATIN SMALL LETTER A WITH DOUBLE GRAVE
          'A',   // 514 Ȃ LATIN CAPITAL LETTER A WITH INVERTED BREVE
          'a',   // 515 ȃ LATIN SMALL LETTER A WITH INVERTED BREVE
          'E',   // 516 Ȅ LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
          'e',   // 517 ȅ LATIN SMALL LETTER E WITH DOUBLE GRAVE
          'E',   // 518 Ȇ LATIN CAPITAL LETTER E WITH INVERTED BREVE
          'e',   // 519 ȇ LATIN SMALL LETTER E WITH INVERTED BREVE
          'I',   // 520 Ȉ LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
          'i',   // 521 ȉ LATIN SMALL LETTER I WITH DOUBLE GRAVE
          'I',   // 522 Ȋ LATIN CAPITAL LETTER I WITH INVERTED BREVE
          'i',   // 523 ȋ LATIN SMALL LETTER I WITH INVERTED BREVE
          'O',   // 524 Ȍ LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
          'o',   // 525 ȍ LATIN SMALL LETTER O WITH DOUBLE GRAVE
          'O',   // 526 Ȏ LATIN CAPITAL LETTER O WITH INVERTED BREVE
          'o',   // 527 ȏ LATIN SMALL LETTER O WITH INVERTED BREVE
          'R',   // 528 Ȑ LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
          'r',   // 529 ȑ LATIN SMALL LETTER R WITH DOUBLE GRAVE
          'R',   // 530 Ȓ LATIN CAPITAL LETTER R WITH INVERTED BREVE
          'r',   // 531 ȓ LATIN SMALL LETTER R WITH INVERTED BREVE
          'U',   // 532 Ȕ LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
          'u',   // 533 ȕ LATIN SMALL LETTER U WITH DOUBLE GRAVE
          'U',   // 534 Ȗ LATIN CAPITAL LETTER U WITH INVERTED BREVE
          'u',   // 535 ȗ LATIN SMALL LETTER U WITH INVERTED BREVE
          'S',   // 536 Ș LATIN CAPITAL LETTER S WITH COMMA BELOW
          's',   // 537 ș LATIN SMALL LETTER S WITH COMMA BELOW
          'T',   // 538 Ț LATIN CAPITAL LETTER T WITH COMMA BELOW
          't',   // 539 ț LATIN SMALL LETTER T WITH COMMA BELOW
          '3',   // 540 Ȝ LATIN CAPITAL LETTER YOGH
          '3',   // 541 ȝ LATIN SMALL LETTER YOGH
          'H',   // 542 Ȟ LATIN CAPITAL LETTER H WITH CARON
          'h',   // 543 ȟ LATIN SMALL LETTER H WITH CARON
          'n',   // 544 Ƞ LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
          'd',   // 545 ȡ LATIN SMALL LETTER D WITH CURL
          'O',   // 546 Ȣ LATIN CAPITAL LETTER OU
          'o',   // 547 ȣ LATIN SMALL LETTER OU
          'Z',   // 548 Ȥ LATIN CAPITAL LETTER Z WITH HOOK
          'z',   // 549 ȥ LATIN SMALL LETTER Z WITH HOOK
          'A',   // 550 Ȧ LATIN CAPITAL LETTER A WITH DOT ABOVE
          'a',   // 551 ȧ LATIN SMALL LETTER A WITH DOT ABOVE
          'E',   // 552 Ȩ LATIN CAPITAL LETTER E WITH CEDILLA
          'e',   // 553 ȩ LATIN SMALL LETTER E WITH CEDILLA
          'O',   // 554 Ȫ LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
          'o',   // 555 ȫ LATIN SMALL LETTER O WITH DIAERESIS AND MACRON
          'O',   // 556 Ȭ LATIN CAPITAL LETTER O WITH TILDE AND MACRON
          'o',   // 557 ȭ LATIN SMALL LETTER O WITH TILDE AND MACRON
          'O',   // 558 Ȯ LATIN CAPITAL LETTER O WITH DOT ABOVE
          'o',   // 559 ȯ LATIN SMALL LETTER O WITH DOT ABOVE
          'O',   // 560 Ȱ LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
          'o',   // 561 ȱ LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON
          'Y',   // 562 Ȳ LATIN CAPITAL LETTER Y WITH MACRON
          'y',   // 563 ȳ LATIN SMALL LETTER Y WITH MACRON
          'l',   // 564 ȴ LATIN SMALL LETTER L WITH CURL
          'n',   // 565 ȵ LATIN SMALL LETTER N WITH CURL
          't',   // 566 ȶ LATIN SMALL LETTER T WITH CURL
          'j',   // 567 ȷ LATIN SMALL LETTER DOTLESS J
          'db',  // 568 ȸ LATIN SMALL LETTER DB DIGRAPH
          'qp',  // 569 ȹ LATIN SMALL LETTER QP DIGRAPH
          'A',   // 570 Ⱥ LATIN CAPITAL LETTER A WITH STROKE
          'C',   // 571 Ȼ LATIN CAPITAL LETTER C WITH STROKE
          'c',   // 572 ȼ LATIN SMALL LETTER C WITH STROKE
          'L',   // 573 Ƚ LATIN CAPITAL LETTER L WITH BAR
          'T',   // 574 Ⱦ LATIN CAPITAL LETTER T WITH DIAGONAL STROKE
          's',   // 575 ȿ LATIN SMALL LETTER S WITH SWASH TAIL
          'z',   // 576 ɀ LATIN SMALL LETTER Z WITH SWASH TAIL
          '?',   // 577 Ɂ LATIN CAPITAL LETTER GLOTTAL STOP
          '?',   // 578 ɂ LATIN SMALL LETTER GLOTTAL STOP
          'B',   // 579 Ƀ LATIN CAPITAL LETTER B WITH STROKE
          'U',   // 580 Ʉ LATIN CAPITAL LETTER U BAR
          'V',   // 581 Ʌ LATIN CAPITAL LETTER TURNED V
          'E',   // 582 Ɇ LATIN CAPITAL LETTER E WITH STROKE
          'e',   // 583 ɇ LATIN SMALL LETTER E WITH STROKE
          'J',   // 584 Ɉ LATIN CAPITAL LETTER J WITH STROKE
          'j',   // 585 ɉ LATIN SMALL LETTER J WITH STROKE
          'Q',   // 586 Ɋ LATIN CAPITAL LETTER SMALL Q WITH HOOK TAIL
          'q',   // 587 ɋ LATIN SMALL LETTER Q WITH HOOK TAIL
          'R',   // 588 Ɍ LATIN CAPITAL LETTER R WITH STROKE
          'r',   // 589 ɍ LATIN SMALL LETTER R WITH STROKE
          'Y',   // 590 Ɏ LATIN CAPITAL LETTER Y WITH STROKE
          'y',   // 591 ɏ LATIN SMALL LETTER Y WITH STROKE
          'a',   // 592 ɐ LATIN SMALL LETTER TURNED A
          'a',   // 593 ɑ LATIN SMALL LETTER ALPHA
          'a',   // 594 ɒ LATIN SMALL LETTER TURNED ALPHA
          'b',   // 595 ɓ LATIN SMALL LETTER B WITH HOOK
          'o',   // 596 ɔ LATIN SMALL LETTER OPEN O
          'c',   // 597 ɕ LATIN SMALL LETTER C WITH CURL
          'd',   // 598 ɖ LATIN SMALL LETTER D WITH TAIL
          'd',   // 599 ɗ LATIN SMALL LETTER D WITH HOOK
          'e',   // 600 ɘ LATIN SMALL LETTER REVERSED E
          'e',   // 601 ə LATIN SMALL LETTER SCHWA
          'e',   // 602 ɚ LATIN SMALL LETTER SCHWA WITH HOOK
          'e',   // 603 ɛ LATIN SMALL LETTER OPEN E
          '3',   // 604 ɜ LATIN SMALL LETTER REVERSED OPEN E
          '3',   // 605 ɝ LATIN SMALL LETTER REVERSED OPEN E WITH HOOK
          'e',   // 606 ɞ LATIN SMALL LETTER CLOSED REVERSED OPEN E
          'j',   // 607 ɟ LATIN SMALL LETTER DOTLESS J WITH STROKE
          'g',   // 608 ɠ LATIN SMALL LETTER G WITH HOOK
          'g',   // 609 ɡ LATIN SMALL LETTER SCRIPT G
          'G',   // 610 ɢ LATIN LETTER SMALL CAPITAL G
          'y',   // 611 ɣ LATIN SMALL LETTER GAMMA
          'o',   // 612 ɤ LATIN SMALL LETTER RAMS HORN
          'u',   // 613 ɥ LATIN SMALL LETTER TURNED H
          'h',   // 614 ɦ LATIN SMALL LETTER H WITH HOOK
          'h',   // 615 ɧ LATIN SMALL LETTER HENG WITH HOOK
          'i',   // 616 ɨ LATIN SMALL LETTER I WITH STROKE
          'i',   // 617 ɩ LATIN SMALL LETTER IOTA
          'I',   // 618 ɪ LATIN LETTER SMALL CAPITAL I
          'l',   // 619 ɫ LATIN SMALL LETTER L WITH MIDDLE TILDE
          'l',   // 620 ɬ LATIN SMALL LETTER L WITH BELT
          'l',   // 621 ɭ LATIN SMALL LETTER L WITH RETROFLEX HOOK
          'lz',  // 622 ɮ LATIN SMALL LETTER LEZH
          'w',   // 623 ɯ LATIN SMALL LETTER TURNED M
          'w',   // 624 ɰ LATIN SMALL LETTER TURNED M WITH LONG LEG
          'm',   // 625 ɱ LATIN SMALL LETTER M WITH HOOK
          'n',   // 626 ɲ LATIN SMALL LETTER N WITH LEFT HOOK
          'n',   // 627 ɳ LATIN SMALL LETTER N WITH RETROFLEX HOOK
          'N',   // 628 ɴ LATIN LETTER SMALL CAPITAL N
          'o',   // 629 ɵ LATIN SMALL LETTER BARRED O
          'OE',  // 630 ɶ LATIN LETTER SMALL CAPITAL OE
          'w',   // 631 ɷ LATIN SMALL LETTER CLOSED OMEGA
          'o',   // 632 ɸ LATIN SMALL LETTER PHI
          'r',   // 633 ɹ LATIN SMALL LETTER TURNED R
          'r',   // 634 ɺ LATIN SMALL LETTER TURNED R WITH LONG LEG
          'r',   // 635 ɻ LATIN SMALL LETTER TURNED R WITH HOOK
          'r',   // 636 ɼ LATIN SMALL LETTER R WITH LONG LEG
          'r',   // 637 ɽ LATIN SMALL LETTER R WITH TAIL
          'r',   // 638 ɾ LATIN SMALL LETTER R WITH FISHHOOK
          'r',   // 639 ɿ LATIN SMALL LETTER REVERSED R WITH FISHHOOK
          'R',   // 640 ʀ LATIN LETTER SMALL CAPITAL R
          'R',   // 641 ʁ LATIN LETTER SMALL CAPITAL INVERTED R
          's',   // 642 ʂ LATIN SMALL LETTER S WITH HOOK
          'S',   // 643 ʃ LATIN SMALL LETTER ESH
          'f',   // 644 ʄ LATIN SMALL LETTER DOTLESS J WITH STROKE AND HOOK
          'S',   // 645 ʅ LATIN SMALL LETTER SQUAT REVERSED ESH
          'f',   // 646 ʆ LATIN SMALL LETTER ESH WITH CURL
          't',   // 647 ʇ LATIN SMALL LETTER TURNED T
          't',   // 648 ʈ LATIN SMALL LETTER T WITH RETROFLEX HOOK
          'u',   // 649 ʉ LATIN SMALL LETTER U BAR
          'u',   // 650 ʊ LATIN SMALL LETTER UPSILON
          'v',   // 651 ʋ LATIN SMALL LETTER V WITH HOOK
          'v',   // 652 ʌ LATIN SMALL LETTER TURNED V
          'm',   // 653 ʍ LATIN SMALL LETTER TURNED W
          'y',   // 654 ʎ LATIN SMALL LETTER TURNED Y
          'Y',   // 655 ʏ LATIN LETTER SMALL CAPITAL Y
          'z',   // 656 ʐ LATIN SMALL LETTER Z WITH RETROFLEX HOOK
          'z',   // 657 ʑ LATIN SMALL LETTER Z WITH CURL
          '3',   // 658 ʒ LATIN SMALL LETTER EZH
          '3',   // 659 ʓ LATIN SMALL LETTER EZH WITH CURL
          '?',   // 660 ʔ LATIN LETTER GLOTTAL STOP
          '?',   // 661 ʕ LATIN LETTER PHARYNGEAL VOICED FRICATIVE
          '?',   // 662 ʖ LATIN LETTER INVERTED GLOTTAL STOP
          'C',   // 663 ʗ LATIN LETTER STRETCHED C
          'O',   // 664 ʘ LATIN LETTER BILABIAL CLICK
          'B',   // 665 ʙ LATIN LETTER SMALL CAPITAL B
          'e',   // 666 ʚ LATIN SMALL LETTER CLOSED OPEN E
          'G',   // 667 ʛ LATIN LETTER SMALL CAPITAL G WITH HOOK
          'H',   // 668 ʜ LATIN LETTER SMALL CAPITAL H
          'j',   // 669 ʝ LATIN SMALL LETTER J WITH CROSSED-TAIL
          'k',   // 670 ʞ LATIN SMALL LETTER TURNED K
          'L',   // 671 ʟ LATIN LETTER SMALL CAPITAL L
          'q',   // 672 ʠ LATIN SMALL LETTER Q WITH HOOK
          't',   // 673 ʡ LATIN LETTER GLOTTAL STOP WITH STROKE
          't',   // 674 ʢ LATIN LETTER REVERSED GLOTTAL STOP WITH STROKE
          'dz',  // 675 ʣ LATIN SMALL LETTER DZ DIGRAPH
          'dz',  // 676 ʤ LATIN SMALL LETTER DEZH DIGRAPH
          'dz',  // 677 ʥ LATIN SMALL LETTER DZ DIGRAPH WITH CURL
          'ts',  // 678 ʦ LATIN SMALL LETTER TS DIGRAPH
          'tS',  // 679 ʧ LATIN SMALL LETTER TESH DIGRAPH
          'tc',  // 680 ʨ LATIN SMALL LETTER TC DIGRAPH WITH CURL
          'fn',  // 681 ʩ LATIN SMALL LETTER FENG DIGRAPH
          'ls',  // 682 ʪ LATIN SMALL LETTER LS DIGRAPH
          'lz',  // 683 ʫ LATIN SMALL LETTER LZ DIGRAPH
          'w',   // 684 ʬ LATIN LETTER BILABIAL PERCUSSIVE
          'n',   // 685 ʭ LATIN LETTER BIDENTAL PERCUSSIVE
          'u',   // 686 ʮ LATIN SMALL LETTER TURNED H WITH FISHHOOK
          'u'    // 687 ʯ LATIN SMALL LETTER TURNED H WITH FISHHOOK AND TAIL
    ]

    function toAscii(text) {
      var result = ''
      var start = 0
      for (var i = 0, len = text.length; i < len; ++i) {
        var code = text.charCodeAt(i)
        var replacement = null
        if (code > 159 + replacements.length) {
          replacement = '?'
        } else if (code > 159) {
          replacement = replacements[code - 160]
        } else if (code > 127) {
          replacement = ''
        }
        if (replacement !== null) {
          result += text.substring(start, i) + replacement
          start = i + 1
        }
      }
      result += text.substring(start)
      return result
    }

    function doWork() {
      if (app.selectionInfo.tag(Frame.Tag_1).tagFormat) {
        var frameNames = ["title", "artist", "album", "comment"]
        for (var fi = 0; fi < frameNames.length; ++fi) {
          var name = frameNames[fi];
          var oldTxt = app.getFrame(tagv1, name)
          var newTxt = toAscii(oldTxt)
          if (newTxt !== oldTxt) {
            app.setFrame(tagv1, name, newTxt)
          }
        }
      }
      if (!nextFile()) {
        if (isStandalone()) {
          // Save the changes if the script is started stand-alone, not from Kid3.
          app.saveDirectory()
        }
        Qt.quit()
      } else {
        setTimeout(doWork, 1)
      }
    }

    firstFile()
    doWork()
  }
}
