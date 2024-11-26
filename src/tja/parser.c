// SPDX-License-Identifier: BSD-2-Clause
#include "parser.h"
#include "alloc.h"
#include "taiko.h"
#include "tja/parser.h"

static taiko_parser_vfuncs vfuncs = {
    .free = (taiko_parser_free_fn)tja_parser_free_,
    .parse_file = (taiko_parser_parse_file_fn)tja_parser_parse_file_,
    .parse_stdio = (taiko_parser_parser_stdio_fn)tja_parser_parse_stdio_,
};

taiko_parser *taiko_parser_tja_create() {
  return taiko_parser_tja_create2(&taiko_default_allocator_);
}

taiko_parser *taiko_parser_tja_create2(taiko_allocator *alloc) {
  tja_parser *parser = tja_parser_create2_(alloc);
  taiko_parser *wrapper = taiko_parser_wrap_(alloc, parser, &vfuncs);

  if (!parser) {
    taiko_parser_free(wrapper);
  } else if (!wrapper) {
    tja_parser_free_(parser);
  }

  return wrapper;
}
