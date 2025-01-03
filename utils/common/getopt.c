/* SPDX-License-Identifier: MIT */
/*
 * Copyright © 2005-2020 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#define _XOPEN_SOURCE 600
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

char *optarg;
int optind = 1, opterr = 1, optopt, __optpos, __optreset = 0;

#define optpos __optpos

int getopt(int argc, char *const argv[], const char *optstring) {
  int i;
  wchar_t c, d;
  int k, l;
  char *optchar;

  if (!optind || __optreset) {
    __optreset = 0;
    __optpos = 0;
    optind = 1;
  }

  if (optind >= argc || !argv[optind])
    return -1;

  if (argv[optind][0] != '-') {
    if (optstring[0] == '-') {
      optarg = argv[optind++];
      return 1;
    }
    return -1;
  }

  if (!argv[optind][1])
    return -1;

  if (argv[optind][1] == '-' && !argv[optind][2])
    return optind++, -1;

  if (!optpos)
    optpos++;
  if ((k = mbtowc(&c, argv[optind] + optpos, MB_LEN_MAX)) < 0) {
    k = 1;
    c = 0xfffd; /* replacement char */
  }
  optchar = argv[optind] + optpos;
  optpos += k;

  if (!argv[optind][optpos]) {
    optind++;
    optpos = 0;
  }

  if (optstring[0] == '-' || optstring[0] == '+')
    optstring++;

  i = 0;
  d = 0;
  do {
    l = mbtowc(&d, optstring + i, MB_LEN_MAX);
    if (l > 0)
      i += l;
    else
      i++;
  } while (l && d != c);

  if (d != c || c == ':') {
    optopt = c;
    if (optstring[0] != ':' && opterr)
      fprintf(stderr, "%s: unrecognized option: %*s\n", argv[0], k, optchar);
    return '?';
  }
  if (optstring[i] == ':') {
    optarg = 0;
    if (optstring[i + 1] != ':' || optpos) {
      optarg = argv[optind++];
      if (optpos)
        optarg += optpos;
      optpos = 0;
    }
    if (optind > argc) {
      optopt = c;
      if (optstring[0] == ':')
        return ':';
      if (opterr)
        fprintf(stderr, "%s: option requires an argument: %*s\n", argv[0], k,
                optchar);
      return '?';
    }
  }
  return c;
}
