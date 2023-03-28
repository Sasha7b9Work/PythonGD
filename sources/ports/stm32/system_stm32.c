#include "py/mphal.h"
#include "boardctrl.h"
#include "powerctrl.h"


#define BIT(x)                  ((uint32_t)((uint32_t)0x01U<<(x)))
#define BITS(start, end)        ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end)))) 
#define REG32(addr)             (*(volatile uint32_t *)(uint32_t)(addr))

/* define startup timeout value of high speed crystal oscillator (HXTAL) */
#if !defined  (HXTAL_STARTUP_TIMEOUT)
#define HXTAL_STARTUP_TIMEOUT   ((uint16_t)0xFFFF)
#endif /* high speed crystal oscillator startup timeout */

#define AHB1_BUS_BASE           ((uint32_t)0x40020000U)             /*!< ahb1 base address                */
#define APB1_BUS_BASE           ((uint32_t)0x40000000U)             /*!< apb1 base address                */

#define CFG0_AHBPSC(regval)     (BITS(4,7) & ((uint32_t)(regval) << 4))
#define RCU_AHB_CKSYS_DIV1      CFG0_AHBPSC(0)                      /*!< AHB prescaler select CK_SYS */
                                
#define CFG0_APB2PSC(regval)    (BITS(13,15) & ((uint32_t)(regval) << 13))
#define RCU_APB2_CKAHB_DIV2     CFG0_APB2PSC(4)                     /*!< APB2 prescaler select CK_AHB/2 */
                                
#define CFG0_APB1PSC(regval)    (BITS(10,12) & ((uint32_t)(regval) << 10))
#define RCU_APB1_CKAHB_DIV4     CFG0_APB1PSC(5)                     /*!< APB1 prescaler select CK_AHB/4 */

#define RCU_BASE                (AHB1_BUS_BASE + 0x00003800U)  /*!< RCU base address                 */
#define RCU                     RCU_BASE
#define RCU_CTL                 REG32(RCU + 0x00U)                  /*!< control register */
#define RCU_CTL_HXTALEN         BIT(16)                             /*!< external high speed oscillator enable */
#define RCU_CTL_HXTALSTB        BIT(17)                             /*!< external crystal oscillator clock stabilization flag */
#define RCU_APB1EN              REG32(RCU + 0x40U)                  /*!< APB1 enable register */
#define RCU_APB1EN_PMUEN        BIT(28)                             /*!< PMU clock enable */
#define RCU_CFG0                REG32(RCU + 0x08U)                  /*!< clock configuration register 0 */
#define RCU_PLL                 REG32(RCU + 0x04U)                  /*!< PLL register */
#define RCU_CTL_PLLSTB          BIT(25)                             /*!< PLL Clock Stabilization Flag */
#define RCU_CTL_PLLEN           BIT(24)                             /*!< PLL enable */
#define RCU_CFG0_SCS            BITS(0,1)                           /*!< system clock switch */
#define RCU_ADDCTL              REG32(RCU + 0xC0U)                  /*!< Additional clock control register */
#define RCU_ADDCTL_PLL48MSEL    BIT(1)                              /*!< PLL48M clock selection */
#define RCU_PLL48MSRC_PLLQ      ((uint32_t)0x00000000U)             /*!< PLL48M source clock select PLLQ */
#define RCU_ADDCTL_CK48MSEL     BIT(0)                              /*!< 48MHz clock selection */
#define RCU_CK48MSRC_PLL48M     ((uint32_t)0x00000000U)             /*!< CK48M source clock select PLL48M */
                                
#define RCU_PLL_PLLSEL          BIT(22)                             /*!< PLL Clock Source Selection */
#define RCU_PLLSRC_HXTAL        RCU_PLL_PLLSEL                      /*!< HXTAL clock selected as source clock of PLL, PLLSAI, PLLI2S */
                                
#define CFG0_SCS(regval)        (BITS(0,1) & ((uint32_t)(regval) << 0))
#define RCU_CKSYSSRC_PLLP       CFG0_SCS(2)                         /*!< system clock source select PLLP */

#define CFG0_SCSS(regval)       (BITS(2,3) & ((uint32_t)(regval) << 2))
#define RCU_SCSS_PLLP           CFG0_SCSS(2)                        /*!< system clock source select PLLP */

#define RCU_REG_VAL(periph)     (REG32(RCU + ((uint32_t)(periph) >> 6)))

#define PMU_BASE                (APB1_BUS_BASE + 0x00007000U)       /*!< PMU base address                 */
#define PMU                     PMU_BASE                            /*!< PMU base address */
#define PMU_CTL                 REG32((PMU) + 0x00000000U)          /*!< PMU control register */
#define PMU_CTL_LDOVS           BITS(14,15)                         /*!< LDO output voltage select */
#define PMU_CTL_HDS             BIT(17)                             /*!< high-driver mode switch */
#define PMU_CS_HDRF             BIT(16)                             /*!< high-driver ready flag */
#define PMU_CS                  REG32((PMU) + 0x00000004U)          /*!< PMU control and status register */
#define PMU_CS_HDSRF            BIT(17)                             /*!< high-driver switch ready flag */
#define PMU_CTL_HDEN            BIT(16)                             /*!< high-driver mode enable */


#define AHB2EN_REG_OFFSET               0x34U                       /*!< AHB2 enable register offset */
#define RCU_REGIDX_BIT(regidx, bitpos)      (((uint32_t)(regidx) << 6) | (uint32_t)(bitpos))
#define RCU_USBFS  (RCU_REGIDX_BIT(AHB2EN_REG_OFFSET, 7U))                  /*!< USBFS clock */


#define RCU_BIT_POS(val)                    ((uint32_t)(val) & 0x1FU)


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

    RCU_REG_VAL(RCU_USBFS) |= BIT(RCU_BIT_POS(RCU_USBFS));
}
