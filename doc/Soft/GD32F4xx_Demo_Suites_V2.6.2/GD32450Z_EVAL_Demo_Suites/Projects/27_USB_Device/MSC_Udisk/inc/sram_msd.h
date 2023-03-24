/*!
    \file    sram_msd.h
    \brief   the header file of sram_msd.c

    \version 2020-08-01, V3.0.0, demo for GD32F4xx
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#ifndef __SRAM_MSD_H
#define __SRAM_MSD_H

#include "stdlib.h"

#define ISRAM_BLOCK_SIZE         512U
#define ISRAM_BLOCK_NUM          80U

/* function declarations */
/* read data from multiple blocks of SRAM */
uint32_t SRAM_ReadMultiBlocks  (uint8_t* pBuf,
                                 uint32_t ReadAddr,
                                 uint16_t BlkSize,
                                 uint32_t BlkNum);
/* write data to multiple blocks of SRAM */
uint32_t SRAM_WriteMultiBlocks (uint8_t* pBuf,
                                 uint32_t WriteAddr,
                                 uint16_t BlkSize,
                                 uint32_t BlkNum);

#endif /* __SRAM_MSD_H */
