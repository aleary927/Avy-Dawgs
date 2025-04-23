#include <math.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum 
{
    STRAIGHT_AHEAD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_AROUND
} Direction;

typedef struct 
{
    float  last_mag;      // last combined magnitude
    int    fwd_drops;     // forward mode drop counter
    int    rev_drops;     // reverse mode drop counter
    bool   reverse_lock;  // true if in reverse mode
    int    cd_timer;      // cooldown ticks remaining
} GuidanceState;

typedef struct 
{
    uint32_t buf_size;    // size of the Goertzel buffers
    int      drop_steps;  // threshold for consecutive drops
    int      reverse_cd;  // cooldown duration after reversing
    float    fwd_thresh;  // max angle (radians) to still go straight
} GuidanceParams;


/**
 * Update guidance state and return a new direction suggestion.
 *
 * @param gbufx       pointer to X-axis Goertzel power buffer
 * @param gbufy       pointer to Y-axis Goertzel power buffer
 * @param posx        next write index into gbufx
 * @param posy        next write index into gbufy
 * @param st          pointer to persistent GuidanceState (init before use)
 * @param p           pointer to GuidanceParams
 * @return            one of STRAIGHT_AHEAD, TURN_LEFT, TURN_RIGHT, TURN_AROUND
 */
 
Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p)
{
    // 1) Fetch most recent sample from each buffer
    uint32_t idx_x   = (posx == 0) ? (p->buf_size - 1) : (posx - 1);
    uint32_t idx_y   = (posy == 0) ? (p->buf_size - 1) : (posy - 1);
    float    Bpar_db = gbufx[idx_x];
    float    Bperp_db= gbufy[idx_y];

    // 2) overall magnitude
    float mag = sqrtf(Bpar_db * Bpar_db + Bperp_db * Bperp_db);

    // 3) drop detection
    bool drop = (mag < st->last_mag);
    st->last_mag = mag;

    // 4) cooldown first, then mode logic
    if (st->cd_timer > 0) 
    {
        st->cd_timer--;
    } 
    else 
    {
        if (!st->reverse_lock) 
        {
            st->fwd_drops = drop ? (st->fwd_drops + 1) : 0;
            if (st->fwd_drops >= p->drop_steps) 
            {
                st->reverse_lock = true;
                st->fwd_drops    = 0;
                st->cd_timer     = p->reverse_cd;
            }
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

    // 5) flip vector if in reverse
    float Bpar  = st->reverse_lock ? -Bpar_db  : Bpar_db;
    float Bperp = st->reverse_lock ? -Bperp_db : Bperp_db;

    // 6) heading suggestion
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
