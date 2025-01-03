// SPDX-License-Identifier: BSD-2-Clause
#include "tja/events.h"

#include "note.h"
#include "section.h"
#include "taco.h"
#include "tja/parser.h"
#include "tja/timestamp.h"
#include <string.h>

static const short note_types_[128] = {
    ['0'] = TACO_EVENT_NONE,     ['1'] = TACO_EVENT_DON,
    ['2'] = TACO_EVENT_KAT,      ['3'] = TACO_EVENT_DON_BIG,
    ['4'] = TACO_EVENT_KAT_BIG,  ['5'] = TACO_EVENT_ROLL,
    ['6'] = TACO_EVENT_ROLL_BIG, ['7'] = TACO_EVENT_BALLOON,
    ['8'] = TACO_EVENT_ROLL_END, ['9'] = TACO_EVENT_KUSUDAMA,
    ['A'] = TACO_EVENT_DON_BIG,  ['B'] = TACO_EVENT_KAT_BIG,
    ['C'] = TACO_EVENT_LANDMINE,
};

int tja_events_push_note_(tja_events *restrict events, int note, int line) {
  taco_event e = {
      .time = 0,
      .type = note < 128 ? note_types_[note] : 0,
      .line = line,
  };

  int result = tja_events_push_event_(events, &e);
  events->units += 1;
  return result;
}

int tja_events_push_event_(tja_events *restrict events,
                           const taco_event *restrict event) {
  if (event->type == TACO_EVENT_NONE) {
    return 0;
  } else if (event->type == TACO_EVENT_TJA_LEVELHOLD) {
    events->levelhold = true;
    return 0;
  }

  taco_event e;
  memcpy(&e, event, sizeof(e));
  tja_event_set_timestamp_(&e, 0, events->units);
  return taco_section_push_(events->events, &e);
}
