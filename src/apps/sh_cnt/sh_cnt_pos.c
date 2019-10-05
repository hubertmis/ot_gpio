
#include "sh_cnt_pos.h"

#include <assert.h>

typedef enum
{
    SH_CNT_POS_REMOTE,
    SH_CNT_POS_LOCAL,

    SH_CNT_POS_NUM
} sh_cnt_pos_t;

uint16_t req_positions[SH_CNT_MOT_NUM][SH_CNT_POS_NUM];

static int update_position(sh_cnt_mot_idx_t idx, sh_cnt_pos_t pos_idx, uint16_t val)
{
    int          result;
    uint16_t     req_position   = SH_CNT_POS_STOP;
    sh_cnt_pos_t valid_pos_idx  = -1;

    assert(idx < SH_CNT_MOT_NUM);
    assert(pos_idx < SH_CNT_POS_NUM);

    // Verify if given value is correct
    switch (val)
    {
        case SH_CNT_POS_STOP:
        case SH_CNT_POS_UP:
        case SH_CNT_POS_DOWN:
            // Special cases that are correct
            break;

        default:
            if (!sh_cnt_mot_val_is_correct(idx, val)) return -1;
    }

    req_positions[idx][pos_idx] = val;

    for (int i = 0; i < SH_CNT_POS_NUM; i++)
    {
        if (req_positions[idx][i] != SH_CNT_POS_STOP)
        {
            req_position  = req_positions[idx][i];
            valid_pos_idx = i;
        }
    }

    if (valid_pos_idx <= pos_idx)
    {
        switch (req_position)
        {
            case SH_CNT_POS_STOP:
                result = 0;
                sh_cnt_mot_stop(idx);
                break;

            case SH_CNT_POS_UP:
                result = 0;
                sh_cnt_mot_up(idx);
                break;

            case SH_CNT_POS_DOWN:
                result = 0;
                sh_cnt_mot_down(idx);
                break;

            default:
                result = sh_cnt_mot_val(idx, req_position);
                break;
        }
    }
    else
    {
        result = 0;
    }

    return result;
}

void sh_cnt_pos_init(void)
{
    for (int i = 0; i < SH_CNT_MOT_NUM; i++)
    {
        for (int j = 0; j < SH_CNT_POS_NUM; j++)
        {
            req_positions[i][j] = SH_CNT_POS_STOP;
        }
    }
}

int sh_cnt_pos_remote_set(sh_cnt_mot_idx_t idx, uint16_t val)
{
    return update_position(idx, SH_CNT_POS_REMOTE, val);
}

int sh_cnt_pos_local_set(sh_cnt_mot_idx_t idx, uint16_t val)
{
    return update_position(idx, SH_CNT_POS_LOCAL, val);
}
