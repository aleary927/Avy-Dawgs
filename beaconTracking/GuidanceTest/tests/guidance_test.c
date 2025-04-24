// tests/guidance_test.c
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "guidance.h"

// Convenience macro for succinct PASS/FAIL reporting
#define RUN(desc, cond) do {                                           \
    if (!(cond)) {                                                     \
        fprintf(stderr, "[FAIL] %s\n", desc);                         \
        return 1;                                                      \
    } else {                                                           \
        printf("[PASS] %s\n", desc);                                  \
    }                                                                  \
} while (0)

int main(void) {
    float gbufx[1], gbufy[1];
    Direction d;

    //
    // 1) Seeding: first call should not steer, just return STRAIGHT_AHEAD
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 3,
            .drop_steps    = 2,
            .reverse_cd    = 2,
            .fwd_thresh    = 0.5235988f,    // ≈π/6 (30°)
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        RUN("init succeeded", guidance_state_init(&st, &p));

        gbufx[0] = 10.0f;  gbufy[0] = 5.0f;
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("seeding returns STRAIGHT_AHEAD", d == STRAIGHT_AHEAD);

        guidance_state_free(&st);
    }

    //
    // 2) STRAIGHT_AHEAD when perpendicular << parallel
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 3,
            .drop_steps    = 2,
            .reverse_cd    = 2,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        gbufx[0] = 7500000;  gbufy[0] = 0.1f;
        guidance_step(gbufx, gbufy, 0, 0, &st, &p);

        gbufx[0] = 15000000;  gbufy[0] = 0.01f;
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("straight when perp << par", d == STRAIGHT_AHEAD);

        guidance_state_free(&st);
    }

    //
    // 3) TURN_LEFT when angle > fwd_thresh
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        gbufx[0] = 75000000;  gbufy[0] = 75000000;  // 45° > 0.5 rad
        guidance_step(gbufx, gbufy, 0, 0, &st, &p);

        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("turn left when perp = par", d == TURN_LEFT);

        guidance_state_free(&st);
    }

    //
    // 4) TURN_RIGHT when turn_dir < 0
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.2f,
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        gbufx[0] = 7500000;  gbufy[0] = 0.0f;
        guidance_step(gbufx, gbufy, 0, 0, &st, &p);

        st.turn_dir = -1;

        gbufx[0] = 3750000;  gbufy[0] = 2000000;  // ~31° > 0.2 rad
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("turn right when turn_dir<0", d == TURN_RIGHT);

        guidance_state_free(&st);
    }

    //
    // 5) fwd_thresh boundary: angle == fwd_thresh -> STRAIGHT_AHEAD
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        gbufx[0] = 1.0f;  gbufy[0] = 0.0f;
        guidance_step(gbufx, gbufy, 0, 0, &st, &p);

        float perp = tanf(0.5f) * 1.0f;
        gbufx[0] = 1.0f;  gbufy[0] = perp;
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("straight at fwd_thresh boundary", d == STRAIGHT_AHEAD);

        guidance_state_free(&st);
    }

    //
    // 6) min_valid_mag filter: below threshold repeats last_dir
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 2.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        st.last_dir = TURN_RIGHT;
        gbufx[0] = 1.0f;  gbufy[0] = 1.0f;
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("min_valid_mag blocks weak mag", d == TURN_RIGHT);

        guidance_state_free(&st);
    }

    //
    // 7) Drop‐based U-turn + cooldown
    //
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 3,
            .drop_steps    = 2,
            .reverse_cd    = 2,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 0.0f
        };
        GuidanceState st;
        guidance_state_init(&st, &p);

        // seed with high mags
        gbufx[0] = 5.0f;  gbufy[0] = 0.0f;
        for (int i = 0; i < 3; i++)
            guidance_step(gbufx, gbufy, 0, 0, &st, &p);

        // two consecutive drops → U-turn
        gbufx[0] = 1.0f;  gbufy[0] = 0.0f;
        guidance_step(gbufx, gbufy, 0, 0, &st, &p);  // drop #1
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);  // drop #2 → U-turn
        RUN("drop-based U-turn", d == TURN_AROUND);

        // next call: hold straight during cooldown
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("hold straight during cooldown", d == STRAIGHT_AHEAD);

        // second cooldown tick: still straight
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("second cooldown step", d == STRAIGHT_AHEAD);

        // now cooldown expired → use a clear left-turn input
        gbufx[0] = 1.0f;  gbufy[0] = 1.0f;
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        RUN("post-cooldown normal steer (left)", d == TURN_LEFT);

        guidance_state_free(&st);
    }

    printf("ALL TESTS PASSED\n");
    return 0;
}
