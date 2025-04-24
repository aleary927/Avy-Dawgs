// guidance.h
#ifndef GUIDANCE_H
#define GUIDANCE_H

#include <stdbool.h>
#include <stdint.h>
#include "circ_buf.h"

typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

typedef struct 
{
    circ_buf_float history;
    float          sum_history;
    int            fwd_drops;
    bool           reverse_lock;
    int            cd_timer;
    bool           did_reverse;
    float          last_ratio;
    int            turn_dir;   /* +1 or -1 */
    bool           seeded;
    Direction      last_dir;
} GuidanceState;

typedef struct 
{
    uint32_t buf_size;       /* length of gbufx & gbufy */
    uint32_t hist_size;      /* how many mags to average */
    int      drop_steps;     /* consecutive drops → U-turn */
    int      reverse_cd;     /* cooldown ticks after U-turn */
    float    fwd_thresh;     /* straight-ahead angle cutoff (rad) */
    float    min_valid_mag;  /* ignore anything weaker than this */
} GuidanceParams;

bool guidance_state_init(GuidanceState *st, const GuidanceParams *p);
void guidance_state_free(GuidanceState *st);

Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p);

#endif /* GUIDANCE_H */
