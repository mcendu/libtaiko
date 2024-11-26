// SPDX-License-Identifier: BSD-2-Clause
#include "tja/segment.h"

#include "note.h"
#include "section.h"
#include "taiko.h"
#include "tja/events.h"
#include "tja/timestamp.h"
#include <stddef.h>

int tja_segment_init_(tja_segment *segment, taiko_section *buf) {
  segment->segment = buf;
  segment->measures = 0;
  segment->levelhold = false;
  return buf != NULL;
}

int tja_segment_push_barline_(tja_segment *segment, int units) {
  int retval = 0;
  taiko_event barline = {
      .time = 0,
      .type = TAIKO_EVENT_MEASURE,
      .measure =
          {
              .real = true,
              .hidden = false,
              .tja_units = units,
          },
  };
  tja_event_set_measure_(&barline, segment->measures);

  retval = taiko_section_push_(segment->segment, &barline);
  return retval;
}

void tja_segment_finish_measure_(tja_segment *segment) {
  segment->measures += 1;
}

int tja_segment_push_events_(tja_segment *segment, tja_events *events) {
  taiko_section_foreach_mut_ (i, events->events) {
    tja_event_set_measure_(i, segment->measures);
  }
  segment->levelhold = segment->levelhold || events->levelhold;
  return taiko_section_concat_(segment->segment, events->events);
}
