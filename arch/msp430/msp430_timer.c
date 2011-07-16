
/**
 *  \file   msp430_timer.c
 *  \brief  MSP430 Timers definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "msp430.h"


/****************************************************/
/****************************************************/
/****************************************************/

// # DEBUG_TIMER defined in msp430_debug.h

#if DEBUG_TIMER  != 0
char *str_mode[] = 
  { "TIMER_STOP", "TIMER_UP", "TIMER_CONT", "TIMER_UD" };
char *str_clocksrc[] = 
  { "TxCLK", "ACLK", "SMCLK", "INTxCLK" };

#define TIMER_DEBUG_TIV 1
#define TIMER_DEBUG_2   1
#endif /* DEBUG_TIMER */


#if TIMER_DEBUG_2 != 0
char *str_cap[] = 
  { "compare", "capture" };
char *str_capturemode[] = 
  { "no capture", "cap. rising edge", "cap. falling edge", "cap. both edges" };
char *str_ccis[] = 
  { "CCIxA", "CCIxB", "GND", "Vcc" };
#   define HW_DMSG_2_DBG(x...) HW_DMSG_TIMER(x)
#else 
#   define HW_DMSG_2_DBG(x...) do { } while (0)
#endif

#if TIMER_DEBUG_TIV != 0
#   define HW_DMSG_TIMER_TIV(x...) HW_DMSG_TIMER(x)
#else
#   define HW_DMSG_TIMER_TIV(x...) HW_DMSG_TIMER(x)
#endif

/****************************************************/
/****************************************************/
/****************************************************/

void msp430_timerA3_set_tiv(void);
void msp430_timerB_set_tiv (void);

/****************************************************/
/* timer set clock div parameters                   */
/****************************************************/

#define SET_DIVBUFFER(TIMER,TIMERN,id)                                                 \
do {                                                                                   \
  MCU.TIMER.divbuffer    = 0;                                                          \
  MCU.TIMER.divuppermask = ~0 << id;                                                   \
  MCU.TIMER.divlowermask = ~0 ^ MCU.TIMER.divuppermask;                                \
  /*HW_DMSG_TIMER("msp430:" TIMERN ":    div id = %d\n",id);		                */ \
  /*HW_DMSG_TIMER("msp430:" TIMERN ":    uppermask = 0x%08x\n",MCU.TIMER.divuppermask); */ \
  /*HW_DMSG_TIMER("msp430:" TIMERN ":    lowermask = 0x%08x\n",MCU.TIMER.divlowermask); */ \
} while (0) 


/****************************************************/
/* write timer comparator control register          */
/****************************************************/

/* check if ifg flag has been set to 1 by software */
#define TCHKIFG_WRITE(TIMER,TIMERN,cctl,cctln,intr)			\
  do {									\
    if ((cc.b.ccie == 1) && (cc.b.ccifg == 1))				\
      {									\
	HW_DMSG_TIMER("msp430:" TIMERN ": checkifg " cctln		\
		      ".ccifg == 1, interrupt set [%"PRId64"\n",	\
		      MACHINE_TIME_GET_NANO());				\
	msp430_interrupt_set(intr);					\
      }									\
  } while (0)


#define TCCTLWRITE_BEGIN(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)	\
  case ADDR:								\
  {									\
  union TYPE cc,mcucc;							\
  cc.s = val;								\
  mcucc.s = MCU.TIMER.cctl[NUM].s;					\
  HW_DMSG_2_DBG("msp430:"TIMERN": "cctln" = 0x%04x [pc=0x%04x]\n",	\
                val & 0xffff, mcu_get_pc());				\
  if (cc.b.cm != mcucc.b.cm)						\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".cm     = %d (%s)\n",				\
		    cc.b.cm,str_capturemode[cc.b.cm]);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".cm     left to %d (%s)\n",			\
		    cc.b.cm,str_capturemode[cc.b.cm]);			\
    }									\
  if (cc.b.ccis != mcucc.b.ccis)					\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".ccis   = %d (%s)\n",				\
                    cc.b.ccis,str_ccis[cc.b.ccis]);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".ccis   left to %d (%s)\n",			\
                    cc.b.ccis,str_ccis[cc.b.ccis]);			\
    }									\
  if (cc.b.scs != mcucc.b.scs)						\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".scs    = %d\n",cc.b.scs);				\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".scs    left to %d\n",cc.b.scs);			\
    }



#define TCCTLWRITE_END(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)	\
  if (cc.b.cap != mcucc.b.cap)						\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".cap    = %d (%s)\n",				\
                    cc.b.cap,str_cap[cc.b.cap]);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".cap    left to %d (%s)\n",			\
                    cc.b.cap,str_cap[cc.b.cap]);			\
    }									\
  if (cc.b.outmod != mcucc.b.outmod)					\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".outmod = %d\n",cc.b.outmod);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".outmod left to %d\n",cc.b.outmod);		\
    }									\
  if (cc.b.ccie != mcucc.b.ccie)					\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".ccie   = %d\n",cc.b.ccie);			\
      TCHKIFG_WRITE(TIMER,TIMERN,cctl[NUM],cctln,intr);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
                    ".ccie   left to %d\n",cc.b.ccie);			\
    }									\
  if (cc.b.cci != mcucc.b.cci)						\
    {									\
      /* read only  bit */						\
      HW_DMSG_TIMER("msp430:" TIMERN ":    " cctln			\
		    ".cci    = %d",cc.b.cci);				\
      HW_DMSG_TIMER("msp430:" TIMERN					\
		    ":      * this bit should be read only */\n");	\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".cci    = %d\n",cc.b.cci);				\
    }									\
  if (cc.b.out != mcucc.b.out)						\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".out    = %d\n",cc.b.out);				\
      if (cc.b.outmod == TIMER_OUTMOD_OUTPUT)				\
	{								\
	  MCU.TIMER.out[NUM] = cc.b.out;				\
	  HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			NUM,MCU.TIMER.out[NUM]);			\
	}								\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".out    left to %d\n",cc.b.out);			\
    }									\
  if (cc.b.cov != mcucc.b.cov)						\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".cov    = %d\n",cc.b.cov);				\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".cov    left to %d\n",cc.b.cov);			\
    }									\
  if (cc.b.ccifg != mcucc.b.ccifg)					\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".ccifg  = %d\n",cc.b.ccifg);			\
      TCHKIFG_WRITE(TIMER,TIMERN,cctl[NUM],cctln,intr);			\
    }									\
  else									\
    {									\
      HW_DMSG_2_DBG("msp430:" TIMERN ":    " cctln			\
		    ".ccifg  left to %d\n",cc.b.ccifg);			\
    }									\
  MCU.TIMER.cctl[NUM].s = val;						\
  msp430_##TIMER##_set_tiv();						\
  }									\
break;



#define TIMERA_TCCTLWRITE(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)	\
  TCCTLWRITE_BEGIN(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)		\
  if (cc.b.scci != mcucc.b.scci)					\
    { /* timer A */							\
      HW_DMSG_TIMER("msp430:" TIMERN ": " cctln ".scci = %d\n",		\
		    cc.b.scci);						\
    }									\
  TCCTLWRITE_END(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)


#define TIMERB_TCCTLWRITE(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)	\
  TCCTLWRITE_BEGIN(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)		\
  if (cc.b.clld != mcucc.b.clld)					\
    { /* timer B */							\
      HW_DMSG_TIMER("msp430:" TIMERN ": " cctln ".clld = %d\n",		\
		    cc.b.clld);						\
      ERROR("msp430:" TIMERN ": " cctln ".clld not supported\n");	\
    }									\
  TCCTLWRITE_END(ADDR,TYPE,TIMER,TIMERN,cctl,cctln,NUM,intr)


/****************************************************/
/* timer check ifg                                  */
/****************************************************/

#define TCHKIFG(TIMER,TIMERN,cctl,cctln,intr)				\
  if ((MCU.TIMER.cctl.b.ccie == 1) && (MCU.TIMER.cctl.b.ccifg == 1))	\
    {									\
      HW_DMSG_TIMER("msp430:" TIMERN ": checkifg " cctln		\
		    ".ccifg == 1, interrupt set [%"PRId64"\n",		\
		    MACHINE_TIME_GET_NANO());				\
      msp430_interrupt_set(intr);					\
      return 1;								\
    }


char* timerA_tiv_to_str(int val)
{
  switch (val) {
  case 0x0: return "none";
  case 0x2: return "TACCR1";
  case 0x4: return "TACCR2";
  case 0x6: return "reserved";
  case 0x8: return "reserved";
  case 0xa: return "timer overflow";
  case 0xc: return "reserved";
  case 0xe: return "reserved";
  default:  return "unknown";
  }
  return "unknown";
}


char* timerB_tiv_to_str(int val)
{
  switch (val) {
  case 0x0: return "none";
  case 0x2: return "TBCCR1";
  case 0x4: return "TBCCR2";
  case 0x6: return "TBCCR3";
  case 0x8: return "TBCCR4";
  case 0xa: return "TBCCR5";
  case 0xc: return "TBCCR6";
  case 0xe: return "timer overflow";
  default:  return "unknown";
  }
  return "unknown";
}

#define ifsetA(TIMER,TIMERN,cctl,val)					\
  if (MCU.TIMER.cctl.b.ccifg && MCU.TIMER.cctl.b.ccie)			\
    {									\
      MCU.TIMER.tiv.s = val;						\
      HW_DMSG_TIMER_TIV("msp430:"TIMERN": tiv set to 0x%02x = %s [%"PRId64"]\n", \
                        val & 0x7,timerA_tiv_to_str(val),MACHINE_TIME_GET_NANO()); \
    }

