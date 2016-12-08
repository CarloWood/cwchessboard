/*
 * IRC - Internet Relay Chat, include/common.c
 * Copyright (C) 1998 Andrea Cocito
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * TABLE GENERATOR
 * The following part of code is NOT included in the actual server's
 * or library source, it's just used to build the above tables
 *
 * This should rebuild the actual tables and automatically place them
 * into this source file, note that this part of code is for developers
 * only, it's supposed to work on both signed and unsigned chars but I
 * actually tested it only on signed-char architectures, the code and
 * macros actually used by the server instead DO work and have been tested
 * on platforms where0 char is both signed or unsigned, this is true as long
 * as the <limits.h> macros are set properly and without any need to rebuild
 * the tables (which as said an admin should NEVER do, tables need to be rebuilt
 * only when one wants to really change the results or when one has to
 * compile on architectures where a char is NOT eight bits [?!], yes
 * it all is supposed to work in that case too... but I can't test it
 * because I've not found a machine in the world where this happens).
 *
 * NEVER -f[un]signed-char on gcc since that does NOT fix the named macros
 * and you end up in a non-ANSI environment where CHAR_MIN and CHAR_MAX
 * are _not_ the real limits of a default 'char' type. This is true for
 * both admins and coders.
 *
 */
#include "config.h"

#include "chattr.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static void zeroTables(void);
static void markString(attr_t macro, char const* s);
static void unMarkString(attr_t macro, char const* s);
static void markRange(attr_t macro, char from, char to);
static void moveMacro(attr_t from, attr_t to);
static void setLowHi(char const firstlow, char const lastlow, char const firsthi);

char NTL_tolower_tab[1 + CHAR_MAX - CHAR_MIN];		/* 256 bytes */
char NTL_toupper_tab[1 + CHAR_MAX - CHAR_MIN];		/* 256 bytes */
attr_t NTL_char_attrib[1 + CHAR_MAX - CHAR_MIN];	/* 256 attr_t = 512 bytes */

/*
 * makeTables() 
 * Where we make the tables, edit ONLY this to change the tables.
 */

