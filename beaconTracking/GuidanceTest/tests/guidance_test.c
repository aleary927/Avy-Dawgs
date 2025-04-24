#include <assert.h>
#include <stdio.h>
#include "guidance.h"

int main(void) {
    float gbufx[1], gbufy[1];
    Direction d;

    /* 1) STRAIGHT_AHEAD when perp ≪ par */
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 0.0f   /* no thresholding here */
        };
        GuidanceState st;
        assert(guidance_state_init(&st, &p));

        gbufx[0] = 0.0f;    /* 0 dB → lin = 1.0 */
        gbufy[0] = -40.0f;  /* -40 dB → lin ~0.01 */
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        assert(d == STRAIGHT_AHEAD);
        printf("[PASS] Straight ahead\n");

        guidance_state_free(&st);
    }

    /* 2) TURN_LEFT when perp ≈ par */
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
        assert(guidance_state_init(&st, &p));

        gbufx[0] = 0.0f;   /* 1.0 lin */
        gbufy[0] = 0.0f;   /* 1.0 lin */
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        assert(d == TURN_LEFT);
        printf("[PASS] Turn left\n");

        guidance_state_free(&st);
    }

    /* 3) min_valid_mag threshold: repeat last_dir on weak signal */
    {
        GuidanceParams p = {
            .buf_size      = 1,
            .hist_size     = 1,
            .drop_steps    = 100,
            .reverse_cd    = 1,
            .fwd_thresh    = 0.5f,
            .min_valid_mag = 1.5f  /* set threshold above √2≈1.414 */
        };
        GuidanceState st;
        assert(guidance_state_init(&st, &p));

        st.last_dir = TURN_RIGHT;
        gbufx[0] = 0.0f;   /* 1.0 lin */
        gbufy[0] = 0.0f;   /* 1.0 lin */
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        assert(d == TURN_RIGHT);
        printf("[PASS] min_valid_mag threshold\n");

        guidance_state_free(&st);
    }

    /* 4) TURN_AROUND in reverse mode (Ppar < 0) */
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
        assert(guidance_state_init(&st, &p));

        st.reverse_lock = true;
        gbufx[0] = 0.0f;   /* 1.0 lin, but flipped negative in code */
        gbufy[0] = 0.0f;   /* 1.0 lin */
        d = guidance_step(gbufx, gbufy, 0, 0, &st, &p);
        assert(d == TURN_AROUND);
        printf("[PASS] Turn around\n");

        guidance_state_free(&st);
    }

    printf("All tests passed!\n");
    return 0;
}