#define ifsetB(TIMER,TIMERN,cctl,val)					\
  if (MCU.TIMER.cctl.b.ccifg && MCU.TIMER.cctl.b.ccie)			\
    {									\
      MCU.TIMER.tiv.s = val;						\
      HW_DMSG_TIMER_TIV("msp430:"TIMERN": tiv set to 0x%02x = %s [%"PRId64"]\n", \
                    val & 0x7,timerB_tiv_to_str(val),MACHINE_TIME_GET_NANO()); \
    }


#define ifzero(TIMER,TIMERN,cctl,cctln,ccr)				\
  if (MCU.TIMER.cctl.b.ccifg)						\
    {									\
      MCU.TIMER.cctl.b.ccifg = 0;					\
      HW_DMSG_TIMER("msp430:"TIMERN": "cctln				\
		    ".ifg set to 0      [%"PRId64"]\n",			\
		    MACHINE_TIME_GET_NANO());				\
    }


/****************************************************/
/* timer compare                                    */
/****************************************************/

#define COMPARE_UNREACHABLE 0x10000u
#define COMPARE_UNREACHABLE_DOWN -1

/* compare ok when     TR >= b_CCR && TR >= CCR */
#define TIMER_COMPARE(TIMER,TIMERN,TR,CCR,TCCTL,NUM,INTR)		\
  if ( /*(MCU.TIMER.CCR > 0) &&   CCR can be compared to 0 !! */	\
      (MCU.TIMER.TCCTL[NUM].b.cap == 0) && /* compare mode */		\
      (MCU.TIMER.TCCTL[NUM].b.ccifg == 0) )				\
    {									\
      /* TR counts to CCR[NUM] */					\
      if ((MCU.TIMER.TR >= MCU.TIMER.CCR[NUM]) &&			\
	  (MCU.TIMER.TR >  MCU.TIMER.b_##CCR[NUM]))			\
	{								\
	  MCU.TIMER.TCCTL[NUM].b.ccifg = 1;				\
	  HW_DMSG_TIMER("msp430:"TIMERN": cmp%d ifg set (ccr=0x%04x, "	\
			"tr=0x%04x, b_ccr=0x%06x) [%"PRId64"]\n",	\
			NUM,MCU.TIMER.CCR[NUM],MCU.TIMER.TR,		\
			MCU.TIMER.b_##CCR[NUM],				\
			MACHINE_TIME_GET_NANO());			\
	  msp430_##TIMER##_set_tiv();					\
	  MCU.TIMER.equ[NUM] = 1;					\
	  /* put unreachable value, back to 0 on wrap */		\
	  MCU.TIMER.b_##CCR [NUM] = COMPARE_UNREACHABLE;		\
	  /*HW_DMSG_2_DBG("msp430:"TIMERN": b_ccr set to 0x%06x\n",*/	\
	  /*MCU.TIMER.b_##CCR[NUM]);*/					\
	  /* set output according to outmod */				\
	  switch (MCU.TIMER.TCCTL[NUM].b.outmod)			\
	    {								\
	    case TIMER_OUTMOD_OUTPUT       :				\
	      break;							\
	    case TIMER_OUTMOD_SET          :				\
	    case TIMER_OUTMOD_SET_RESET    :				\
	      MCU.TIMER.out[NUM] = 1;					\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    case TIMER_OUTMOD_TOGGLE_RESET :			        \
	    case TIMER_OUTMOD_TOGGLE       :				\
	    case TIMER_OUTMOD_TOGGLE_SET   :				\
	      MCU.TIMER.out[NUM] = 1 - MCU.TIMER.out[NUM];		\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    case TIMER_OUTMOD_RESET        :				\
	    case TIMER_OUTMOD_RESET_SET    :				\
	      MCU.TIMER.out[NUM] = 0;					\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    }								\
	  /* FIXME: CCI is latched in SCCI except for TimerB         */ \
	  if (MCU.TIMER.TCCTL[NUM].b.ccie == 1)				\
	    {								\
	      HW_DMSG_TIMER("msp430:"TIMERN": interrupt %d from "	\
			    "TIMER_COMPARE %d tiv 0x%x\n",		\
			    INTR,NUM,MCU.TIMER.tiv.s);			\
	      msp430_interrupt_set(INTR);				\
	    }								\
	}								\
      else if (MCU.TIMER.b_##CCR [NUM] != COMPARE_UNREACHABLE)		\
	{								\
	  MCU.TIMER.b_##CCR [NUM] = MCU.TIMER.TR;			\
	}								\
    }                                                                                  

/* compare ok when  CCR => TR  &&  b_CCR >= TR */
#define TIMER_COMPARE_DOWN(TIMER,TIMERN,TR,CCR,TCCTL,NUM,INTR,MAX)	\
  if ((MCU.TIMER.CCR[NUM] > 0) && (MCU.TIMER.CCR[NUM] <= MAX) &&	\
      (MCU.TIMER.TCCTL[NUM].b.cap == 0) && /* compare mode */		\
      (MCU.TIMER.TCCTL[NUM].b.ccifg == 0) )				\
    {									\
      if ((MCU.TIMER.CCR[NUM] >= MCU.TIMER.TR) &&			\
	  (MCU.TIMER.b_##CCR[NUM] > MCU.TIMER.TR))			\
	{								\
	  MCU.TIMER.TCCTL[NUM].b.ccifg = 1;					\
	  HW_DMSG_TIMER("msp430:"TIMERN": cmp%d ifg set (ccr=0x%04x, tr=0x%04x, b_ccr=0x%06x) [%"PRId64"]\n", \
			NUM,MCU.TIMER.CCR[NUM],MCU.TIMER.TR,		\
			MCU.TIMER.b_##CCR[NUM],				\
			MACHINE_TIME_GET_NANO());			\
	  msp430_##TIMER##_set_tiv();					\
	  MCU.TIMER.equ[NUM] = 1;					\
	  /* put unreachable value, back to 0 on wrap */		\
	  MCU.TIMER.b_##CCR[NUM] = COMPARE_UNREACHABLE_DOWN;		\
	  HW_DMSG_2_DBG("msp430:"TIMERN": b_ccr set to 0x%06x\n",	\
			MCU.TIMER.b_##CCR[NUM]);			\
	  /* set output according to outmod */				\
	  switch (MCU.TIMER.TCCTL[NUM].b.outmod)			\
	    {								\
	    case TIMER_OUTMOD_OUTPUT       :				\
	      break;							\
	    case TIMER_OUTMOD_SET          :				\
	    case TIMER_OUTMOD_SET_RESET    :				\
	      MCU.TIMER.out[NUM] = 1;					\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    case TIMER_OUTMOD_TOGGLE_RESET :			        \
	    case TIMER_OUTMOD_TOGGLE       :				\
	    case TIMER_OUTMOD_TOGGLE_SET   :				\
	      MCU.TIMER.out[NUM] = 1 - MCU.TIMER.out[NUM];		\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    case TIMER_OUTMOD_RESET        :				\
	    case TIMER_OUTMOD_RESET_SET    :				\
	      MCU.TIMER.out[NUM] = 0;					\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	      break;							\
	    }								\
	  /* FIXME: CCI is latched in SCCI except for TimerB         */	\
	  if (MCU.TIMER.TCCTL[NUM].b.ccie == 1)				\
	    {								\
	      HW_DMSG_TIMER("msp430:"TIMERN": interrupt %d from TIMER_COMPARE_DOWN %d tiv 0x%x\n", \
			    INTR,NUM,MCU.TIMER.tiv.s);			\
	      msp430_interrupt_set(INTR);				\
	    }								\
	}								\
      else if (MCU.TIMER.b_##CCR[NUM] != COMPARE_UNREACHABLE_DOWN)	\
	{								\
	  MCU.TIMER.b_##CCR[NUM] = MCU.TIMER.TR;			\
	}								\
    }                                                                                  


#define TIMER_COMPARE_WRAPS(TIMER,TIMERN,CCR,TCCTL,NUM)			\
  do {									\
    HW_DMSG_2_DBG("msp430:"TIMERN": b_ccr%d wraps = 0\n",NUM);		\
    /* set output according to outmod */				\
    switch (MCU.TIMER.TCCTL[NUM].b.outmod)				\
      {									\
      case TIMER_OUTMOD_OUTPUT       :					\
      case TIMER_OUTMOD_SET          :					\
      case TIMER_OUTMOD_TOGGLE       :					\
      case TIMER_OUTMOD_RESET        :					\
	break;								\
      case TIMER_OUTMOD_TOGGLE_RESET :					\
      case TIMER_OUTMOD_SET_RESET    :					\
	MCU.TIMER.out[NUM] = 0;						\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	break;								\
      case TIMER_OUTMOD_TOGGLE_SET   :					\
      case TIMER_OUTMOD_RESET_SET    :					\
	MCU.TIMER.out[NUM] = 1;						\
	      HW_DMSG_2_DBG("msp430:"TIMERN": out%d = %d\n",		\
			    NUM,MCU.TIMER.out[NUM]);			\
	break;								\
      }									\
    MCU.TIMER.b_##CCR [NUM] = 0;					\
  } while (0)

#define TIMER_COMPARE_WRAPS_DOWN(TIMER,TIMERN,CCR,NUM,MAX)		\
  do {									\
    HW_DMSG_2_DBG("msp430:"TIMERN": b_ccr%d wraps = 0x%x\n",MAX);	\
    MCU.TIMER.b_##CCR [NUM] = MAX;					\
  } while (0)



/************/
/* timer A3 */
/************/
#define WRITE_TIMERA3_CCR(NUM)						\
  do {									\
    MCU.timerA3.taccr[NUM]  = val & 0x00ffffl;				\
    if (MCU.timerA3.tar > MCU.timerA3.taccr[NUM])			\
      MCU.timerA3.b_taccr[NUM] = COMPARE_UNREACHABLE;			\
    else								\
      MCU.timerA3.b_taccr[NUM] = 0;					\
    HW_DMSG_TIMER("msp430:timerA3: taccr%d  = 0x%04x "			\
		  "(TAR = 0x%04x) [%"PRId64"]\n",			\
		  NUM,MCU.timerA3.taccr[NUM],MCU.timerA3.tar,		\
		  MACHINE_TIME_GET_NANO());				\
  } while(0)


/***********/
/* timer B */
/***********/
#define WRITE_TIMERB_CCR(NUM)						\
  do {									\
    MCU.timerB.tbccr[NUM]  = val & 0x00ffffl;				\
    MCU.timerB.tbcl[NUM]   = val & 0x00ffffl;				\
    if (MCU.timerB.tbr > MCU.timerB.tbcl[NUM])				\
      MCU.timerB.b_tbcl[NUM] = COMPARE_UNREACHABLE;			\
    else								\
      MCU.timerB.b_tbcl[NUM] = 0;					\
    HW_DMSG_TIMER("msp430:"TIMERBNAME": tbccr%d  = 0x%04x "		\
		  "(TBR = 0x%04x) [%"PRId64"]\n",			\
		  NUM,MCU.timerB.tbccr[NUM],MCU.timerB.tbr,		\
		  MACHINE_TIME_GET_NANO());				\
  }while(0)

#define TBCCRWRITE(NUM)							\
  case TBCCR##NUM :							\
  WRITE_TIMERB_CCR(NUM);						\
  break;


/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer A3 ******************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timera3)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

enum timerA3_addr_t {
  TAIV      = 0x012e, /* read only */

  TACTL     = 0x0160,
  TACCTL0   = 0x0162,
  TACCTL1   = 0x0164,
  TACCTL2   = 0x0166,
  TA_RES1   = 0x0168, /* reserved */
  TA_RES2   = 0x016a, /* reserved */
  TA_RES3   = 0x016c, /* reserved */
  TA_RES4   = 0x016e, /* reserved */

  TAR       = 0x0170,
  TACCR0    = 0x0172,
  TACCR1    = 0x0174,
  TACCR2    = 0x0176,
  TA_RES5   = 0x0178, /* reserved */
  TA_RES6   = 0x017a, /* reserved */
  TA_RES7   = 0x017c, /* reserved */
  TA_RES8   = 0x017e  /* reserved */
};

#define TIMER_A3_START 0x0160
#define TIMER_A3_END   0x017e

enum timerA5_addr_t {
  TA1IV     = 0x011e,
  TA1CTL    = 0x0180,
  TA1CCTL0  = 0x0182,
  TA1CCTL1  = 0x0184,
  TA1CCTL2  = 0x0186,
  TA1CCTL3  = 0x0188,
  TA1CCTL4  = 0x018a,
  TA1R      = 0x0190,
  TA1CCR0   = 0x0192,
  TA1CCR1   = 0x0194,
  TA1CCR2   = 0x0196,
  TA1CCR3   = 0x0198,
  TA1CCR4   = 0x019a
};

#define TIMER_A5_START  0x180
#define TIMER_A5_END    0x19e

void msp430_timerA3_create()
{
  msp430_io_register_addr8(TAIV,msp430_timerA3_read8,msp430_timerA3_write8);
  msp430_io_register_range8(TIMER_A3_START,TIMER_A3_END+1,msp430_timerA3_read8,msp430_timerA3_write8);

  msp430_io_register_addr16(TAIV,msp430_timerA3_read,msp430_timerA3_write);
  msp430_io_register_range16(TIMER_A3_START,TIMER_A3_END,msp430_timerA3_read,msp430_timerA3_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA3_reset(void)
{
  memset(&MCU.timerA3,0,sizeof(struct msp430_timerA3_t));
  SET_DIVBUFFER(timerA3,"timerA3",MCU.timerA3.tactl.b.id);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA3_set_tiv(void)
{
       ifsetA(timerA3,"timerA3",tacctl[1],0x02)    /* highest */
  else ifsetA(timerA3,"timerA3",tacctl[2],0x04)
  else if (MCU.timerA3.tactl.b.taifg)             /* lowest */
    {
      MCU.timerA3.tiv.s = 0x0a;
      HW_DMSG_TIMER_TIV("msp430:timerA3: tiv set to 0x0a\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_timerA3_reset_highest_intr(void)
{
       ifzero(timerA3,"timerA3",tacctl[1],"tacctl1",taccr1)  /* reset highest interrupt */
  else ifzero(timerA3,"timerA3",tacctl[2],"tacctl2",taccr2)
  else if (MCU.timerA3.tactl.b.taifg)
    {
      MCU.timerA3.tactl.b.taifg = 0;
      HW_DMSG_TIMER("msp430:timerA3: tactl.ifg set to 0\n");
    }
  msp430_timerA3_set_tiv();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/**
 *         5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  
 * TACTL  [  unused   | aa| bb| cc|u|a|b|c] 
 *
 *
 * aa TASSELx timer A clock source select
 *              00 TACLK   /  external
 *              01 ACLK    /  internal from basic clock 
 *              10 SMCLK   /  internal from basic clock
 *              11 INCLK   /  external
 * bb ID      timer A input divider 
 *              00  /1
 *              01  /2
 *              10  /4
 *              11  /8
 * cc MC      mode control
 *              00 stop mode 
 *              01 up mode
 *              10 continuous
 *              11 up/down
 *
 *  a TACLR   timer A clear
 *  b TAIE    timer A interrupt enable
 *              0 interrupt disable
 *              1 interrupt enable
 *  c TAIFG   timer A interrupt flag
 *              0 no interrupt pending
 *              1 interrupt pending
 */

/** 
 * clock source
 * ACLK   Auxiliary clock -> basic clock module
 * SMCLK  Sub System master clock -> basic clock module
 * external
 * TACLK
 * INCLK
 */

#define TAR_MAX_LIMIT 0x00ffffl

void msp430_timerA3_update(void)
{
  int clock;
  int tar_inc;
  /***************/
  /* Timer block */
  /***************/
  if (MCU.timerA3.tactl.b.mc == TIMER_STOP)
    return ;
  
  clock = 0;
  switch (MCU.timerA3.tactl.b.tassel)
    {
    case TIMER_SOURCE_TxCLK:
      ERROR("msp430:timerA3: source TACLK not implemented\n");
      break;
    case TIMER_SOURCE_ACLK:
      clock = MCU_CLOCK.ACLK_increment;
      break;
    case TIMER_SOURCE_SMCLK:
      clock = MCU_CLOCK.SMCLK_increment;
      break;
    case TIMER_SOURCE_INTxCLK:
      ERROR("msp430:timerA3: source INTACLK not implemented\n");
      break;
    }

  MCU.timerA3.divbuffer +=clock;

  if ((clock == 0) || ((MCU.timerA3.divbuffer & MCU.timerA3.divuppermask) == 0))
    {
      return;
    }

  /*HW_DMSG_TIMER("msp430:timerA3: divbuffer %d div %d mask %x clocks %d",
    MCU.timerA3.divbuffer, MCU.timerA3.tactl.b.id, MCU.timerA3.divlowermask, clock);*/
  tar_inc = MCU.timerA3.divbuffer >> MCU.timerA3.tactl.b.id; // div 
  MCU.timerA3.divbuffer &= MCU.timerA3.divlowermask;         // mod
  /*HW_DMSG_TIMER(" / tar_inc %d divbuffer %d\n",tar_inc, MCU.timerA3.divbuffer);*/
  /*HW_DMSG_TIMER("msp430:timerA3: tar_inc %d\n",tar_inc); */

  switch (MCU.timerA3.tactl.b.mc)
    {
    case TIMER_STOP:
      /* should not be reached due to return a few lines above */
      break;

    case TIMER_UP:     /* UP counter */
      if (MCU.timerA3.taccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UP */
	{
	  MCU.timerA3.tar += tar_inc;
	  /* HW_DMSG_TIMER("msp430:timerA3: tar (%x) += %d\n",MCU.timerA3.tar,tar_inc); */

          /**************************/
          /* capture/compare blocks */
          /**************************/
          TIMER_COMPARE(timerA3,"timerA3:taccr1:",tar,taccr,tacctl,1,INTR_TIMERA3_1)
          TIMER_COMPARE(timerA3,"timerA3:taccr2:",tar,taccr,tacctl,2,INTR_TIMERA3_1)

	  if (MCU.timerA3.tar >= MCU.timerA3.taccr[0])
	    {
	      /* (ccr0 - 1) -> ccr0 */
	      MCU.timerA3.tacctl[0].b.ccifg = 1;
	      msp430_timerA3_set_tiv();
	      if (MCU.timerA3.tacctl[0].b.ccie == 1)
		{
		  HW_DMSG_TIMER("msp430:timerA3: set interrupt TIMERA3_0 from TIMER_UP\n");
		  msp430_interrupt_set(INTR_TIMERA3_0);
		}
	      
	      /*  ccr0      -> 0    */
	      MCU.timerA3.tactl.b.taifg   = 1;
	      msp430_timerA3_set_tiv();
	      if (MCU.timerA3.tactl.b.taie == 1)
		{
		  HW_DMSG_TIMER("msp430:timerA3: set interrupt TIMERA3_1 from TIMER_UP\n");
		  msp430_interrupt_set(INTR_TIMERA3_1);
		}
	      MCU.timerA3.tar -= MCU.timerA3.taccr[0];
	      HW_DMSG_2_DBG("msp430:timerA3: up mode wraps to 0 ===============================\n");
              TIMER_COMPARE_WRAPS(timerA3,"timerA3:taccr1",taccr,tacctl,1);
	      TIMER_COMPARE_WRAPS(timerA3,"timerA3:taccr2",taccr,tacctl,2);
	    }
	}
      break;

    case TIMER_CONT:    /* Continuous counter */
      MCU.timerA3.tar += tar_inc;
      /* HW_DMSG_TIMER("msp430:timerA3: tar skip to 0x%04x (inc 0x%04x)\n",MCU.timerA3.tar,tar_inc); */

      /**************************/
      /* capture/compare blocks */
      /**************************/
      TIMER_COMPARE(timerA3,"timerA3",tar,taccr,tacctl,0,INTR_TIMERA3_0)
      TIMER_COMPARE(timerA3,"timerA3",tar,taccr,tacctl,1,INTR_TIMERA3_1)
      TIMER_COMPARE(timerA3,"timerA3",tar,taccr,tacctl,2,INTR_TIMERA3_1)

      if (MCU.timerA3.tar >= TAR_MAX_LIMIT)
	{
	  MCU.timerA3.tactl.b.taifg = 1;
	  msp430_timerA3_set_tiv();
	  if (MCU.timerA3.tactl.b.taie == 1)
	    {
	      HW_DMSG_TIMER("msp430:timerA3: interrupt TIMERA3_1 from TIMER_CONT (tar 0x%06x) [%"PRId64"]\n",
			    MCU.timerA3.tar,MACHINE_TIME_GET_NANO());
	      msp430_interrupt_set(INTR_TIMERA3_1);
	    }
	  MCU.timerA3.tar -= 0xffffu;
	  /* contig mode bad wraps */
	  HW_DMSG_TIMER("msp430:timerA3: contig mode wraps to 0 ===============================\n");
	  TIMER_COMPARE_WRAPS(timerA3,"timerA3",taccr,tacctl,0);
	  TIMER_COMPARE_WRAPS(timerA3,"timerA3",taccr,tacctl,1);
	  TIMER_COMPARE_WRAPS(timerA3,"timerA3",taccr,tacctl,2);
	}
      break;

    case TIMER_UD:      /* UP/DOWN counter */
      if (MCU.timerA3.taccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UD */
	{
	  if (MCU.timerA3.udmode == TIMER_UD_UP)
	    {
	      MCU.timerA3.tar += tar_inc;
	      if (MCU.timerA3.tar >= MCU.timerA3.taccr[0])
		{
		  /* we are going UP, so the timer wraps and is going down */
		  MCU.timerA3.udmode = TIMER_UD_DOWN;
		  MCU.timerA3.tar    = MCU.timerA3.taccr[0];
		  MCU.timerA3.tacctl[0].b.ccifg = 1;
		  msp430_timerA3_set_tiv();
		  if (MCU.timerA3.tacctl[0].b.ccie == 1)
		    {
		      HW_DMSG_TIMER("msp430:timerA3: set interrupt TIMERA3_1 from TIMER_UD in UP mode\n");
		      msp430_interrupt_set(INTR_TIMERA3_0);
		    }
		  HW_DMSG_TIMER("msp430:timerA3: Up/Down mode wraps to max ===============================\n");
		  TIMER_COMPARE_WRAPS_DOWN(timerA3,"timerA3",taccr,1,MCU.timerA3.taccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerA3,"timerA3",taccr,2,MCU.timerA3.taccr[0]);
		}
	      else
		{
		  TIMER_COMPARE(timerA3,"timerA3",tar,taccr,tacctl,1,INTR_TIMERA3_1)
		  TIMER_COMPARE(timerA3,"timerA3",tar,taccr,tacctl,2,INTR_TIMERA3_1)
		}
	    }
	  else /* timer is down */
	    {
	      MCU.timerA3.tar -= tar_inc;
	      if (MCU.timerA3.tar <= 0)
		{
		  /* we are going down, we wraps and start up */
		  MCU.timerA3.udmode        = TIMER_UD_UP;
		  MCU.timerA3.tar           = 0;
		  MCU.timerA3.tactl.b.taifg = 1;
		  msp430_timerA3_set_tiv();
		  if (MCU.timerA3.tactl.b.taie == 1)
		    {
		      HW_DMSG_TIMER("msp430:timerA3: set interrupt TIMERB_1 from TIMER_UD in DOWN mode\n");
		      msp430_interrupt_set(INTR_TIMERA3_1);
		    }
		  HW_DMSG_TIMER("msp430:timerA3: Up/Down mode wraps to 0 ===============================\n");
		  TIMER_COMPARE_WRAPS(timerA3,"timerA3",taccr,tacctl,1);
		  TIMER_COMPARE_WRAPS(timerA3,"timerA3",taccr,tacctl,2);
		}
	      else
		{
		  TIMER_COMPARE_DOWN(timerA3,"timerA3",tar,taccr,tacctl,1,INTR_TIMERA3_1,MCU.timerA3.taccr[0])
                  TIMER_COMPARE_DOWN(timerA3,"timerA3",tar,taccr,tacctl,2,INTR_TIMERA3_1,MCU.timerA3.taccr[0])
		}
	    }
	}
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA3_capture(void)
{
  int i;
  /* 
     at this time we don't care about:
     SCS  : synchroneous capture source
     SCCI : Synchronized capture/compare input
  */ 
  for(i=0 ; i < TIMERA_COMPARATOR ; i++)
    {
      if (MCU.timerA3.tacctl[ i ].b.cap ==1 && 
          MCU.timerA3.tacctl[ i ].b.cm > 0)
	{
	  switch (MCU.timerA3.tacctl[ i ].b.ccis)
	    {
	      /*
	        CCR0: A=P1.1 B=P2.2
	        CCR1: A=P1.2 B=CAOUT
	        CCR2: A=P1.3 B=ACLK
	      */
	    case 0: /* CCIxA = TA1 */

#if defined(MSP430f1611)
	      /* on msp430f1611 this capture pin is p1.(i+1) */
	      if (MCU.digiIO.in_updated[0] & (0x2 << i))
#else
	      if (1)
		ERROR("msp430:" TIMERBNAME ": device specific capture ports undefined\n");
	      else
#endif
		{
		  int rising_edge  = (MCU.timerA3.tacctl[ i ].b.cm == 1) &&  (MCU.digiIO.in[0] & (0x2 << i));
		  int falling_edge = (MCU.timerA3.tacctl[ i ].b.cm == 2) && !(MCU.digiIO.in[0] & (0x2 << 1));
		  int both_edges   = (MCU.timerA3.tacctl[ i ].b.cm == 3);
		  if (rising_edge || falling_edge || both_edges) 
		    {
		      MCU.timerA3.taccr [ i ] = MCU.timerA3.tar;
		      MCU.timerA3.tacctl[ i ].b.ccifg = 1;
		      msp430_timerA3_set_tiv();
		      if (MCU.timerA3.tacctl[ i ].b.ccie == 1)
			{
			  HW_DMSG_TIMER("msp430:" TIMERANAME ": set interrupt TIMERA_1 from CAPTURE CCI%dA\n",i);
			  msp430_interrupt_set(INTR_TIMERA3_1);
			}
		    }
		}

	      break;
	    case 1: /* CCIxB */
#if !defined(MSP430f1611)
	      ERROR("msp430:" TIMERANAME ": device specific capture ports undefined CCI%dB\n",i);
#elif defined(MSP430f1611)
	      if ((i == 0)) /* P2.2 */
		{
		  ERROR("msp430:" TIMERANAME ": device specific capture ports undefined CCI%dB\n",i);
		}
	      else if ((i == 1)) /* CAOUT */
		{
		  ERROR("msp430:" TIMERANAME ": device specific capture ports undefined CCI%dB\n",i);
		}
	      else if ((i == 2) && (MCU_CLOCK.ACLK_increment > 0))
		{
		  MCU.timerA3.taccr [ i ] = MCU.timerA3.tar;
		  MCU.timerA3.tacctl[ i ].b.ccifg = 1;
		  msp430_timerA3_set_tiv();
		  if (MCU.timerA3.tacctl[ i ].b.ccie == 1)
		    {
		      HW_DMSG_TIMER("msp430:" TIMERANAME ": set interrupt TIMERA3_1 from CAPTURE CCI%dB\n",i);
		      msp430_interrupt_set(INTR_TIMERA3_1);
		    }
		}
#endif
	      break;
	    case 2: /* GND */
	      ERROR("msp430:" TIMERANAME ": capture not implemented on this port (GND)\n");
	      break;
	    case 3: /* Vcc */
	      ERROR("msp430:" TIMERANAME ": capture not implemented on this port (Vcc)\n");
	      break;
	    }
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA3_write8(uint16_t addr, int8_t val)
{
  msp430_timerA3_write(addr,val);
}

void msp430_timerA3_write(uint16_t addr, int16_t val)
{
  switch ((enum timerA3_addr_t)addr)
    {
    case TAIV      : /* read only */
      /* although this register is read only, we can have a write on it */
      msp430_timerA3_reset_highest_intr();      
      HW_DMSG_TIMER("msp430:timerA3: taiv write, reset highest intr\n");
      break;

    case TACTL:
      {
	union {
	  uint16_t         s;
	  struct tactl_t   b;
	} tactl;
	
	HW_DMSG_2_DBG("msp430:timerA3: tactl   = 0x%04x\n",val);
	tactl.s = val;

	if (tactl.b.taclr) /* this one must be first as it resets divider and ssel */
	  {
	    MCU.timerA3.tactl.b.id = 0; 
	    MCU.timerA3.tar        = 0;
	    tactl.b.taclr          = 0;
	    SET_DIVBUFFER(timerA3,"timerA3",0); 
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.taclr clear\n");
	  }

	if (tactl.b.tassel != MCU.timerA3.tactl.b.tassel)
	  {
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.tassel set to %d (%s)\n",
			  tactl.b.tassel,str_clocksrc[tactl.b.tassel]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:timerA3:    tactl.tassel left to %d (%s)\n",
			  tactl.b.tassel,str_clocksrc[tactl.b.tassel]);
	  }

	if (tactl.b.id != MCU.timerA3.tactl.b.id)
	  {
	    SET_DIVBUFFER(timerA3,"timerA3",tactl.b.id);
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.id set to %d (DIV = %d)\n",tactl.b.id,1<<tactl.b.id);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:timerA3:    tactl.id left to %d (DIV = %d)\n",tactl.b.id,1<<tactl.b.id);
	  }

	if (tactl.b.mc != MCU.timerA3.tactl.b.mc)
	  {
	    if ((tactl.b.mc == TIMER_UP) && (MCU.timerA3.tar > MCU.timerA3.taccr[0]))
	      {
		MCU.timerA3.tar    = 0; /* restart from zero */
	      }
	    MCU.timerA3.udmode = TIMER_UD_UP;
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.mc going to mode %d (%s)\n",
			  tactl.b.mc,str_mode[tactl.b.mc]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:timerA3:    tactl.mc left to mode %d (%s)\n",
			  tactl.b.mc,str_mode[tactl.b.mc]);
	  }

	if (tactl.b.taie != MCU.timerA3.tactl.b.taie)
	  {
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.ie set to %d\n",tactl.b.taie);
	    if ((tactl.b.taie == 1) && (tactl.b.taifg == 1))
	      {
		HW_DMSG_TIMER("msp430:timerA3: checkifg tactl.taifg == 1, interrupt set\n");
		msp430_interrupt_set(INTR_TIMERA3_1);
	      }
	  }

	if (tactl.b.taifg != MCU.timerA3.tactl.b.taifg)
	  {
	    HW_DMSG_TIMER("msp430:timerA3:    tactl.tbifg set to %d\n",tactl.b.taifg);
	    if ((tactl.b.taie == 1) && (tactl.b.taifg == 1))
	      {
		HW_DMSG_TIMER("msp430:timerA3: checkifg tactl.taifg == 1, interrupt set\n");
		msp430_interrupt_set(INTR_TIMERA3_1);
	      }
	  }

	MCU.timerA3.tactl.s = tactl.s;
	msp430_timerA3_set_tiv();
      }
      break;

      TIMERA_TCCTLWRITE(TACCTL0,tacctlu_t,timerA3,"timerA3",tacctl,"tacctl0",0,INTR_TIMERA3_0)
      TIMERA_TCCTLWRITE(TACCTL1,tacctlu_t,timerA3,"timerA3",tacctl,"tacctl1",1,INTR_TIMERA3_1)
      TIMERA_TCCTLWRITE(TACCTL2,tacctlu_t,timerA3,"timerA3",tacctl,"tacctl2",2,INTR_TIMERA3_1)


    case TAR:
      MCU.timerA3.tar = val & 0xffffu;
      HW_DMSG_TIMER("msp430:timerA3: tar     = 0x%04x [%"PRId64"]\n",
		    MCU.timerA3.tar,MACHINE_TIME_GET_NANO());
      break;

    case TACCR0    :
      if ((MCU.timerA3.tactl.b.mc == TIMER_UP) || (MCU.timerA3.tactl.b.mc == TIMER_UD))
	{
	  if ((MCU.timerA3.taccr[0] == 0) && (val > 0))
	    {
	      MCU.timerA3.udmode = TIMER_UD_UP;
	      MCU.timerA3.tar    = 0;
	      HW_DMSG_TIMER("msp430:timerA3: taccr0 > 0, restarts the timer\n");
	    }
	  else if (val < MCU.timerA3.tar) 
	    {
	      if (MCU.timerA3.tactl.b.mc == TIMER_UP)
		{
		  MCU.timerA3.tar = 0;
		  HW_DMSG_TIMER("msp430:timerA3: taccr0 > tar, restarts from 0\n");
		}
	      else if (MCU.timerA3.udmode == TIMER_UD_UP)
		{
		  MCU.timerA3.udmode = TIMER_UD_DOWN;
		  HW_DMSG_TIMER("msp430:timerA3: taccr0 > tar, going mode down\n");
		}
	    }
	}
      WRITE_TIMERA3_CCR(0);
      break;

    case TACCR1    : WRITE_TIMERA3_CCR(1); break;
    case TACCR2    : WRITE_TIMERA3_CCR(2); break;

    case TA_RES1   : /* reserved */
    case TA_RES2   : /* reserved */
    case TA_RES3   : /* reserved */
    case TA_RES4   : /* reserved */
    case TA_RES5   : /* reserved */
    case TA_RES6   : /* reserved */
    case TA_RES7   : /* reserved */
    case TA_RES8   : /* reserved */
      ERROR("msp430:timerA3: bad write address [0x%04x]\n",addr);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_timerA3_read8(uint16_t addr)
{
  return msp430_timerA3_read(addr) & 0xff;
}

int16_t msp430_timerA3_read(uint16_t addr)
{
  int16_t ret;
  switch ((enum timerA3_addr_t) addr)
    {
    case TACTL     : ret = MCU.timerA3.tactl.s;     break;
    case TACCTL0   : ret = MCU.timerA3.tacctl[0].s; break;
    case TACCTL1   : ret = MCU.timerA3.tacctl[1].s; break;
    case TACCTL2   : ret = MCU.timerA3.tacctl[2].s; break;
    case TAR       : ret = MCU.timerA3.tar;         break;
    case TACCR0    : ret = MCU.timerA3.taccr[0];    break;
    case TACCR1    : ret = MCU.timerA3.taccr[1];    break;
    case TACCR2    : ret = MCU.timerA3.taccr[2];    break;
    case TAIV      : 
      ret = MCU.timerA3.tiv.s;
      HW_DMSG_TIMER("msp430:timerA3: read TAIV [0x%04x] = 0x%04x\n",addr,ret);
      msp430_timerA3_reset_highest_intr();
      break;
    case TA_RES1   : /* reserved */
    case TA_RES2   : /* reserved */
    case TA_RES3   : /* reserved */
    case TA_RES4   : /* reserved */
    case TA_RES5   : /* reserved */
    case TA_RES6   : /* reserved */
    case TA_RES7   : /* reserved */
    case TA_RES8   : /* reserved */
    default        :
      ERROR("msp430:timerA3: bad read address 0x%04x\n",addr);
      ret = 0;
      break;
    }
  /*  HW_DMSG_TIMER("msp430:timerA3: read [0x%04x] = 0x%04x\n",addr,ret); */
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_timerA3_chkifg(void)
{
  int ret = 0;

  TCHKIFG(timerA3,"timerA3",tacctl[0],"tacctl0",INTR_TIMERA3_0)

  if (MCU.timerA3.tiv.s)
    {
       if ((MCU.timerA3.tactl.b.taie == 1) && (MCU.timerA3.tactl.b.taifg == 1))
	{
	  HW_DMSG_TIMER("msp430:timerA3: checkifg tactl.taifg == 1, interrupt set\n");
	  msp430_interrupt_set(INTR_TIMERA3_1);
	  return 1;
	}
      
      TCHKIFG(timerA3,"timerA3",tacctl[1],"tacctl1",INTR_TIMERA3_1)
      TCHKIFG(timerA3,"timerA3",tacctl[2],"tacctl2",INTR_TIMERA3_1)
    }
  return ret;
}
#endif

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer A5 ******************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timera5)

void msp430_timerA5_create()
{
  // check 8 address space
  msp430_io_register_addr16(TA1IV,msp430_timerA5_read,msp430_timerA5_write);
  msp430_io_register_range16(TIMER_A5_START,TIMER_A5_END,msp430_timerA5_read,msp430_timerA5_write);
}

void msp430_timerA5_reset(void)
{
  memset(&MCU.timerA5,0,sizeof(struct msp430_timerA3_t));
}

/* ************************************************** */

void msp430_timerA5_update(void)
{
}

/* ************************************************** */

void msp430_timerA5_capture(void)
{
}

/* ************************************************** */

int16_t msp430_timerA5_read  (uint16_t addr)
{
  ERROR("msp430:timera5: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */

void msp430_timerA5_write (uint16_t addr, int16_t val)
{
  ERROR("msp430:timera5: write [0x%04x] = 0x%04x, block not implemented\n",addr,val);
}
#endif

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer B ********************************************************************* */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timerb3) || defined(__msp430_have_timerb7)

/* ************************************************** */

static int timerB_limit[] = { TBR_16, TBR_12, TBR_10, TBR_8 };
static void msp430_timerB_set_limit(int i)
{
  MCU.timerB.tbr_limit = timerB_limit[i];
  HW_DMSG_TIMER("msp430:" TIMERBNAME ": set tbr limit to 0x%04x\n",MCU.timerB.tbr_limit);
}

enum timerB_addr_t {
 TBIV      = 0x011e,
 TBCTL     = 0x0180,
 TBCCTL0   = 0x0182,
 TBCCTL1   = 0x0184,
 TBCCTL2   = 0x0186,
 TBCCTL3   = 0x0188,
 TBCCTL4   = 0x018a,
 TBCCTL5   = 0x018c,
 TBCCTL6   = 0x018e,
 TBR       = 0x0190,
 TBCCR0    = 0x0192,
 TBCCR1    = 0x0194,
 TBCCR2    = 0x0196,
 TBCCR3    = 0x0198,
 TBCCR4    = 0x019a,
 TBCCR5    = 0x019c,
 TBCCR6    = 0x019e
};

#if defined(__msp430_have_timerb3)
#define TIMER_B_START  0x180 // TODO
#define TIMER_B_END    0x19e // TODO
#elif defined(__msp430_have_timerb7)
#define TIMER_B_START  0x180
#define TIMER_B_END    0x19e
#endif

void msp430_timerB_create(void)
{
  msp430_io_register_addr16(TBIV,msp430_timerB_read,msp430_timerB_write);
  msp430_io_register_range16(TIMER_B_START,TIMER_B_END,msp430_timerB_read,msp430_timerB_write);
}

void msp430_timerB_reset(void)
{
  memset(&MCU.timerB,0,sizeof(struct msp430_timerB_t));
  SET_DIVBUFFER(timerB,TIMERBNAME,MCU.timerB.tbctl.b.id);
  msp430_timerB_set_limit(MCU.timerB.tbctl.b.cntl);
}

/* ************************************************** */

void msp430_timerB_set_tiv(void)
{ /* tbcctl0 excluded -> int 0 */
       ifsetB(timerB,TIMERBNAME,tbcctl[1],0x02)   /* highest */
  else ifsetB(timerB,TIMERBNAME,tbcctl[2],0x04)
#if defined(__msp430_have_timerb7)
  else ifsetB(timerB,TIMERBNAME,tbcctl[3],0x06)
  else ifsetB(timerB,TIMERBNAME,tbcctl[4],0x08)
  else ifsetB(timerB,TIMERBNAME,tbcctl[5],0x0a)
  else ifsetB(timerB,TIMERBNAME,tbcctl[6],0x0c)
#endif
  else if (MCU.timerB.tbctl.b.tbifg)              /* lowest */
    {
      MCU.timerB.tiv.s = 0x0e;
      HW_DMSG_TIMER("msp430:"TIMERBNAME": tiv set to 0x0e\n");
    }
}

/* ************************************************** */

static void msp430_timerB_reset_highest_intr(void)
{ /* tbcctl0 excluded -> int 0 */
       ifzero(timerB,TIMERBNAME,tbcctl[1],"tbcctl1",tbcl[1])   /* reset highest interrupt */
  else ifzero(timerB,TIMERBNAME,tbcctl[2],"tbcctl2",tbcl[2])
#if defined(__msp430_have_timerb7)
  else ifzero(timerB,TIMERBNAME,tbcctl[3],"tbcctl3",tbcl[3])
  else ifzero(timerB,TIMERBNAME,tbcctl[4],"tbcctl4",tbcl[4])
  else ifzero(timerB,TIMERBNAME,tbcctl[5],"tbcctl5",tbcl[5])
  else ifzero(timerB,TIMERBNAME,tbcctl[6],"tbcctl6",tbcl[6])
#endif
  else if (MCU.timerB.tbctl.b.tbifg)
    {
      MCU.timerB.tbctl.b.tbifg = 0;
      HW_DMSG_TIMER("msp430:"TIMERBNAME": tbctl.ifg set to 0\n");
    }
  msp430_timerB_set_tiv();
}

/* ************************************************** */

void msp430_timerB_update(void)
{
  int clock;
  int tbr_inc;
  /***************/
  /* Timer block */
  /***************/
  if (MCU.timerB.tbctl.b.mc == TIMER_STOP)
    return ;

  clock = 0;
  switch (MCU.timerB.tbctl.b.tbssel)
    {
    case TIMER_SOURCE_TxCLK:
      ERROR("msp430:" TIMERBNAME ": source TBCLK not implemented\n");
      break;
    case TIMER_SOURCE_ACLK:
      clock = MCU_CLOCK.ACLK_increment;
      break;
    case TIMER_SOURCE_SMCLK:
      clock = MCU_CLOCK.SMCLK_increment;
      break;
    case TIMER_SOURCE_INTxCLK:
      ERROR("msp430:" TIMERBNAME ": source INTBCLK not implemented\n");
      break;
    }
  
  MCU.timerB.divbuffer +=clock;

  if ((clock == 0) || ((MCU.timerB.divbuffer & MCU.timerB.divuppermask) == 0))
    {
      return;
    }

  tbr_inc = MCU.timerB.divbuffer >> MCU.timerB.tbctl.b.id;
  MCU.timerB.divbuffer &= MCU.timerB.divlowermask; 
  /* HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbr_inc %d\n",tbr_inc); */

  switch (MCU.timerB.tbctl.b.mc)
    {
    case TIMER_STOP:
      /* should not be reached */
      break;

    case TIMER_UP:     /* UP counter */
      if (MCU.timerB.tbccr[0] > 0) /* timer is stopped if tbccr0 == 0 in UP */
	{
	  MCU.timerB.tbr += tbr_inc;
	  /* HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbr (%x) += %d\n",MCU.timerB.tbr,tbr_inc); */

          /**************************/
          /* capture/compare blocks */
          /**************************/
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,1,INTR_TIMERB_1)
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,2,INTR_TIMERB_1)
#if defined(__msp430_have_timerb7)
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,3,INTR_TIMERB_1)
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,4,INTR_TIMERB_1)
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,5,INTR_TIMERB_1)
          TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,6,INTR_TIMERB_1)
#endif

	  if (MCU.timerB.tbr >= MCU.timerB.tbccr[0])
	    {
	      /* (ccr0 - 1) -> ccr0 */
	      MCU.timerB.tbcctl[0].b.ccifg = 1;
	      msp430_timerB_set_tiv();
	      if (MCU.timerB.tbcctl[0].b.ccie == 1)
		{
		  msp430_interrupt_set(INTR_TIMERB_0);
		}
	      
	      /*  ccr0      -> 0    */
	      MCU.timerB.tbctl.b.tbifg   = 1;
	      msp430_timerB_set_tiv();
	      if (MCU.timerB.tbctl.b.tbie == 1)
		{
		  HW_DMSG_TIMER("Set interrupt TIMERB_1 from TIMER_UP\n");
		  msp430_interrupt_set(INTR_TIMERB_1);
		}
	      MCU.timerB.tbr -= MCU.timerB.tbccr[0];
	      HW_DMSG_TIMER("msp430:"TIMERBNAME": up mode wraps to 0 ===============================\n");
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,1);
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,2);
#if defined(__msp430_have_timerb7)
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,3);
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,4);
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,5);
              TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,6);
#endif
	    }
	}
      break;

    case TIMER_CONT:    /* Continuous counter */
      MCU.timerB.tbr += tbr_inc;
      /* HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbr +%d -> 0x%04x\n",tbr_inc,MCU.timerB.tbr); */

      /**************************/
      /* capture/compare blocks */
      /**************************/
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,0,INTR_TIMERB_0)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,1,INTR_TIMERB_1)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,2,INTR_TIMERB_1)
#if defined(__msp430_have_timerb7)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,3,INTR_TIMERB_1)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,4,INTR_TIMERB_1)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,5,INTR_TIMERB_1)
      TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,6,INTR_TIMERB_1)
#endif

      if (MCU.timerB.tbr >= MCU.timerB.tbr_limit)
	{
	  MCU.timerB.tbctl.b.tbifg = 1;
	  msp430_timerB_set_tiv();
	  if (MCU.timerB.tbctl.b.tbie == 1)
	    {
	      HW_DMSG_TIMER("msp430:"TIMERBNAME": set interrupt TIMERB_1 from TIMER_CONT\n");
	      msp430_interrupt_set(INTR_TIMERB_1);
	    }
	  MCU.timerB.tbr -= MCU.timerB.tbr_limit;
	  /* contig mode bad wraps */
	  HW_DMSG_TIMER("msp430:"TIMERBNAME": contig mode wraps to 0 ===============================\n");
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,0);
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,1);
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,2);
#if defined(__msp430_have_timerb7)
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,3);
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,4);
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,5);
	  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,6);
