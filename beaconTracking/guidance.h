#ifndef GUIDANCE_H
#define GUIDANCE_H

#include <stdbool.h>
#include <stdint.h>
#include "circ_buf_float.h"

/**
 * Possible output directions for the user to follow
 */
typedef enum {
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

/**
 * Keeps track of history, sums, and state for guidance
 */
typedef struct {
    circ_buf_float history;    /* circular buffer of recent magnitudes */
    float          sum_history;/* rolling sum of buffer values */
    int            fwd_drops;  /* consecutive drops in forward mode */
    int            rev_drops;  /* consecutive drops in reverse mode */
    bool           reverse_lock; /* true if in reverse mode */
    int            cd_timer;   /* cooldown ticks before toggling */
    Direction      last_dir;   /* last returned direction */
} GuidanceState;

/**
 * Tuning parameters for the guidance algorithm
 */
typedef struct {
    uint32_t buf_size;       /* length of Goertzel power buffers */
    int      drop_steps;     /* drops in a row to flip mode */
    int      reverse_cd;     /* cooldown duration (measurements) */
    float    fwd_thresh;     /* angular threshold for 'straight' */
    float    min_valid_mag;  /* ignore magnitudes below this */
    uint32_t hist_size;      /* length of magnitude history buffer */
} GuidanceParams;

/**
 * Initialize the GuidanceState using the given parameters.
 * Returns true on success, false on invalid params or allocation failure.
 */
bool guidance_state_init(GuidanceState *st, const GuidanceParams *p);

/**
 * Release resources held by GuidanceState
 */
void guidance_state_free(GuidanceState *st);

/**
 * Process the latest antenna readings and update state.
 * Returns the next Direction for the user.
 */
Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p);

#endif /* GUIDANCE_H */
