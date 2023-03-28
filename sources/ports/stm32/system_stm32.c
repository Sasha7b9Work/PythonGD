#include "py/mphal.h"
#include "boardctrl.h"
#include "powerctrl.h"


MP_WEAK void SystemClock_Config(void)
{
    uint32_t timeout = 0U;
    
    /* enable HXTAL */
    RCU_CTL |= RCU_CTL_HXTALEN;

    /* wait until HXTAL is stable or the startup time is longer than HXTAL_STARTUP_TIMEOUT */
    while((0U == (RCU_CTL & RCU_CTL_HXTALSTB)) && (HXTAL_STARTUP_TIMEOUT != timeout++)){
    }

    /* if fail */
    if(0U == (RCU_CTL & RCU_CTL_HXTALSTB)){
        while(1){
        }
    }

    RCU_APB1EN |= RCU_APB1EN_PMUEN;
    PMU_CTL |= PMU_CTL_LDOVS;
    /* HXTAL is stable */
    /* AHB = SYSCLK */
    RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
    /* APB2 = AHB/2 */
    RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
    /* APB1 = AHB/4 */
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV4;

    /* Configure the main PLL, PSC = 8, PLL_N = 336, PLL_P = 2, PLL_Q = 7 */ 
    RCU_PLL = (8U | (336 << 6U) | (((2 >> 1U) -1U) << 16U) |
                   (RCU_PLLSRC_HXTAL) | (7 << 24U));

    /* enable PLL */
    RCU_CTL |= RCU_CTL_PLLEN;

    /* wait until PLL is stable */
    while(0U == (RCU_CTL & RCU_CTL_PLLSTB)){
    }
  
    /* Enable the high-drive to extend the clock frequency to 168 Mhz */
    PMU_CTL |= PMU_CTL_HDEN;
    while(0U == (PMU_CS & PMU_CS_HDRF)){
    }
    
    /* select the high-drive mode */
    PMU_CTL |= PMU_CTL_HDS;
    while(0U == (PMU_CS & PMU_CS_HDSRF)){
    }

    /* select PLL as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_PLLP;

    /* wait until PLL is selected as system clock */
    while(0U == (RCU_CFG0 & RCU_SCSS_PLLP)){
    }
    
    
    
    // USB_FS

    uint32_t reg;

    // rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);

    reg = RCU_ADDCTL;
    /* reset the PLL48MSEL bit and set according to pll48m_clock_source */
    reg &= ~RCU_ADDCTL_PLL48MSEL;
    RCU_ADDCTL = (reg | RCU_PLL48MSRC_PLLQ);

    // rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);

    reg = RCU_ADDCTL;
    /* reset the CK48MSEL bit and set according to i2s_clock_source */
    reg &= ~RCU_ADDCTL_CK48MSEL;
    RCU_ADDCTL = (reg | RCU_CK48MSRC_PLL48M);

    // rcu_periph_clock_enable(RCU_USBFS);

    RCU_REG_VAL(periph) |= BIT(RCU_BIT_POS(RCU_USBFS));
}