#endif
	}
      break;

    case TIMER_UD:      /* UP/DOWN counter */
      if (MCU.timerB.tbccr[0] > 0) /* timer is stopped if tbccr0 == 0 in UD */
	{
	  if (MCU.timerB.udmode == TIMER_UD_UP)  
	    {
	      MCU.timerB.tbr += tbr_inc;
	      if (MCU.timerB.tbr >= MCU.timerB.tbccr[0])
		{
		  /* we are going UP, so the timer wraps and is going down */
		  MCU.timerB.udmode = TIMER_UD_DOWN;
		  MCU.timerB.tbr    = MCU.timerB.tbccr[0];
		  MCU.timerB.tbcctl[0].b.ccifg = 1;
		  msp430_timerB_set_tiv();
		  if (MCU.timerB.tbcctl[0].b.ccie == 1)
		    {
		      HW_DMSG_TIMER("msp430:"TIMERBNAME": set interrupt TIMERB_0 from TIMER_UD in UP mode tiv = 0x%x\n",MCU.timerB.tiv.s);
		      msp430_interrupt_set(INTR_TIMERB_0);
		    }
		  HW_DMSG_TIMER("msp430:"TIMERBNAME": Up/Down mode wraps to max ===============================\n");
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,1,MCU.timerB.tbccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,2,MCU.timerB.tbccr[0]);
#if defined(__msp430_have_timerb7)
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,3,MCU.timerB.tbccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,4,MCU.timerB.tbccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,5,MCU.timerB.tbccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerB,TIMERBNAME,tbcl,6,MCU.timerB.tbccr[0]);
#endif
		}
	      else
		{
		  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,1,INTR_TIMERB_1)
                  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,2,INTR_TIMERB_1)
