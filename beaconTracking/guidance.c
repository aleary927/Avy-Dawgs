#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "circ_buf_float.h"

// Possible output directions for the user to follow
typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

typedef struct 
{
    circ_buf_float  history;        // circular buffer of recent magnitudes (linear scale)
    float           sum_history;    // rolling sum of values in mag_history
    int             fwd_drops;      // how many consecutive drops seen in forward mode
    int             rev_drops;      // how many consecutive drops seen in reverse mode
    bool            reverse_lock;   // true if in reverse mode
    int             cd_timer;       // ticks remaining before we can switch modes again
    Direction       last_dir;       // what direction we returned last time
} GuidanceState;

typedef struct 
{
    uint32_t buf_size;      // length of the circular Goertzel power buffers
    int      drop_steps;    // how many drops in a row to trigger a mode change
    int      reverse_cd;    // cooldown duration (in measurements) after switching modes
    float    fwd_thresh;    // maximum angular offset (radians) to still call straight
    float    min_valid_mag; // ignore any magnitude below this (linear units)
    uint32_t hist_size;     // length of our mag_history buffer
} GuidanceParams;

typedef bool init_ret;
static inline bool guidance_state_init(GuidanceState *st, const GuidanceParams *p)
{
    if (p -> hist_size == 0 || p -> buf_size == 0)
        return false;

    st -> history.buf = malloc(p -> hist_size * sizeof *st -> history.buf);
    st -> history.idx  = 0;
    st -> history.size = p -> hist_size;

    for (uint32_t i = 0; i < p -> hist_size; i++)
        st -> history.buf[i] = p -> min_valid_mag;
    st -> sum_history = p -> min_valid_mag * p -> hist_size;

    st -> fwd_drops = 0;
    st -> rev_drops = 0;
    st -> reverse_lock = false;
    st -> cd_timer = 0;
    st -> last_dir = STRAIGHT_AHEAD;
    return true;
}

static inline void guidance_state_free(GuidanceState *st)
{
    free(st -> history.buf);
    st -> history.buf = NULL;
}

/**
 * Examine the latest two antenna power readings, update our internal state,
 * and suggest which way to head next. Only use samples stronger than min_valid_mag. 
 * If we see a weak sample, just re-issue the last direction.
 *
 * @param gbufx       pointer to the circular buffer of X-axis (parallel) power values
 * @param gbufy       pointer to the circular buffer of Y-axis (perpendicular) power values
 * @param posx        next write index into gbufx
 * @param posy        next write index into gbufy
 * @param st          pointer to persistent GuidanceState (init before use)
 * @param p           pointer to GuidanceParams
 * @return            a Direction enum telling the user where to go next
 */
Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p)
{
    // 1) Fetch most recent sample index
    uint32_t ix = (posx < p -> buf_size ? posx : 0);
    uint32_t iy = (posy < p -> buf_size ? posy : 0);
    ix = ix ? ix - 1 : p -> buf_size - 1;
    iy = iy ? iy - 1 : p -> buf_size - 1;

    float Bpar_db  = gbufx[ix];
    float Bperp_db = gbufy[iy];

    // 2) Convert dB readings to linear amplitude
    float Bpar_lin  = powf(10.0f, Bpar_db  / 20.0f);
    float Bperp_lin = powf(10.0f, Bperp_db / 20.0f);

    // 3) overall magnitude in linear units
    float mag = sqrtf(Bpar_lin * Bpar_lin + Bperp_lin * Bperp_lin);

    // 4) if it's too weak, repeat last direction (history and counters unchanged)
    if (mag < p -> min_valid_mag) 
    {
        return st -> last_dir;
    }

    // 5) Compute average of history in O(1) via rolling sum
    float avg_prev = st -> sum_history / (float)p -> hist_size;

    // 6) Detect a drop
    bool drop = (mag < avg_prev);

    // 7) update rolling sum + circ_buf
    float old = circ_buf_rd_float(&st -> history);
    st -> sum_history -= old;
    circ_buf_wr_float(&st -> history, mag);
    st -> sum_history += mag;

    // 8) Handle cooldown and mode flips
    if (st -> cd_timer > 0) 
    {
        st -> cd_timer--;
    }
    else 
    {
        if (!st -> reverse_lock) 
        {
            st -> fwd_drops = drop ? (st -> fwd_drops + 1) : 0;
            if (st -> fwd_drops >= p -> drop_steps) 
            {
                st -> reverse_lock = true;
                st -> fwd_drops    = 0;
                st -> cd_timer     = p -> reverse_cd;
            }
        } 
        else 
        {
            st -> rev_drops = drop ? (st -> rev_drops + 1) : 0;
            if (st -> rev_drops >= p -> drop_steps) 
            {
                st -> reverse_lock = false;
                st -> rev_drops    = 0;
                st -> cd_timer     = p -> reverse_cd;
            }
        }
    }

    // 9) Possibly flip vector in reverse mode
    float Ppar  = st -> reverse_lock ? -Bpar_lin  : Bpar_lin;
    float Pperp = st -> reverse_lock ? -Bperp_lin : Bperp_lin;

    // 10) Translate into a discrete direction
    Direction dir;
    if (Ppar < 0.0f) 
    {
        dir = TURN_AROUND;
    } 
    else 
    {
        float ang = atan2f(Pperp, Ppar);
        if (fabsf(ang) <= p -> fwd_thresh) 
        {
            dir = STRAIGHT_AHEAD;
        } 
        else if (ang > 0.0f) 
        {
            dir = TURN_LEFT;
        } 
        else 
        {
            dir = TURN_RIGHT;
        }
    }

    // 11) Save and return
    st->last_dir = dir;
    return dir;
}
