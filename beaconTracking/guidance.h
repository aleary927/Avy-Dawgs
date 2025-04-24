#ifndef GUIDANCE_H
#define GUIDANCE_H

#include <stdbool.h>
#include <stdint.h>
#include "circ_buf_float.h"

/**
 * Possible output directions for the user to follow
 */
typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

/**
 * Runtime state for the guidance algorithm.  Must be initialized
 * with guidance_state_init() before use.
 */
typedef struct 
{
    circ_buf_float history;         /* circular buffer of recent magnitudes */
    float          sum_history;     /* rolling sum of buffer values */
    int            fwd_drops;       /* consecutive drops in forward mode */
    int            rev_drops;       /* consecutive drops in reverse mode */
    bool           reverse_lock;    /* true if in reverse mode */
    int            cd_timer;        /* cooldown ticks before toggling */
    Direction      last_dir;        /* last returned direction */
} GuidanceState;

/**
 * Static parameters to tune algorithm behavior.  Fill this
 * once and pass by pointer to init and to each guidance_step.
*/
typedef struct 
{
    uint32_t buf_size;       /* length of Goertzel power buffers */
    int      drop_steps;     /* drops in a row to flip mode */
    int      reverse_cd;     /* cooldown duration (measurements) */
    float    fwd_thresh;     /* angular threshold for 'straight' */
    float    min_valid_mag;  /* ignore magnitudes below this */
    uint32_t hist_size;      /* length of magnitude history buffer */
} GuidanceParams;

/**
 * Allocate and initialize a GuidanceState.
 * st  : pointer to uninitialized state struct
 * p   : pointer to tuning parameters
 * returns true on success, false if p is invalid or OOM
*/
bool guidance_state_init(GuidanceState *st, const GuidanceParams *p);

/**
 * Free internal buffers in a GuidanceState.
 * st  : pointer to initialized state
 * After calling, st is zeroed except history.buf==NULL
*/
void guidance_state_free(GuidanceState *st);

/**
 * Main guidance step: ingest newest Goertzel power readings,
 * detect drops, optionally flip forward/reverse, and return
 * a discrete Direction.
 *
 * gbufx, gbufy : arrays of length buf_size containing dB values
 * posx, posy  : write indices into gbufx/gbufy (use latest sample)
 * st           : pointer to state from guidance_state_init
 * p            : pointer to your tuning params
 *
 * returns STRAIGHT_AHEAD, TURN_LEFT, TURN_RIGHT, or TURN_AROUND
 */
Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p);

#endif /* GUIDANCE_H */