#if defined(__msp430_have_timerb7)
                  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,3,INTR_TIMERB_1)
                  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,4,INTR_TIMERB_1)
                  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,5,INTR_TIMERB_1)
                  TIMER_COMPARE(timerB,TIMERBNAME,tbr,tbcl,tbcctl,6,INTR_TIMERB_1)
#endif
		}
	    }
	  else /* timer is down */
	    {
	      MCU.timerB.tbr -= tbr_inc;
	      if (MCU.timerB.tbr <= 0)
		{
		  /* we are going down, we wraps and start up */
		  MCU.timerB.udmode        = TIMER_UD_UP;
		  MCU.timerB.tbr           = 0;
		  MCU.timerB.tbctl.b.tbifg = 1;
		  msp430_timerB_set_tiv();
		  if (MCU.timerB.tbctl.b.tbie == 1)
		    {
		      HW_DMSG_TIMER("msp430:"TIMERBNAME": set interrupt TIMERB_1 from TIMER_UD in DOWN mode\n");
		      msp430_interrupt_set(INTR_TIMERB_1);
		    }
		  HW_DMSG_TIMER("msp430:"TIMERBNAME": Up/Down mode wraps to 0 ===============================\n");
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,1);
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,2);
#if defined(__msp430_have_timerb7)
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,3);
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,4);
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,5);
		  TIMER_COMPARE_WRAPS(timerB,TIMERBNAME,tbcl,tbcctl,6);
