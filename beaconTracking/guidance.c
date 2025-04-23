#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Possible output directions for the user to follow
typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

// Tracks the history and current mode of the guidance algorithm
typedef struct 
{
    float  *mag_history;    // circular buffer of recent signal magnitudes
    uint32_t hist_idx;      // next slot in mag_history to overwrite
    int    fwd_drops;       // how many consecutive drops seen in forward mode
    int    rev_drops;       // how many consecutive drops seen in reverse mode
    bool   reverse_lock;    // true if in reverse mode
    int    cd_timer;        // ticks remaining before we can switch modes again
    Direction last_dir;     // what direction we returned last time
} GuidanceState;

// Tunable parameters for how sensitive and how often we reverse
typedef struct 
{
    uint32_t buf_size;      // length of the circular Goertzel power buffers
    int      drop_steps;    // how many drops in a row to trigger a mode change
    int      reverse_cd;    // cooldown duration (in measurements) after switching modes
    float    fwd_thresh;    // maximum angular offset (radians) to still call straight
    float    min_valid_mag; // ignore any mag below this
    uint32_t hist_size;     // length of our mag_history buffer
} GuidanceParams;

static inline void guidance_state_init (GuidanceState *st, const GuidanceParams *p)
{
    // allocate the history buffer
    st->mag_history = malloc(p->hist_size * sizeof(*st->mag_history));
    // initialize every slot to min_valid_mag so initial avg is stable
    for (uint32_t i = 0; i < p->hist_size; i++) {
        st->mag_history[i] = p->min_valid_mag;
    }

    st -> hist_idx = 0;
    st -> fwd_drops = 0;
    st -> rev_drops = 0;
    st -> reverse_lock = false;
    st -> cd_timer = 0;
    st -> last_dir = STRAIGHT_AHEAD;
}

static inline void guidance_state_free(GuidanceState *st)
{
    free(st->mag_history);
    st->mag_history = NULL;
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
    // 1) Fetch most recent sample from each buffer
    uint32_t ix = posx ? posx - 1 : p->buf_size - 1;
    uint32_t iy = posy ? posy - 1 : p->buf_size - 1;
    float Bpar_db  = gbufx[ix];
    float Bperp_db = gbufy[iy];

    // 2) overall magnitude
    float mag = sqrtf(Bpar_db * Bpar_db + Bperp_db * Bperp_db);

    // 3) if it's too weak, ignore it and repeat last direction
    if (mag < p->min_valid_mag) 
    {
        return st->last_dir;
    }

    // 4) Compute average of the previous hist_size mags
    float sum = 0.0f;
    for (uint32_t i = 0; i < p->hist_size; i++) {
        sum += st->mag_history[i];
    }
    float avg_prev = sum / p->hist_size;

    // 5) Detect a drop if current mag dips below that average
    bool drop = (mag < avg_prev);

    // 6) Update our history buffer
    st -> mag_history[st -> hist_idx] = mag;
    st -> hist_idx = (st -> hist_idx + 1) % p -> hist_size;

    // 7) Handle cooldown, then possibly flip forward/reverse mode.
    if (st -> cd_timer > 0) 
    {
        st -> cd_timer--; // Still cooling down from last switch; just decrement timer
    } 
    else // Cooldown expired: allow counting drops toward a mode flip
    {
        if (!st->reverse_lock) 
        {
            st -> fwd_drops = drop ? (st -> fwd_drops + 1) : 0; // In forward mode: count drops and switch to reverse if threshold reached
            if (st -> fwd_drops >= p -> drop_steps) 
            {
                st -> reverse_lock = true; // enter reverse mode
                st -> fwd_drops = 0; // reset counter
                st -> cd_timer = p->reverse_cd; // start cooldown
            }
        } 
        else 
        {
            st -> rev_drops = drop ? (st -> rev_drops + 1) : 0; // In reverse mode: count drops and switch back when threshold reached
            if (st -> rev_drops >= p -> drop_steps) 
            {
                st -> reverse_lock = false; // back to forward mode
                st -> rev_drops = 0; // reset counter
                st -> cd_timer = p -> reverse_cd; // start cooldown
            }
        }
    }

    // 8) If in reverse mode, flip the sign of both components
    float Bpar  = st -> reverse_lock ? -Bpar_db  : Bpar_db;
    float Bperp = st -> reverse_lock ? -Bperp_db : Bperp_db;

    // 9) Translate vector into a discrete direction command
    Direction dir;
    if (Bpar < 0.0f) 
    {
        dir = TURN_AROUND; // If the parallel component points backwards, do a full turn around
    }
    else
    {
        float ang = atan2f(Bperp, Bpar); // Compute the deviation angle from straight ahead
        if (fabsf(ang) <= p -> fwd_thresh) // If within the straight-ahead threshold, keep going
        {
            dir = STRAIGHT_AHEAD;
        } 
        else if (ang > 0.0f) // Otherwise, choose left or right based on the sign of the angle
        {
            dir = TURN_LEFT;
        } 
        else 
        {
            dir = TURN_RIGHT;
        }
    }

    // 10) Save for next time, then return.
    st -> last_dir = dir;
    return dir;
}
