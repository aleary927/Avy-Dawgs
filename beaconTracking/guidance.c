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

Direction guidance_step(double Bpar_db, double Bperp_db, GuidanceState *st, const GuidanceParams *p)
{
    // 1) compute combined magnitude
    double mag = sqrt(Bpar_db*Bpar_db + Bperp_db*Bperp_db);

    // 2) detect drop vs. last magnitude
    bool drop = (mag < st->last_mag);
    st->last_mag = mag;

    // 3) forward/reverse logic
    if (!st->reverse_lock) 
    {
        if (drop)
        {
            st->fwd_drops++;
        }
        else
        {
            st->fwd_drops = 0;
        }
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
            if (drop)
            {
                st->rev_drops++;
            }
            else
            {
                st->rev_drops = 0;
            }
            if (st->rev_drops >= p->drop_steps) 
            {
                st->reverse_lock = false;
                st->rev_drops    = 0;
            }
        }
    }

    // 4) optionally invert components if in reverse mode
    double Bpar = st->reverse_lock ? -Bpar_db : Bpar_db;
    double Bperp= st->reverse_lock ? -Bperp_db: Bperp_db;

    // 5) decide suggestion
    if (Bpar < 0.0) 
    {
        return TURN_AROUND;
    }
    double ang = atan2(Bperp, Bpar);
    if (fabs(ang) <= p->fwd_thresh)
    {
        return STRAIGHT_AHEAD;
    } 
    else if (ang > 0.0) 
    {
        return TURN_LEFT;
    } 
    else 
    {
        return TURN_RIGHT;
    }
}