#endif
		}
              else
		{
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,1,INTR_TIMERB_1,MCU.timerB.tbccr[0])
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,2,INTR_TIMERB_1,MCU.timerB.tbccr[0])
#if defined(__msp430_have_timerb7)
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,3,INTR_TIMERB_1,MCU.timerB.tbccr[0])
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,4,INTR_TIMERB_1,MCU.timerB.tbccr[0])
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,5,INTR_TIMERB_1,MCU.timerB.tbccr[0])
                  TIMER_COMPARE_DOWN(timerB,TIMERBNAME,tbr,tbcl,tbcctl,6,INTR_TIMERB_1,MCU.timerB.tbccr[0])
#endif
		}
	    }
	}
      break;
    }
}

/* ************************************************** */

#define TBCCRWRITE_ERROR(NUM)						\
  case TBCCR##NUM :							\
  ERROR("msp430:" TIMERBNAME ": tbccr%d not present\n",NUM);		\
  break;


#define TBCCTLWRITE_ERROR(NUM)						\
  case TBCCTL##NUM :							\
  ERROR("msp430:" TIMERBNAME ": tbcctl%d not present\n",NUM);		\
  break;


/* ************************************************** */

void msp430_timerB_capture(void)
{
  int i;
  /* 
     at this time we don't care about:
     SCS  : synchroneous capture source
     SCCI : Synchronized capture/compare input
  */ 
  for(i=0 ; i < TIMERB_COMPARATOR ; i++)
    {
      if (MCU.timerB.tbcctl[ i ].b.cap == 1 && 
          MCU.timerB.tbcctl[ i ].b.cm > 0)
	{
	  switch (MCU.timerB.tbcctl[ i ].b.ccis)
	    {
	    case 0: /* CCIxA = TB1 */

#if defined(MSP430f1611)
	      /* on msp430f1611 this capture pin is p4.i */
	      if (MCU.digiIO.in_updated[3] & (0x1 << i))
#else
	      if (1)
		ERROR("msp430:" TIMERBNAME ": device specific capture ports undefined\n");
	      else
#endif
		{
		  int rising_edge  = (MCU.timerB.tbcctl[ i ].b.cm == 1) &&  (MCU.digiIO.in[3] & (0x1 << i));
		  int falling_edge = (MCU.timerB.tbcctl[ i ].b.cm == 2) && !(MCU.digiIO.in[3] & (0x1 << 1));
		  int both_edges   = (MCU.timerB.tbcctl[ i ].b.cm == 3);
		  if (rising_edge || falling_edge || both_edges) 
		    {
		      MCU.timerB.tbccr [ i ] = MCU.timerB.tbr;
		      MCU.timerB.tbcctl[ i ].b.ccifg = 1;
		      msp430_timerB_set_tiv();
		      if (MCU.timerB.tbcctl[ i ].b.ccie == 1)
			{
			  HW_DMSG_TIMER("msp430:" TIMERBNAME ": set interrupt TIMERB_1 from CAPTURE CCI%dA\n",i);
			  msp430_interrupt_set(INTR_TIMERB_1);
			}
		    }
		}
	      break;
	    case 1: /* CCIxB */
#if !defined(MSP430f1611)
	      ERROR("msp430:" TIMERBNAME ": device specific capture ports undefined\n");
	      /* on msp430f1611 this capture pin is p4.i */
#elif defined(MSP430f1611)
	      if ((i < 6) && (MCU.digiIO.in_updated[3] & (0x1 << i)))
		{
		  int rising_edge  = (MCU.timerB.tbcctl[ i ].b.cm == 1) &&  (MCU.digiIO.in[3] & (0x1 << i));
		  int falling_edge = (MCU.timerB.tbcctl[ i ].b.cm == 2) && !(MCU.digiIO.in[3] & (0x1 << 1));
		  int both_edges   = (MCU.timerB.tbcctl[ i ].b.cm == 3);
		  if (rising_edge || falling_edge || both_edges) 
		    {
		      MCU.timerB.tbccr [ i ] = MCU.timerB.tbr;
		      MCU.timerB.tbcctl[ i ].b.ccifg = 1;
		      msp430_timerB_set_tiv();
		      if (MCU.timerB.tbcctl[ i ].b.ccie == 1)
			{
			  HW_DMSG_TIMER("msp430:" TIMERBNAME ": set interrupt TIMERB_1 from CAPTURE CCI%dB\n",i);
			  msp430_interrupt_set(INTR_TIMERB_1);
			}
		    }
		}
	      else if ((i == 6) && (MCU_CLOCK.ACLK_increment > 0)) 
		{ /* we flipped ACLK so we have done both falling and rising edge */
		  MCU.timerB.tbccr [ i ] = MCU.timerB.tbr;
		  MCU.timerB.tbcctl[ i ].b.ccifg = 1;
		  msp430_timerB_set_tiv();
		  if (MCU.timerB.tbcctl[ i ].b.ccie == 1)
		    {
		      HW_DMSG_TIMER("msp430:" TIMERBNAME ": set interrupt TIMERB_1 from CAPTURE CCI%dB\n",i);
		      msp430_interrupt_set(INTR_TIMERB_1);
		    }
		}
#endif
	      break;
	    case 2: /* GND */
	      HW_DMSG_TIMER("msp430:" TIMERBNAME ": capture not implemented on this port\n");
	      ERROR("msp430:" TIMERBNAME ": capture not implemented on this port (GND)\n");
	      break;
	    case 3: /* Vcc */
	      HW_DMSG_TIMER("msp430:" TIMERBNAME ": capture not implemented on this port\n");
	      ERROR("msp430:" TIMERBNAME ": capture not implemented on this port (Vcc)\n");
	      break;
	    }
	}
    }
}


