#include <math.h>
#include <stdbool.h>

// --- possible return values for suggested heading ---
typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

// --- state carried between calls ---
typedef struct 
{
    double last_mag;      // last overall magnitude
    int    fwd_drops;     // consecutive drops moving forward
    int    rev_drops;     // consecutive drops moving backward
    bool   reverse_lock;  // are we in reverse mode?
    int    cd_timer;      // cooldown steps before unlocking reverse
} GuidanceState;

// --- tunable parameters ---
typedef struct 
{
    int    drop_steps;    // drops needed to trigger mode change
    int    reverse_cd;    // cooldown steps in reverse mode
    double fwd_thresh;    // radian threshold to go straight
} GuidanceParams;

/**
 * Update guidance state and return a new direction suggestion.
 *
 * @param Bpar_db   signed parallel reading (dB or linear units)
 * @param Bperp_db  signed perpendicular reading
 * @param st        pointer to persistent GuidanceState (must be initialized)
 * @param p         pointer to GuidanceParams
 * @return          one of STRAIGHT_AHEAD, TURN_LEFT, TURN_RIGHT, TURN_AROUND
 */

Direction guidance_step(const float *gbuffx, const float *gbuffy, unit32_t posx, unit32_t posy GuidanceState *st, const GuidanceParams *p)
{

    // 1) pull out the last sample from each buffer
    uint32_t idx_x = (posx == 0) ? (GOERTZEL_BUF_SIZE - 1) : (posx - 1);
    uint32_t idx_y = (posy == 0) ? (GOERTZEL_BUF_SIZE - 1) : (posy - 1);
    float Bpar_db  = gbufx[idx_x];
    float Bperp_db = gbufy[idx_y];

    // 2) compute combined magnitude
    float mag = sqrt(Bpar_db * Bpar_db + Bperp_db * Bperp_db);

    // 3) detect drop vs. last magnitude
    bool drop = (mag < st->last_mag);
    st->last_mag = mag;

    // 4) forward/reverse logic
    if (!st->reverse_lock) 
    {
        st->fwd_drops = drop ? (st->fwd_drops + 1) : 0;
        if (st->fwd_drops >= p->drop_steps)
        {
            st->reverse_lock = true;
            st->fwd_drops = 0;
            st->cd_timer = p->reverse_cd;
        }
    } 
    else 
    {
        if (st->cd_timer > 0)
        {
            st->cd_timer--;
        } 
        else 
        {
            st->rev_drops = drop ? (st->rev_drops + 1) : 0;
            if (st->rev_drops >= p->drop_steps) 
            {
                st->reverse_lock = false;
                st->rev_drops    = 0;
            }
        }
    }

    // 5) invert components if in reverse mode
    float Bpar = st->reverse_lock ? -Bpar_db : Bpar_db;
    float Bperp= st->reverse_lock ? -Bperp_db: Bperp_db;

    // 6) decide suggestion
    if (Bpar < 0.0f) 
    {
        return TURN_AROUND;
    }
    float ang = atan2f(Bperp, Bpar);
    if (fabsf(ang) <= p->fwd_thresh)
    {
        return STRAIGHT_AHEAD;
    } 
    else if (ang > 0.0f) 
    {
        return TURN_LEFT;
    } 
    else 
    {
        return TURN_RIGHT;
    }
}
