
#include <stdbool.h>

typedef int sh_rmt_btn_idx_t;

void sh_rmt_btn_init(void);
void sh_rmt_btn_process(void);

bool sh_rmt_btn_reset_btn_pressed(void);

extern void sh_rmt_btn_evt(sh_rmt_btn_idx_t idx);
extern void sh_rmt_btn_rls_evt(sh_rmt_btn_idx_t idx);
