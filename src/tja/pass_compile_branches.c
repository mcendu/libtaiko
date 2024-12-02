// SPDX-License-Identifier: BSD-2-Clause
#include "tja/postproc.h"

#include "note.h"
#include "section.h"
#include "taco.h"
#include "tja/branchtype.h"
#include "tja/parser.h"
#include <string.h>

const struct branch_start_event_ branch_scores[] = {
    [TAIKO_BRANCHTYPE_NONE] = {0, 0, 0, 0, 0, 0, 0, 0},
    [TAIKO_BRANCHTYPE_ACCURACY] = {20, 20, 10, 10, 1, 0, 0, 0},
    [TAIKO_BRANCHTYPE_ACCURACY_BIG] = {0, 20, 0, 10, 1, 0, 0, 0},
    [TAIKO_BRANCHTYPE_ROLL] = {0, 0, 0, 0, 1, 0, 0, 0},
};

typedef struct threshold_stats_ threshold_stats;

struct threshold_stats_ {
  int branch_type;
  int small_notes;
  int big_notes;
  int thres_a_src;
  int thres_m_src;
};

typedef void branch_threshold_calc(const threshold_stats *stats, int *thres_a,
                                   int *thres_m);

static void roll_threshold(const threshold_stats *stats, int *thres_a,
                           int *thres_m) {
  *thres_a = stats->thres_a_src;
  *thres_m = stats->thres_m_src;
}

static void acc_threshold(const threshold_stats *stats, int *thres_a,
                          int *thres_m) {
  int total = branch_scores[stats->branch_type].good * stats->small_notes +
              branch_scores[stats->branch_type].good_big * stats->big_notes;
  double percentage_a = (double)stats->thres_a_src / (double)0x1000000;
  double percentage_m = (double)stats->thres_m_src / (double)0x1000000;
  *thres_a = percentage_a * total;
  *thres_m = percentage_m * total;
}

static void none_threshold(const threshold_stats *stats, int *thres_a,
                           int *thres_m) {
  *thres_a = 1;
  *thres_m = 1;
}

branch_threshold_calc *const threshold_calc[] = {
    [TAIKO_BRANCHTYPE_NONE] = none_threshold,
    [TAIKO_BRANCHTYPE_ACCURACY] = acc_threshold,
    [TAIKO_BRANCHTYPE_ACCURACY_BIG] = acc_threshold,
    [TAIKO_BRANCHTYPE_ROLL] = roll_threshold,
};

void tja_pass_compile_branches_(tja_parser *parser, taiko_section *branch) {
  threshold_stats stats;
  int section_time = 0;
  int check_time = 0;
  int threshold_a;
  int threshold_m;

  memset(&stats, 0, sizeof(stats));

  taiko_section *new_events =
      taiko_section_create_(tja_parser_allocator_(parser));

  taiko_section_foreach_mut_ (i, branch) {
    switch (taiko_event_type(i)) {
    case TAIKO_EVENT_DON:
    case TAIKO_EVENT_KAT:
      stats.small_notes += 1;
      break;
    case TAIKO_EVENT_DON_BIG:
    case TAIKO_EVENT_KAT_BIG:
      stats.big_notes += 1;
      break;

    case TAIKO_EVENT_BRANCH_START:
      section_time = taiko_event_time(i);
      check_time = taiko_event_time(i);
      memset(&stats, 0, sizeof(stats));
      i->type = TAIKO_EVENT_NONE;
      break;
    case TAIKO_EVENT_MEASURE:
      check_time = taiko_event_time(i);
      break;

    case TAIKO_EVENT_TJA_BRANCH_TYPE:
      stats.branch_type = taiko_event_detail_int(i);
      if (stats.branch_type < 0 || stats.branch_type >= TAIKO_BRANCHTYPE_MAX)
        stats.branch_type = TAIKO_BRANCHTYPE_NONE;
      i->type = TAIKO_EVENT_NONE;
      break;
    case TAIKO_EVENT_TJA_BRANCH_THRESHOLD: {
      stats.thres_a_src = i->branch_cond.advanced;
      stats.thres_m_src = i->branch_cond.master;

      threshold_a = 0;
      threshold_m = 0;
      threshold_calc[stats.branch_type](&stats, &threshold_a, &threshold_m);

      taiko_event branch_events[3] = {
          {
              .time = section_time,
              .type = TAIKO_EVENT_BRANCH_START,
              .branch_start = branch_scores[stats.branch_type],
          },
          {
              .time = check_time,
              .type = TAIKO_EVENT_BRANCH_CHECK,
              .branch_cond = {.advanced = threshold_a, .master = threshold_m},
          },
          {
              .time = taiko_event_time(i),
              .type = TAIKO_EVENT_BRANCH_JUMP,
              .raw_params = {},
          },
      };
      taiko_section_push_many_(new_events, branch_events, 3);

      i->type = TAIKO_EVENT_NONE;
      memset(&stats, 0, sizeof(stats));
      section_time = taiko_event_time(i);
      check_time = taiko_event_time(i);
      break;
    }
    }
  }

  taiko_section_concat_(branch, new_events);
  taiko_section_free_(new_events);
}