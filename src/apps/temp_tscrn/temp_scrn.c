//
// Created by bearh on 20.06.19.
//

#include "temp_scrn.h"

#include <stdio.h>

#include "../../lib/ft8xx/ft8xx.h"
#include "../../lib/ft8xx/ft8xx_copro.h"
#include "../../lib/ft8xx/ft8xx_dl.h"

int temp_scrn_display(int32_t temp1, int32_t temp2, int32_t setting1, int32_t setting2)
{
    int32_t temps[] = {temp1, temp2};
    int32_t settings[] = {setting1, setting2};

    FT8XX_COPRO_BLOCK
    {
        FT8XX_COPRO(cmd_dlstart);

        FT8XX_COPRO_ARGS(cmd, CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
        FT8XX_COPRO_ARGS(cmd, CLEAR(1, 1, 1));

        FT8XX_COPRO_ARGS(cmd, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));
        FT8XX_COPRO_ARGS(cmd, TAG(3));

        for (int i = 0; i < sizeof(temps) / sizeof(temps[0]); i++)
        {
            int32_t  temp = temps[i];
            uint32_t x    = 120;
            uint32_t y    = 120 + i * 40;

            if (temp < -50)
            {
                FT8XX_COPRO_ARGS(cmd_text, x, y, 29, 0, "Sensor error");
            }
            else
            {
                const unsigned int str_len = 6;
                char               str[str_len];
                int32_t            dec  = temp / 10;
                int32_t            frac = temp - dec * 10;

                frac = frac >= 0 ? frac : -frac;
                snprintf(str, str_len, "%d.%d", dec, frac);
                FT8XX_COPRO_ARGS(cmd_text, x, y, 31, 0, str);
            }
        }

        for (int i = 0; i < sizeof(settings) / sizeof(settings[0]); i++)
        {
            const unsigned int str_len = 6;
            char               str[str_len];

            int32_t  temp = settings[i];
            uint32_t x    = 370;
            uint32_t y    = 120 + i * 40;

            int32_t            dec  = temp / 10;
            int32_t            frac = temp - dec * 10;
            frac = frac >= 0 ? frac : -frac;
            snprintf(str, str_len, "%d.%d", dec, frac);
            FT8XX_COPRO_ARGS(cmd_text, x, y, 27, 0, str);
        }

        FT8XX_COPRO_ARGS(cmd, TAG(1));
        FT8XX_COPRO_ARGS(cmd_text, 320, 100, 31, 0, "+");
        FT8XX_COPRO_ARGS(cmd, TAG(2));
        FT8XX_COPRO_ARGS(cmd_text, 320, 140, 31, 0, "-");

        FT8XX_COPRO_ARGS(cmd, DISPLAY());
        FT8XX_COPRO(cmd_swap);
    }

    return 0;
}

