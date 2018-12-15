//
// Created by bearh on 13.12.18.
//

#ifndef OT_GPIO_MCBOR_CONST_H
#define OT_GPIO_MCBOR_CONST_H

#define INIT_BYTE_SIZE 1
enum {
    ADD_INFO_1_BYTE = 24,
    ADD_INFO_2_BYTE = 25,
    ADD_INFO_4_BYTE = 26,
    ADD_INFO_8_BYTE = 27,
};

enum {
    MAJ_TYPE_UINT        = 0,
    MAJ_TYPE_NEGINT      = (1 << 5),
    MAJ_TYPE_BYTE_STRING = (2 << 5),
    MAJ_TYPE_TEXT_STRING = (3 << 5),
    MAJ_TYPE_ARRAY       = (4 << 5),
    MAJ_TYPE_MAP         = (5 << 5),
    MAJ_TYPE_OPT         = (6 << 5),
    MAJ_TYPE_FLOAT_OTHER = (7 << 5),
    MAJ_TYPE_MASK        = (7 << 5),
};


#endif //OT_GPIO_MCBOR_CONST_H