static void makeTables(void)
{
  zeroTables();

  markString(pgn_blank, "\011\013\014\040");

  markString(pgn_eol, "\r\n");

  markRange(pgn_file, 'a', 'h');

  markRange(pgn_rank, '1', '8');

  markString(pgn_piece, "RNBQK");

  markString(pgn_check, "+#");

  markString(pgn_punctuation_junk, ",;");

  markRange(pgn_digit, '0', '9');

  markString(pgn_alpha, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

  moveMacro(pgn_alnum, pgn_tagname_continuation);
  markString(pgn_tagname_continuation, "_");

  markString(pgn_tag_separator_junk, ":=");

  markRange(pgn_printable_string, -96, -1);
  markRange(pgn_printable_string, 35, 91);
  markRange(pgn_printable_string, 93, 126);
  markString(pgn_printable_string, " !");

  moveMacro(pgn_eol, pgn_quote_or_eol);
  markString(pgn_quote_or_eol, "\"");

  markString(pgn_comment_start, "{;");

  markRange(pgn_printable_comment, -96, -1);
  markRange(pgn_printable_comment, 32, 124);
  moveMacro(pgn_white_space, pgn_printable_comment);
  markString(pgn_printable_comment, "~");

  markRange(pgn_printable, -96, -1);
  markRange(pgn_printable, 32, 126);

  /* And finally let's take care of the toLower/toUpper stuff */

  setLowHi('a', 'z', 'A');
  setLowHi('\xe0', '\xf6', '\xc0');
  setLowHi('\xf8', '\xfe', '\xd8');
}

/* 
 * main()
 * This is the main program to be executed for -DMAKETABLES
 */

static void dumphw(attr_t* p, int beg);
static void dumphb(char* p, int beg);

int main(void)
{
  int i;

  /* Make the tables */
  makeTables();

  /* Dump them as ANSI C source to be included below */
  printf("/*\n * Automatically Generated Tables - DO NOT EDIT\n */\n");
  printf("#include <limits.h>\n#include \"chattr.h\"\n");

  /* NTL_tolower_tab */
  printf("char const ToLowerTab_8859_1[] = {\n");
  printf("#if (CHAR_MIN<0)\n");
  i = (int)((char)SCHAR_MIN);
  dumphb(NTL_tolower_tab, i);
  printf("                ,\n");
  printf("#endif /* (CHAR_MIN<0) */\n");
  i = 0;
  dumphb(NTL_tolower_tab, i);
  printf("#if (!(CHAR_MIN<0))\n");
  printf("                ,\n");
  i = (int)((char)SCHAR_MIN);
  dumphb(NTL_tolower_tab, i);
  printf("#endif /* (!(CHAR_MIN<0)) */\n");
  printf("  };\n\n");

  /* NTL_toupper_tab */
  printf("char const ToUpperTab_8859_1[] = {\n");
  printf("#if (CHAR_MIN<0)\n");
  i = (int)((char)SCHAR_MIN);
  dumphb(NTL_toupper_tab, i);
  printf("                ,\n");
  printf("#endif /* (CHAR_MIN<0) */\n");
  i = 0;
  dumphb(NTL_toupper_tab, i);
  printf("#if (!(CHAR_MIN<0))\n");
  printf("                ,\n");
  i = (int)((char)SCHAR_MIN);
  dumphb(NTL_toupper_tab, i);
  printf("#endif /* (!(CHAR_MIN<0)) */\n");
  printf("  };\n\n");

  /* NTL_char_attrib */
  printf("attr_t const PGN_CharAttrTab[] = {\n");
  printf("#if (CHAR_MIN<0)\n");
  i = (int)((char)SCHAR_MIN);
  dumphw(NTL_char_attrib, i);
  printf("                ,\n");
  printf("#endif /* (CHAR_MIN<0) */\n");
  i = 0;
  dumphw(NTL_char_attrib, i);
  printf("#if (!(CHAR_MIN<0))\n");
  printf("                ,\n");
  i = (int)((char)SCHAR_MIN);
  dumphw(NTL_char_attrib, i);
  printf("#endif /* (!(CHAR_MIN<0)) */\n");
  printf("  };\n\n");
}

/* A few utility functions for makeTables() */

static void zeroTables(void)
{
  for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
  {
    NTL_tolower_tab[i - CHAR_MIN] = (char)i;	/* Unchanged */
    NTL_toupper_tab[i - CHAR_MIN] = (char)i;	/* Unchanged */
    NTL_char_attrib[i - CHAR_MIN] = 0x0000;	/* Nothing */
  }
}

static void markString(attr_t macro, char const* s)
{
  while (*s)
    NTL_char_attrib[*(s++) - CHAR_MIN] |= macro;
}

static void unMarkString(attr_t macro, char const* s)
{
  while (*s)
    NTL_char_attrib[*(s++) - CHAR_MIN] &= ~macro;
}

static void markRange(attr_t macro, char from, char to)
{
  for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
    if (((unsigned char)i >= (unsigned char)from)
	&& ((unsigned char)i <= (unsigned char)to))
      NTL_char_attrib[(char)i - CHAR_MIN] |= macro;
}

static void moveMacro(attr_t from, attr_t to)
{
  for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
    if (NTL_char_attrib[i - CHAR_MIN] & from)
      NTL_char_attrib[i - CHAR_MIN] |= to;
}

static void setLowHi(char const firstlow, char const lastlow, char const firsthi)
{
  for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
    if (((unsigned char)i >= (unsigned char)firstlow)
	&& ((unsigned char)i <= (unsigned char)lastlow))
    {
      int j = ((int)((char)(i + (int)(firsthi - firstlow))));
      NTL_tolower_tab[((char)j) - CHAR_MIN] = (char)i;
      NTL_toupper_tab[((char)i) - CHAR_MIN] = (char)j;
    }
}

/* These are used in main() to actually dump the tables, each function
   dumps half table as hex/char constants... */

#define ROWSIZE 8

static void dumphb(char* tbl, int beg)
{
  int i, j, k;
  char *p = &tbl[beg - CHAR_MIN];
  unsigned char c;
  for (i = 0; i <= SCHAR_MAX; i += ROWSIZE)
  {
    k = i + ROWSIZE - 1;
    if (k > SCHAR_MAX)
      k = SCHAR_MAX;

    c = (unsigned char)(beg + i);
    printf("/*");
    if ((c > 0) && (c < SCHAR_MAX) && (isprint(c)) && (c != '\\')
	&& (c != '\''))
      printf(" '%c'", c);
    else
      printf(" x%02x", ((int)c));

    c = (unsigned char)(beg + k);
    printf("-");
    if ((c > 0) && (c < SCHAR_MAX) && (isprint(c)) && (c != '\\')
	&& (c != '\''))
      printf("'%c'", c);
    else
      printf("x%02x", ((int)c));
    printf(" */");

    for (j = i; j <= k; j++)
    {
      c = p[j];
      if ((c > 0) && (c < SCHAR_MAX) && (isprint(c)) && (c != '\\')
	  && (c != '\''))
	printf("    '%c'", c);
      else
	printf(" '\\x%02x'", ((int)c));
      if (j < SCHAR_MAX)
	printf(",");
    }
    printf("\n");
  }
}

static void dumphw(attr_t* tbl, int beg)
{
  int i, j, k;
  attr_t* p = &tbl[beg - CHAR_MIN];
  unsigned char c;
  for (i = 0; i <= SCHAR_MAX; i += ROWSIZE)
  {
    k = i + ROWSIZE - 1;
    if (k > SCHAR_MAX)
      k = SCHAR_MAX;

    c = (unsigned char)(beg + i);
    printf("/*");
    if ((c > 0) && (c < SCHAR_MAX) && (isprint(c)) && (c != '\\')
	&& (c != '\''))
      printf(" '%c'", c);
    else
      printf(" x%02x", ((int)c));

    c = (unsigned char)(beg + k);
    printf("-");
    if ((c > 0) && (c < SCHAR_MAX) && (isprint(c)) && (c != '\\')
	&& (c != '\''))
      printf("'%c'", c);
    else
      printf("x%02x", ((int)c));
    printf(" */");

    for (j = i; j <= k; j++)
    {
      printf(" 0x%04x", p[j] & 0xffffffff);
      if (j < SCHAR_MAX)
	printf(",");
    }
    printf("\n");
  }
}