/* ************************************************** */

void msp430_timerB_write (uint16_t addr, int16_t val)
{
  switch ((enum timerB_addr_t)addr)
    {
    case TBIV: /* read only */
      /* although this register is read only, we can have a write on it */
      msp430_timerB_reset_highest_intr();
      HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbiv write, reset highest intr\n");
      break;

    case TBCTL:
      {
	union {
	  uint16_t       s;
	  struct tbctl_t b;
	} tbctl;
	
	HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbctl   = 0x%04x\n",val);
	tbctl.s = val;

	if (tbctl.b.tbclr)  /* this one must be first as it resets divider and ssel */
	  {
	    MCU.timerB.tbctl.b.id = 0;
	    MCU.timerB.tbr        = 0;
	    tbctl.b.tbclr         = 0;
	    SET_DIVBUFFER(timerB,TIMERBNAME,0);
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.tbclr    clear\n");
	  }

	if (tbctl.b.tbclgrp != MCU.timerB.tbctl.b.tbclgrp)
	  {
	    ERROR("msp430:" TIMERBNAME ": TBCLx group not implemented\n");
	  }

	if (tbctl.b.cntl != MCU.timerB.tbctl.b.cntl)
	  {
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.cntl     set to %d (0x%04x)\n",
			  tbctl.b.cntl,timerB_limit[tbctl.b.cntl]);
	    msp430_timerB_set_limit(tbctl.b.cntl);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:" TIMERBNAME ":    tbctl.cntl     left to %d (0x%04x)\n",
			  tbctl.b.cntl,timerB_limit[tbctl.b.cntl]);
	  }

	if (tbctl.b.tbssel != MCU.timerB.tbctl.b.tbssel)
	  {
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.tbssel   set to %d (%s)\n",
			  tbctl.b.tbssel,str_clocksrc[tbctl.b.tbssel]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:" TIMERBNAME ":    tbctl.tbssel   left to %d (%s)\n",
			  tbctl.b.tbssel,str_clocksrc[tbctl.b.tbssel]);
	  }

	if (tbctl.b.id != MCU.timerB.tbctl.b.id)
	  {
	    SET_DIVBUFFER(timerB,TIMERBNAME,tbctl.b.id);
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.id       set to %d (DIV = %d)\n",tbctl.b.id,1<<tbctl.b.id);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:" TIMERBNAME ":    tbctl.id       left to %d (DIV = %d)\n",tbctl.b.id,1<<tbctl.b.id);
	  }

	if (tbctl.b.mc != MCU.timerB.tbctl.b.mc)
	  {
	    if ((tbctl.b.mc == TIMER_UP) && (MCU.timerB.tbr > MCU.timerB.tbccr[0]))
	      {
		MCU.timerB.tbr    = 0; /* restart from zero */
	      }
	    MCU.timerB.udmode = TIMER_UD_UP;
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.mc       going to mode %d (%s)\n",
			  tbctl.b.mc,str_mode[tbctl.b.mc]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:" TIMERBNAME ":    tbctl.mc       left to mode %d (%s)\n",
			  tbctl.b.mc,str_mode[tbctl.b.mc]);
	  }

	if (tbctl.b.tbie != MCU.timerB.tbctl.b.tbie)
	  {
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.ie       set to %d\n",tbctl.b.tbie);
	    if ((tbctl.b.tbie == 1) && (tbctl.b.tbifg == 1))
	      {
		HW_DMSG_TIMER("msp430:timerB: checkifg tbctl.tbifg == 1, interrupt set [%"PRId64"]\n", 
			      MACHINE_TIME_GET_NANO());
		msp430_interrupt_set(INTR_TIMERB_1);
	      }
	  }

	if (tbctl.b.tbifg != MCU.timerB.tbctl.b.tbifg)
	  {
	    HW_DMSG_TIMER("msp430:" TIMERBNAME ":    tbctl.tbifg    set to %d\n",tbctl.b.tbifg);
	    if ((tbctl.b.tbie == 1) && (tbctl.b.tbifg == 1))
	      {
		HW_DMSG_TIMER("msp430:timerB: checkifg tbctl.tbifg == 1, interrupt set [%"PRId64"]\n", 
			      MACHINE_TIME_GET_NANO());
		msp430_interrupt_set(INTR_TIMERB_1);
	      }
	  }

	MCU.timerB.tbctl.s = tbctl.s;
	msp430_timerB_set_tiv();
      }
      break;

      TIMERB_TCCTLWRITE(TBCCTL0,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl0",0,INTR_TIMERB_0)
      TIMERB_TCCTLWRITE(TBCCTL1,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl1",1,INTR_TIMERB_1)
      TIMERB_TCCTLWRITE(TBCCTL2,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl2",2,INTR_TIMERB_1)
#if defined(__msp430_have_timerb7)
      TIMERB_TCCTLWRITE(TBCCTL3,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl3",3,INTR_TIMERB_1)
      TIMERB_TCCTLWRITE(TBCCTL4,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl4",4,INTR_TIMERB_1)
      TIMERB_TCCTLWRITE(TBCCTL5,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl5",5,INTR_TIMERB_1)
      TIMERB_TCCTLWRITE(TBCCTL6,tbcctlu_t,timerB,TIMERBNAME,tbcctl,"tbcctl6",6,INTR_TIMERB_1)
#else
      TBCCTLWRITE_ERROR(3)
      TBCCTLWRITE_ERROR(4)
      TBCCTLWRITE_ERROR(5)
      TBCCTLWRITE_ERROR(6)
#endif

    case TBR:
      MCU.timerB.tbr = val & MCU.timerB.tbr_limit;
      HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbr    = 0x%04x [%"PRId64"]\n",
		    MCU.timerB.tbr,MACHINE_TIME_GET_NANO());
      break;

    case TBCCR0:
      if ((MCU.timerB.tbctl.b.mc == TIMER_UP) || (MCU.timerB.tbctl.b.mc == TIMER_UD))
	{
	  if ((MCU.timerB.tbccr[0] == 0) && (val > 0))
	    {
	      MCU.timerB.udmode = TIMER_UD_UP;
	      MCU.timerB.tbr    = 0;
	      HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbccr0 > 0, restarts the timer\n");
	    }
	  else if (val < MCU.timerB.tbr) 
	    {
	      if (MCU.timerB.tbctl.b.mc == TIMER_UP)
		{
		  MCU.timerB.tbr = 0;
		  HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbccr0 > tbr, restarts from 0\n");
		}
	      else if (MCU.timerB.udmode == TIMER_UD_UP)
		{
		  MCU.timerB.udmode = TIMER_UD_DOWN;
		  HW_DMSG_TIMER("msp430:" TIMERBNAME ": tbccr0 > tar, going mode down\n");
		}
	    }
	}
      WRITE_TIMERB_CCR(0);
      break;

      TBCCRWRITE(1)
      TBCCRWRITE(2)
#if defined(__msp430_have_timerb7)
      TBCCRWRITE(3)
      TBCCRWRITE(4)
      TBCCRWRITE(5)
      TBCCRWRITE(6)
#else
      TBCCRWRITE_ERROR(3)
      TBCCRWRITE_ERROR(4)
      TBCCRWRITE_ERROR(5)
      TBCCRWRITE_ERROR(6)
#endif
    }
}
 
/* ************************************************** */

int16_t 
msp430_timerB_read  (uint16_t addr)
{
  int16_t ret;
  switch ((enum timerB_addr_t) addr)
    {
    case TBCTL     : ret = MCU.timerB.tbctl.s;   break;
    case TBCCTL0   : ret = MCU.timerB.tbcctl[0].s; break;
    case TBCCTL1   : ret = MCU.timerB.tbcctl[1].s; break;
    case TBCCTL2   : ret = MCU.timerB.tbcctl[2].s; break;
#if defined(__msp430_have_timerb7)
    case TBCCTL3   : ret = MCU.timerB.tbcctl[3].s; break;
    case TBCCTL4   : ret = MCU.timerB.tbcctl[4].s; break;
    case TBCCTL5   : ret = MCU.timerB.tbcctl[5].s; break;
    case TBCCTL6   : ret = MCU.timerB.tbcctl[6].s; break;
#endif
    case TBR       : ret = MCU.timerB.tbr;       break;
    case TBCCR0    : ret = MCU.timerB.tbccr[0];    break;
    case TBCCR1    : ret = MCU.timerB.tbccr[1];    break;
    case TBCCR2    : ret = MCU.timerB.tbccr[2];    break;
#if defined(__msp430_have_timerb7)
    case TBCCR3    : ret = MCU.timerB.tbccr[3];    break;
    case TBCCR4    : ret = MCU.timerB.tbccr[4];    break;
    case TBCCR5    : ret = MCU.timerB.tbccr[5];    break;
    case TBCCR6    : ret = MCU.timerB.tbccr[6];    break;
#endif
    case TBIV      : 
      ret = MCU.timerB.tiv.s;
      HW_DMSG_TIMER("msp430:" TIMERBNAME ": read TBIV [0x%04x] = 0x%04x\n",addr,ret);
      msp430_timerB_reset_highest_intr();
      break;
    default        :
      ERROR("msp430:" TIMERBNAME ": bad read address 0x%04x\n",addr);
      ret = 0;
      break;
    }
  /*  HW_DMSG_TIMER("msp430:" TIMERBNAME ": read [0x%04x] = 0x%04x\n",addr,ret); */
  return ret;
}

/* ************************************************** */

int msp430_timerB_chkifg(void)
{
  int ret = 0;

  TCHKIFG(timerB,TIMERBNAME,tbcctl[0],"tbcctl0",INTR_TIMERB_0)

  if (MCU.timerB.tiv.s)
    {    
	if ((MCU.timerB.tbctl.b.tbie == 1) && (MCU.timerB.tbctl.b.tbifg == 1))
        {
          HW_DMSG_TIMER("msp430:timerB: checkifg tbctl.tbifg == 1, interrupt set [%"PRId64"]\n", 
			MACHINE_TIME_GET_NANO());
          msp430_interrupt_set(INTR_TIMERB_1);
          return 1;
        }

      TCHKIFG(timerB,TIMERBNAME,tbcctl[1],"tbcctl1",INTR_TIMERB_1)
      TCHKIFG(timerB,TIMERBNAME,tbcctl[2],"tbcctl2",INTR_TIMERB_1)
#if defined(__msp430_have_timerb7)
      TCHKIFG(timerB,TIMERBNAME,tbcctl[3],"tbcctl3",INTR_TIMERB_1)
      TCHKIFG(timerB,TIMERBNAME,tbcctl[4],"tbcctl4",INTR_TIMERB_1)
      TCHKIFG(timerB,TIMERBNAME,tbcctl[5],"tbcctl5",INTR_TIMERB_1)
      TCHKIFG(timerB,TIMERBNAME,tbcctl[6],"tbcctl6",INTR_TIMERB_1)
#endif
    }
  return ret;
}

#endif
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
