// guidance.c
#include "guidance.h"
#include <math.h>

#define EPSILON 1e-6f

#define MAX_HIST_SIZE 64
static float hist_buf[MAX_HIST_SIZE];

bool guidance_state_init(GuidanceState *st, const GuidanceParams *p)
{
    if (!p || p->hist_size == 0 || p->hist_size > MAX_HIST_SIZE || p->buf_size == 0)
        return false;
    st->history.buf  = hist_buf;
    st->history.size = p->hist_size;
    st->history.idx  = 0;

    // temp-seed until the first real sample
    for (uint32_t i = 0; i < p->hist_size; i++)
    {
        st->history.buf[i] = p->min_valid_mag;
    }
    st->sum_history = p->min_valid_mag * p->hist_size;

    st->fwd_drops    = 0;
    st->reverse_lock = false;
    st->cd_timer     = 0;
    st->did_reverse  = false;
    st->last_ratio   = 1.0f;
    st->turn_dir     = +1;
    st->seeded       = false;
    st->last_dir     = STRAIGHT_AHEAD;
    return true;
}


Direction guidance_step(const float *gbufx, const float *gbufy, uint32_t posx, uint32_t posy, GuidanceState *st, const GuidanceParams *p)
{
    // --- 1) Read the most recent sample from your Goertzel buffers ---
    uint32_t ix = posx < p->buf_size ? posx : 0;
    uint32_t iy = posy < p->buf_size ? posy : 0;
    ix = ix ? ix - 1 : p->buf_size - 1;
    iy = iy ? iy - 1 : p->buf_size - 1;

    float Bpar  = fabsf(gbufx[ix]);
    float Bperp = fabsf(gbufy[iy]);

    Direction out;
    // --- 2) Combined magnitude & weak-signal check ---
    float mag = sqrtf(Bpar*Bpar + Bperp*Bperp);
    if (mag < p->min_valid_mag) 
    {
        out = NO_SIGNAL;
        return out;
    }

    // --- 3) First real measurement seeds your history buffer ---
    if (!st->seeded) 
    {
        for (uint32_t i = 0; i < p->hist_size; i++)
        {
            st->history.buf[i] = mag;
        }
        st->sum_history = mag * p->hist_size;
        st->seeded      = true;
        return st->last_dir;
    }

    // --- 4) Update rolling-history & compute previous average ---
    float old = st->history.buf[st->history.idx];
    circ_buf_wr_float(&st->history, mag);
    st->sum_history = st->sum_history - old + mag;
    float avg_prev  = st->sum_history / (float)p->hist_size;

    // --- 5) Drop detection → possibly enter reverse_lock & flag U-turn ---
    if (!st->reverse_lock) 
    {
        if (mag < avg_prev) 
            st->fwd_drops++;
        else                
            st->fwd_drops = 0;

        if (st->fwd_drops >= p->drop_steps) 
        {
            st->reverse_lock = true;
            st->cd_timer     = p->reverse_cd;
            st->did_reverse  = true;
            st->fwd_drops    = 0;
        }
    }

    // --- 6) Steering (reverse_lock handled first) ---
    if (st->reverse_lock) 
    {
        if (st->did_reverse) 
        {
            // one-time U-turn
            out               = TURN_AROUND;
            st->did_reverse   = false;
        } 
        else 
        {
            // hold reversed heading
            out = STRAIGHT_AHEAD;
        }

        // --- 7) Now apply cooldown for next iteration ---
        if (st->cd_timer > 0) 
        {
            st->cd_timer--;
        } 
        else 
        {
            st->reverse_lock = false;
        }

    } else 
    {
        // --- 8) Normal forward steering via angle & ratio test ---
        float ang = atan2f(Bperp, Bpar);
        if (fabsf(ang) <= p->fwd_thresh) 
        {
            out = STRAIGHT_AHEAD;
        } 
        else 
        {
            // ratio = Bpar / max(eps, Bperp)
            float denom = (Bperp > EPSILON ? Bperp : EPSILON);
            float ratio = Bpar / denom;
            if (ratio < st->last_ratio) 
            {
                st->turn_dir = -st->turn_dir;
            }
            st->last_ratio = ratio;
            out = (st->turn_dir > 0 ? TURN_LEFT : TURN_RIGHT);
        }
    }

    st->last_dir = out;
    return out;
}
