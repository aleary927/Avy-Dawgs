#include <math.h>
#include <stdbool.h>
#include <stdint.h>

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
    float  last_mag;      // last measured combined signal magnitude
    int    fwd_drops;     // how many consecutive drops seen in forward mode
    int    rev_drops;     // how many consecutive drops seen in reverse mode
    bool   reverse_lock;  // true if in reverse mode
    int    cd_timer;      // ticks remaining before we can switch modes again
} GuidanceState;

// Tunable parameters for how sensitive and how often we reverse
typedef struct 
{
    uint32_t buf_size;    // length of the circular Goertzel power buffers
    int      drop_steps;  // how many drops in a row to trigger a mode change
    int      reverse_cd;  // cooldown duration (in measurements) after switching modes
    float    fwd_thresh;  // maximum angular offset (radians) to still call straight
} GuidanceParams;


/**
 * Examine the latest two antenna power readings, update our internal state,
 * and suggest which way to head next.
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
    uint32_t idx_x   = (posx == 0) ? (p->buf_size - 1) : (posx - 1);
    uint32_t idx_y   = (posy == 0) ? (p->buf_size - 1) : (posy - 1);
    float    Bpar_db = gbufx[idx_x]; // parallel-component power (in dB)
    float    Bperp_db= gbufy[idx_y]; // perpendicular-component power (in dB)

    // 2) overall magnitude
    float mag = sqrtf(Bpar_db * Bpar_db + Bperp_db * Bperp_db);

    // 3) drop detection
    bool drop = (mag < st->last_mag);
    st->last_mag = mag; // update for next time

    // 4) cooldown first, then mode logic
    if (st->cd_timer > 0) 
    {
        st->cd_timer--; // Still cooling down from last switch; just decrement timer
    } 
    else // Cooldown expired: allow counting drops toward a mode flip
    {
        if (!st->reverse_lock) 
        {
            st->fwd_drops = drop ? (st->fwd_drops + 1) : 0; // In forward mode: count drops and switch to reverse if threshold reached
            if (st->fwd_drops >= p->drop_steps) 
            {
                st->reverse_lock = true; // enter reverse mode
                st->fwd_drops    = 0; // reset counter
                st->cd_timer     = p->reverse_cd; // start cooldown
            }
        } 
        else 
        {
            st->rev_drops = drop ? (st->rev_drops + 1) : 0; // In reverse mode: count drops and switch back when threshold reached
            if (st->rev_drops >= p->drop_steps) 
            {
                st->reverse_lock = false; // back to forward mode
                st->rev_drops    = 0; // reset counter
                st->cd_timer = p->reverse_cd // start cooldown
            }
        }
    }

    // 5) If in reverse mode, flip the sign of both components
    float Bpar  = st->reverse_lock ? -Bpar_db  : Bpar_db;
    float Bperp = st->reverse_lock ? -Bperp_db : Bperp_db;

    // 6) Translate vector into a discrete direction command
    if (Bpar < 0.0f) 
    {
        return TURN_AROUND; // If the parallel component points backwards, do a full turn around
    }
    float ang = atan2f(Bperp, Bpar); // Compute the deviation angle from straight ahead
    if (fabsf(ang) <= p->fwd_thresh) // If within the straight-ahead threshold, keep going
    {
        return STRAIGHT_AHEAD;
    } 
    else if (ang > 0.0f) // Otherwise, choose left or right based on the sign of the angle
    {
        return TURN_LEFT;
    } 
    else 
    {
        return TURN_RIGHT;
    }
}
