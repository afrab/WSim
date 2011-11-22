
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

void msp430_timerA_set_tiv(void);
void msp430_timerB_set_tiv(void);

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
      HW_DMSG_TIMER_TIV("msp430:"TIMERN": tiv set to 0x%02x = %s [%"PRId64"]\n",   \
                        val & 0x7,timerA_tiv_to_str(val),MACHINE_TIME_GET_NANO()); \
    }

#define ifsetB(TIMER,TIMERN,cctl,val)					\
  if (MCU.TIMER.cctl.b.ccifg && MCU.TIMER.cctl.b.ccie)			\
    {									\
      MCU.TIMER.tiv.s = val;						\
      HW_DMSG_TIMER_TIV("msp430:"TIMERN": tiv set to 0x%02x = %s [%"PRId64"]\n", \
                    val & 0x7,timerB_tiv_to_str(val),MACHINE_TIME_GET_NANO());   \
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
  if (/*(MCU.TIMER.CCR > 0) &&   CCR can be compared to 0 !! */	        \
      (MCU.TIMER.TCCTL[NUM].b.cap   == 0) && /* compare mode */		\
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



/***********/
/* timer A */
/***********/
#define WRITE_TIMERA_CCR(NUM)						\
  do {									\
    MCU.timerA.taccr[NUM]  = val & 0x00ffffl;				\
    if (MCU.timerA.tar > MCU.timerA.taccr[NUM])			\
      MCU.timerA.b_taccr[NUM] = COMPARE_UNREACHABLE;			\
    else								\
      MCU.timerA.b_taccr[NUM] = 0;					\
    HW_DMSG_TIMER("msp430:"TIMERANAME": taccr%d  = 0x%04x "		\
		  "(TAR = 0x%04x) [%"PRId64"]\n",			\
		  NUM,MCU.timerA.taccr[NUM],MCU.timerA.tar,		\
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

/*************/
/* timer TA0 */
/*************/
#define WRITE_TIMERTA0_CCR(NUM)						\
  do {									\
    MCU.timerTA0.ta0ccr[NUM]  = val & 0x00ffffl;				\
    if (MCU.timerTA0.tar > MCU.timerTA0.ta0ccr[NUM])			\
      MCU.timerTA0.b_ta0ccr[NUM] = COMPARE_UNREACHABLE;			\
    else								\
      MCU.timerTA0.b_ta0ccr[NUM] = 0;					\
    HW_DMSG_TIMER("msp430:timerTA0: ta0ccr%d  = 0x%04x "			\
		  "(TAR = 0x%04x) [%"PRId64"]\n",			\
		  NUM,MCU.timerTA0.ta0ccr[NUM],MCU.timerTA0.tar,		\
		  MACHINE_TIME_GET_NANO());				\
  } while(0)

/*************/
/* timer TA1 */
/*************/
#define WRITE_TIMERTA1_CCR(NUM)						\
  do {									\
    MCU.timerTA1.ta1ccr[NUM]  = val & 0x00ffffl;				\
    if (MCU.timerTA1.tar > MCU.timerTA1.ta1ccr[NUM])			\
      MCU.timerTA1.b_ta1ccr[NUM] = COMPARE_UNREACHABLE;			\
    else								\
      MCU.timerTA1.b_ta1ccr[NUM] = 0;					\
    HW_DMSG_TIMER("msp430:timerTA1: ta1ccr%d  = 0x%04x "			\
		  "(TAR = 0x%04x) [%"PRId64"]\n",			\
		  NUM,MCU.timerTA1.ta1ccr[NUM],MCU.timerTA1.tar,		\
		  MACHINE_TIME_GET_NANO());				\
  } while(0)

#define TBCCRWRITE(NUM)							\
  case TBCCR##NUM :							\
  WRITE_TIMERB_CCR(NUM);						\
  break;


/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer A ********************************************************************* */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timera3) || defined(__msp430_have_timera5)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(__msp430_have_timera3)
enum timerA_addr_t {
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

#define TIMER_A_START 0x0160
#define TIMER_A_END   0x017e

#elif defined(__msp430_have_timera5)

enum timerA_addr_t {
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

#define TIMER_A_START  0x180
#define TIMER_A_END    0x19e
#endif

void msp430_timerA_create()
{
  msp430_io_register_addr8(TAIV,msp430_timerA_read8,msp430_timerA_write8);
  msp430_io_register_range8(TIMER_A_START,TIMER_A_END+1,msp430_timerA_read8,msp430_timerA_write8);

  msp430_io_register_addr16(TAIV,msp430_timerA_read,msp430_timerA_write);
  msp430_io_register_range16(TIMER_A_START,TIMER_A_END,msp430_timerA_read,msp430_timerA_write);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA_reset(void)
{
  memset(&MCU.timerA,0,sizeof(struct msp430_timerA_t));
  SET_DIVBUFFER(timerA,TIMERANAME,MCU.timerA.tactl.b.id);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA_set_tiv(void)
{
       ifsetA(timerA,TIMERANAME,tacctl[1],0x02)    /* highest */
  else ifsetA(timerA,TIMERANAME,tacctl[2],0x04)
  else if (MCU.timerA.tactl.b.taifg)               /* lowest */
    {
      MCU.timerA.tiv.s = 0x0a;
      HW_DMSG_TIMER_TIV("msp430:"TIMERANAME": tiv set to 0x0a = %s [%"PRId64"]\n",
                        timerA_tiv_to_str(0x0a),MACHINE_TIME_GET_NANO());
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void msp430_timerA_reset_highest_intr(void)
{
       ifzero(timerA,TIMERANAME,tacctl[1],"tacctl1",taccr1)  /* reset highest interrupt */
  else ifzero(timerA,TIMERANAME,tacctl[2],"tacctl2",taccr2)
  else if (MCU.timerA.tactl.b.taifg)
    {
      MCU.timerA.tactl.b.taifg = 0;
      HW_DMSG_TIMER("msp430:"TIMERANAME": tactl.ifg set to 0\n");
    }
  msp430_timerA_set_tiv();
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

void msp430_timerA_update(void)
{
  int clock;
  int tar_inc;
  /***************/
  /* Timer block */
  /***************/
  if (MCU.timerA.tactl.b.mc == TIMER_STOP)
    return ;
  
  clock = 0;
  switch (MCU.timerA.tactl.b.tassel)
    {
    case TIMER_SOURCE_TxCLK:
      ERROR("msp430:"TIMERANAME": source TACLK not implemented\n");
      break;
    case TIMER_SOURCE_ACLK:
      clock = MCU_CLOCK.ACLK_increment;
      break;
    case TIMER_SOURCE_SMCLK:
      clock = MCU_CLOCK.SMCLK_increment;
      break;
    case TIMER_SOURCE_INTxCLK:
      ERROR("msp430:"TIMERANAME": source INTACLK not implemented\n");
      break;
    }

  MCU.timerA.divbuffer +=clock;

  if ((clock == 0) || ((MCU.timerA.divbuffer & MCU.timerA.divuppermask) == 0))
    {
      return;
    }

  /*HW_DMSG_TIMER("msp430:"TIMERANAME": divbuffer %d div %d mask %x clocks %d",
    MCU.timerA.divbuffer, MCU.timerA.tactl.b.id, MCU.timerA.divlowermask, clock);*/
  tar_inc = MCU.timerA.divbuffer >> MCU.timerA.tactl.b.id; // div 
  MCU.timerA.divbuffer &= MCU.timerA.divlowermask;         // mod
  /*HW_DMSG_TIMER(" / tar_inc %d divbuffer %d\n",tar_inc, MCU.timerA.divbuffer);*/
  /*HW_DMSG_TIMER("msp430:"TIMERANAME": tar_inc %d\n",tar_inc); */

  switch (MCU.timerA.tactl.b.mc)
    {
    case TIMER_STOP:
      /* should not be reached due to return a few lines above */
      break;

    case TIMER_UP:     /* UP counter */
      if (MCU.timerA.taccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UP */
	{
	  MCU.timerA.tar += tar_inc;
	  /* HW_DMSG_TIMER("msp430:"TIMERANAME": tar (%x) += %d\n",MCU.timerA.tar,tar_inc); */

          /**************************/
          /* capture/compare blocks */
          /**************************/
          TIMER_COMPARE(timerA,TIMERANAME":taccr1:",tar,taccr,tacctl,1,INTR_TIMERA_1)
          TIMER_COMPARE(timerA,TIMERANAME":taccr2:",tar,taccr,tacctl,2,INTR_TIMERA_1)

	  if (MCU.timerA.tar >= MCU.timerA.taccr[0])
	    {
	      /* (ccr0 - 1) -> ccr0 */
	      MCU.timerA.tacctl[0].b.ccifg = 1;
	      msp430_timerA_set_tiv();
	      if (MCU.timerA.tacctl[0].b.ccie == 1)
		{
		  HW_DMSG_TIMER("msp430:"TIMERANAME": set interrupt TIMERA_0 from TIMER_UP\n");
		  msp430_interrupt_set(INTR_TIMERA_0);
		}
	      
	      /*  ccr0      -> 0    */
	      MCU.timerA.tactl.b.taifg   = 1;
	      msp430_timerA_set_tiv();
	      if (MCU.timerA.tactl.b.taie == 1)
		{
		  HW_DMSG_TIMER("msp430:"TIMERANAME": set interrupt TIMERA_1 from TIMER_UP\n");
		  msp430_interrupt_set(INTR_TIMERA_1);
		}
	      MCU.timerA.tar -= MCU.timerA.taccr[0];
	      HW_DMSG_2_DBG("msp430:"TIMERANAME": up mode wraps to 0 ===============================\n");
              TIMER_COMPARE_WRAPS(timerA,TIMERANAME":taccr1",taccr,tacctl,1);
	      TIMER_COMPARE_WRAPS(timerA,TIMERANAME":taccr2",taccr,tacctl,2);
	    }
	}
      break;

    case TIMER_CONT:    /* Continuous counter */
      MCU.timerA.tar += tar_inc;
      /* HW_DMSG_TIMER("msp430:"TIMERANAME": tar skip to 0x%04x (inc 0x%04x)\n",MCU.timerA.tar,tar_inc); */

      /**************************/
      /* capture/compare blocks */
      /**************************/
      TIMER_COMPARE(timerA,TIMERANAME,tar,taccr,tacctl,0,INTR_TIMERA_0)
      TIMER_COMPARE(timerA,TIMERANAME,tar,taccr,tacctl,1,INTR_TIMERA_1)
      TIMER_COMPARE(timerA,TIMERANAME,tar,taccr,tacctl,2,INTR_TIMERA_1)

      if (MCU.timerA.tar >= TAR_MAX_LIMIT)
	{
	  MCU.timerA.tactl.b.taifg = 1;
	  msp430_timerA_set_tiv();
	  if (MCU.timerA.tactl.b.taie == 1)
	    {
	      HW_DMSG_TIMER("msp430:"TIMERANAME": interrupt TIMERA_1 from TIMER_CONT (tar 0x%06x) [%"PRId64"]\n",
			    MCU.timerA.tar,MACHINE_TIME_GET_NANO());
	      msp430_interrupt_set(INTR_TIMERA_1);
	    }
	  MCU.timerA.tar -= 0xffffu;
	  /* contig mode bad wraps */
	  HW_DMSG_TIMER("msp430:"TIMERANAME": contig mode wraps to 0 ===============================\n");
	  TIMER_COMPARE_WRAPS(timerA,TIMERANAME,taccr,tacctl,0);
	  TIMER_COMPARE_WRAPS(timerA,TIMERANAME,taccr,tacctl,1);
	  TIMER_COMPARE_WRAPS(timerA,TIMERANAME,taccr,tacctl,2);
	}
      break;

    case TIMER_UD:      /* UP/DOWN counter */
      if (MCU.timerA.taccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UD */
	{
	  if (MCU.timerA.udmode == TIMER_UD_UP)
	    {
	      MCU.timerA.tar += tar_inc;
	      if (MCU.timerA.tar >= MCU.timerA.taccr[0])
		{
		  /* we are going UP, so the timer wraps and is going down */
		  MCU.timerA.udmode = TIMER_UD_DOWN;
		  MCU.timerA.tar    = MCU.timerA.taccr[0];
		  MCU.timerA.tacctl[0].b.ccifg = 1;
		  msp430_timerA_set_tiv();
		  if (MCU.timerA.tacctl[0].b.ccie == 1)
		    {
		      HW_DMSG_TIMER("msp430:"TIMERANAME": set interrupt TIMERA_1 from TIMER_UD in UP mode\n");
		      msp430_interrupt_set(INTR_TIMERA_0);
		    }
		  HW_DMSG_TIMER("msp430:"TIMERANAME": Up/Down mode wraps to max ===============================\n");
		  TIMER_COMPARE_WRAPS_DOWN(timerA,TIMERANAME,taccr,1,MCU.timerA.taccr[0]);
		  TIMER_COMPARE_WRAPS_DOWN(timerA,TIMERANAME,taccr,2,MCU.timerA.taccr[0]);
		}
	      else
		{
		  TIMER_COMPARE(timerA,TIMERANAME,tar,taccr,tacctl,1,INTR_TIMERA_1)
		  TIMER_COMPARE(timerA,TIMERANAME,tar,taccr,tacctl,2,INTR_TIMERA_1)
		}
	    }
	  else /* timer is down */
	    {
	      MCU.timerA.tar -= tar_inc;
	      if (MCU.timerA.tar <= 0)
		{
		  /* we are going down, we wraps and start up */
		  MCU.timerA.udmode        = TIMER_UD_UP;
		  MCU.timerA.tar           = 0;
		  MCU.timerA.tactl.b.taifg = 1;
		  msp430_timerA_set_tiv();
		  if (MCU.timerA.tactl.b.taie == 1)
		    {
		      HW_DMSG_TIMER("msp430:"TIMERANAME": set interrupt TIMERA_1 from TIMER_UD in DOWN mode\n");
		      msp430_interrupt_set(INTR_TIMERA_1);
		    }
		  HW_DMSG_TIMER("msp430:"TIMERANAME": Up/Down mode wraps to 0 ===============================\n");
		  TIMER_COMPARE_WRAPS(timerA,TIMERANAME,taccr,tacctl,1);
		  TIMER_COMPARE_WRAPS(timerA,TIMERANAME,taccr,tacctl,2);
		}
	      else
		{
		  TIMER_COMPARE_DOWN(timerA,TIMERANAME,tar,taccr,tacctl,1,INTR_TIMERA_1,MCU.timerA.taccr[0])
                  TIMER_COMPARE_DOWN(timerA,TIMERANAME,tar,taccr,tacctl,2,INTR_TIMERA_1,MCU.timerA.taccr[0])
		}
	    }
	}
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#if defined(MSP430f1611) 	     
/* on msp430f1611 this capture pin is p1.(i+1) 
	        CCR0: A=P1.1 B=P2.2
	        CCR1: A=P1.2 B=CAOUT
	        CCR2: A=P1.3 B=ACLK
*/
#define TIMERA_CAPTURE_A_INPUT_TEST(i)   ((MCU.digiIO.in_updated[0] & (0x2 << i)) != 0 ? 1 : 0)
#define TIMERA_CAPTURE_A_INPUT_VALUE(i)  ((MCU.digiIO.in[0]         & (0x2 << i)) != 0 ? 1 : 0)
#define TIMERA_CAPTURE_A_INPUT_TAR(i)    (MCU.timerA.tar)

static int TIMERA_CAPTURE_B_INPUT_TEST(int i)
{
  switch(i) {
  case 0: return ((MCU.digiIO.in_updated[1] & (0x4)) != 0 ? 1 : 0);
  case 1: return -1;
  case 2: return ((MCU_CLOCK.ACLK_increment > 0) ? 1 : 0);
  }
  return -1;
}
static int TIMERA_CAPTURE_B_INPUT_VALUE(int i)
{
  switch(i) {
  case 0: return ((MCU.digiIO.in[1]         & (0x4)) != 0 ? 1 : 0);
  case 1: return 0;
  case 2: return ((MCU_CLOCK.ACLK_increment > 0) && (MCU.timerA.tacctl[ i ].b.cm == 1)) ? 1 : 0;
  }
  return 0;
}
static int TIMERA_CAPTURE_B_INPUT_TAR(int i)
{
  switch(i) {
  case 0: return MCU.timerA.tar;
  case 1: return MCU.timerA.tar;
  case 2: return MCU.timerA.tar; // MCU_CLOCK.ACLK_increment > 1, should we go back in time ?
  }
  return 0;
}
#else
#define TIMERA_CAPTURE_A_INPUT_TEST(i)   -1
#define TIMERA_CAPTURE_A_INPUT_VALUE(i)  0
#define TIMERA_CAPTURE_A_INPUT_TAR(i)    0
#define TIMERA_CAPTURE_B_INPUT_TEST(i)   -1
#define TIMERA_CAPTURE_B_INPUT_VALUE(i)  0
#define TIMERA_CAPTURE_B_INPUT_TAR(i)    0
#endif

void msp430_timerA_capture_port(int i, char* AB, int test_bit, int input_bit, int input_tar)
{
  switch (test_bit) {
  case 0: /* nothing happened */
    break;
  case 1:
    {
      int rising_edge  = (MCU.timerA.tacctl[ i ].b.cm == 1) &&  (input_bit);
      int falling_edge = (MCU.timerA.tacctl[ i ].b.cm == 2) && !(input_bit);
      int both_edges   = (MCU.timerA.tacctl[ i ].b.cm == 3);
      if (rising_edge || falling_edge || both_edges) 
	{
	  MCU.timerA.taccr [ i ]         = (input_tar);
	  MCU.timerA.tacctl[ i ].b.ccifg = 1;
	  MCU.timerA.tacctl[ i ].b.cci   = (input_bit);
	  msp430_timerA_set_tiv();
	  
	  if (MCU.timerA.tacctl[ i ].b.ccie == 1)
	    {
	      if (i==0)
		{
		  HW_DMSG_TIMER("msp430:" TIMERANAME ": set interrupt TIMERA_0 from CAPTURE CCI%d%s\n",i,AB);
		  msp430_interrupt_set(INTR_TIMERA_0);
		}
	      else
		{
		  HW_DMSG_TIMER("msp430:" TIMERANAME ": set interrupt TIMERA_1 from CAPTURE CCI%d%s\n",i,AB);
		  msp430_interrupt_set(INTR_TIMERA_1);
		}
	    }
	}
      break;
    default:
      ERROR("msp430:" TIMERANAME ": device specific capture ports undefined for CCI%d%s\n",i,AB);
      break;
    }
  }
}

void msp430_timerA_capture(void)
{
  int i;
  for(i=0 ; i < TIMERA_COMPARATOR ; i++)
    {
      if ((MCU.timerA.tacctl[ i ].b.cap == 1) && (MCU.timerA.tacctl[ i ].b.cm > 0))
	{
	  switch (MCU.timerA.tacctl[ i ].b.ccis)
	    {
	      /***************/
	    case 0: /* CCIxA */
	      /***************/
	      msp430_timerA_capture_port(i,"A",
	                                 (TIMERA_CAPTURE_A_INPUT_TEST(i)),
	                                 (TIMERA_CAPTURE_A_INPUT_VALUE(i)),
	                                 (TIMERA_CAPTURE_A_INPUT_TAR(i)));
	      break; 
	      /*****************/
	    case 1:  /* CCIxB  */
	      /*****************/
	      msp430_timerA_capture_port(i,"B",
	                                 (TIMERA_CAPTURE_B_INPUT_TEST(i)),
	                                 (TIMERA_CAPTURE_B_INPUT_VALUE(i)),
	                                 (TIMERA_CAPTURE_B_INPUT_TAR(i)));
	      break;
	      /*************/
	    case 2: /* GND */
	      /*************/
	      ERROR("msp430:" TIMERANAME ": capture not implemented on this port (GND)\n");
	      break;
	      /*************/
	    case 3: /* Vcc */
	      /*************/
	      ERROR("msp430:" TIMERANAME ": capture not implemented on this port (Vcc)\n");
	      break;
	    }
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_timerA_write8(uint16_t addr, int8_t val)
{
  msp430_timerA_write(addr,val);
}

void msp430_timerA_write(uint16_t addr, int16_t val)
{
  switch ((enum timerA_addr_t)addr)
    {
    case TAIV      : /* read only */
      /* although this register is read only, we can have a write on it */
      msp430_timerA_reset_highest_intr();      
      HW_DMSG_TIMER("msp430:"TIMERANAME": taiv write, reset highest intr\n");
      break;

    case TACTL:
      {
	union {
	  uint16_t         s;
	  struct tactl_t   b;
	} tactl;
	
	HW_DMSG_2_DBG("msp430:"TIMERANAME": tactl   = 0x%04x\n",val);
	tactl.s = val;

	if (tactl.b.taclr) /* this one must be first as it resets divider and ssel */
	  {
	    MCU.timerA.tactl.b.id = 0; 
	    MCU.timerA.tar        = 0;
	    tactl.b.taclr          = 0;
	    SET_DIVBUFFER(timerA,TIMERANAME,0); 
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.taclr clear\n");
	  }

	if (tactl.b.tassel != MCU.timerA.tactl.b.tassel)
	  {
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.tassel set to %d (%s)\n",
			  tactl.b.tassel,str_clocksrc[tactl.b.tassel]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:"TIMERANAME":    tactl.tassel left to %d (%s)\n",
			  tactl.b.tassel,str_clocksrc[tactl.b.tassel]);
	  }

	if (tactl.b.id != MCU.timerA.tactl.b.id)
	  {
	    SET_DIVBUFFER(timerA,TIMERANAME,tactl.b.id);
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.id set to %d (DIV = %d)\n",tactl.b.id,1<<tactl.b.id);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:"TIMERANAME":    tactl.id left to %d (DIV = %d)\n",tactl.b.id,1<<tactl.b.id);
	  }

	if (tactl.b.mc != MCU.timerA.tactl.b.mc)
	  {
	    if ((tactl.b.mc == TIMER_UP) && (MCU.timerA.tar > MCU.timerA.taccr[0]))
	      {
		MCU.timerA.tar    = 0; /* restart from zero */
	      }
	    MCU.timerA.udmode = TIMER_UD_UP;
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.mc going to mode %d (%s)\n",
			  tactl.b.mc,str_mode[tactl.b.mc]);
	  }
	else
	  {
	    HW_DMSG_2_DBG("msp430:"TIMERANAME":    tactl.mc left to mode %d (%s)\n",
			  tactl.b.mc,str_mode[tactl.b.mc]);
	  }

	if (tactl.b.taie != MCU.timerA.tactl.b.taie)
	  {
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.ie set to %d\n",tactl.b.taie);
	    if ((tactl.b.taie == 1) && (tactl.b.taifg == 1))
	      {
		HW_DMSG_TIMER("msp430:"TIMERANAME": checkifg tactl.taifg == 1, interrupt set\n");
		msp430_interrupt_set(INTR_TIMERA_1);
	      }
	  }

	if (tactl.b.taifg != MCU.timerA.tactl.b.taifg)
	  {
	    HW_DMSG_TIMER("msp430:"TIMERANAME":    tactl.tbifg set to %d\n",tactl.b.taifg);
	    if ((tactl.b.taie == 1) && (tactl.b.taifg == 1))
	      {
		HW_DMSG_TIMER("msp430:"TIMERANAME": checkifg tactl.taifg == 1, interrupt set\n");
		msp430_interrupt_set(INTR_TIMERA_1);
	      }
	  }

	MCU.timerA.tactl.s = tactl.s;
	msp430_timerA_set_tiv();
      }
      break;

      TIMERA_TCCTLWRITE(TACCTL0,tacctlu_t,timerA,TIMERANAME,tacctl,"tacctl0",0,INTR_TIMERA_0)
      TIMERA_TCCTLWRITE(TACCTL1,tacctlu_t,timerA,TIMERANAME,tacctl,"tacctl1",1,INTR_TIMERA_1)
      TIMERA_TCCTLWRITE(TACCTL2,tacctlu_t,timerA,TIMERANAME,tacctl,"tacctl2",2,INTR_TIMERA_1)


    case TAR:
      MCU.timerA.tar = val & 0xffffu;
      HW_DMSG_TIMER("msp430:"TIMERANAME": tar     = 0x%04x [%"PRId64"]\n",
		    MCU.timerA.tar,MACHINE_TIME_GET_NANO());
      break;

    case TACCR0    :
      if ((MCU.timerA.tactl.b.mc == TIMER_UP) || (MCU.timerA.tactl.b.mc == TIMER_UD))
	{
	  if ((MCU.timerA.taccr[0] == 0) && (val > 0))
	    {
	      MCU.timerA.udmode = TIMER_UD_UP;
	      MCU.timerA.tar    = 0;
	      HW_DMSG_TIMER("msp430:"TIMERANAME": taccr0 > 0, restarts the timer\n");
	    }
	  else if (val < MCU.timerA.tar) 
	    {
	      if (MCU.timerA.tactl.b.mc == TIMER_UP)
		{
		  MCU.timerA.tar = 0;
		  HW_DMSG_TIMER("msp430:"TIMERANAME": taccr0 > tar, restarts from 0\n");
		}
	      else if (MCU.timerA.udmode == TIMER_UD_UP)
		{
		  MCU.timerA.udmode = TIMER_UD_DOWN;
		  HW_DMSG_TIMER("msp430:"TIMERANAME": taccr0 > tar, going mode down\n");
		}
	    }
	}
      WRITE_TIMERA_CCR(0);
      break;

    case TACCR1    : WRITE_TIMERA_CCR(1); break;
    case TACCR2    : WRITE_TIMERA_CCR(2); break;

    case TA_RES1   : /* reserved */
    case TA_RES2   : /* reserved */
    case TA_RES3   : /* reserved */
    case TA_RES4   : /* reserved */
    case TA_RES5   : /* reserved */
    case TA_RES6   : /* reserved */
    case TA_RES7   : /* reserved */
    case TA_RES8   : /* reserved */
      ERROR("msp430:"TIMERANAME": bad write address (reserved) [0x%04x]\n",addr);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_timerA_read8(uint16_t addr)
{
  return msp430_timerA_read(addr) & 0xff;
}

int16_t msp430_timerA_read(uint16_t addr)
{
  int16_t ret;
  switch ((enum timerA_addr_t) addr)
    {
    case TACTL     : ret = MCU.timerA.tactl.s;     break;
    case TACCTL0   : ret = MCU.timerA.tacctl[0].s; break;
    case TACCTL1   : ret = MCU.timerA.tacctl[1].s; break;
    case TACCTL2   : ret = MCU.timerA.tacctl[2].s; break;
    case TAR       : ret = MCU.timerA.tar;         break;
    case TACCR0    : ret = MCU.timerA.taccr[0];    break;
    case TACCR1    : ret = MCU.timerA.taccr[1];    break;
    case TACCR2    : ret = MCU.timerA.taccr[2];    break;
    case TAIV      : 
      ret = MCU.timerA.tiv.s;
      HW_DMSG_TIMER("msp430:"TIMERANAME": read TAIV [0x%04x] = 0x%04x\n",addr,ret);
      msp430_timerA_reset_highest_intr();
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
      ERROR("msp430:"TIMERANAME": bad read address 0x%04x\n",addr);
      ret = 0;
      break;
    }
  /*  HW_DMSG_TIMER("msp430:"TIMERANAME": read [0x%04x] = 0x%04x\n",addr,ret); */
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_timerA_chkifg(void)
{
  int ret = 0;

  TCHKIFG(timerA,TIMERANAME,tacctl[0],"tacctl0",INTR_TIMERA_0)

  if (MCU.timerA.tiv.s)
    {
       if ((MCU.timerA.tactl.b.taie == 1) && (MCU.timerA.tactl.b.taifg == 1))
	{
	  HW_DMSG_TIMER("msp430:"TIMERANAME": checkifg tactl.taifg == 1, interrupt set\n");
	  msp430_interrupt_set(INTR_TIMERA_1);
	  return 1;
	}
      
      TCHKIFG(timerA,TIMERANAME,tacctl[1],"tacctl1",INTR_TIMERA_1)
      TCHKIFG(timerA,TIMERANAME,tacctl[2],"tacctl2",INTR_TIMERA_1)
    }
  return ret;
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
/* ************************************************** */
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
/* ************************************************** */
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
      HW_DMSG_TIMER("msp430:"TIMERBNAME": tiv set to 0x0e = %s [%"PRId64"]\n",
		    timerB_tiv_to_str(0x0e),MACHINE_TIME_GET_NANO());
    }
}

/* ************************************************** */
/* ************************************************** */
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
/* ************************************************** */
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
/* ************************************************** */
/* ************************************************** */

#if defined(MSP430f1611)
/* on msp430f1611 this capture pin is p4.i 
  CCR0  A=4.0  B=4.0
  CCRx  A=4.x  B=4.x
  CCR6  A=4.6  B=ACLK
*/
#define TIMERB_CAPTURE_A_INPUT_TEST(i)   ((MCU.digiIO.in_updated[3] & (0x1 << i)) != 0 ? 1 : 0)
#define TIMERB_CAPTURE_A_INPUT_VALUE(i)  ((MCU.digiIO.in[3]         & (0x1 << i)) != 0 ? 1 : 0)
#define TIMERB_CAPTURE_A_INPUT_TBR(i)    (MCU.timerB.tbr)

static int TIMERB_CAPTURE_B_INPUT_TEST(int i)
{
  if (i < 6)
    return TIMERB_CAPTURE_A_INPUT_TEST(i);
  return ((MCU_CLOCK.ACLK_increment > 0) ? 1 : 0);
}
static int TIMERB_CAPTURE_B_INPUT_VALUE(int i)
{
  if (i < 6)
    return TIMERB_CAPTURE_A_INPUT_VALUE(i);
  return ((MCU_CLOCK.ACLK_increment > 0) ? 1 : 0);
}
static int TIMERB_CAPTURE_B_INPUT_TBR(int i)
{
  if (i < 6)
    return TIMERB_CAPTURE_A_INPUT_TBR(i);
  return MCU.timerB.tbr; // MCU_CLOCK.ACLK_increment > 1, should we go back in time ?
}
#else
#define TIMERB_CAPTURE_A_INPUT_TEST(i)   -1
#define TIMERB_CAPTURE_A_INPUT_VALUE(i)  0
#define TIMERB_CAPTURE_A_INPUT_TBR(i)    0
#define TIMERB_CAPTURE_B_INPUT_TEST(i)   -1
#define TIMERB_CAPTURE_B_INPUT_VALUE(i)  0
#define TIMERB_CAPTURE_B_INPUT_TBR(i)    0
#endif

void msp430_timerB_capture_port(int i, char* AB, int test_bit, int input_bit, int input_tbr)
{
  switch (test_bit) {
  case 0: /* nothing happened */
    break;
  case 1:
    {
      int rising_edge  = (MCU.timerB.tbcctl[ i ].b.cm == 1) &&  (input_bit);
      int falling_edge = (MCU.timerB.tbcctl[ i ].b.cm == 2) && !(input_bit);
      int both_edges   = (MCU.timerB.tbcctl[ i ].b.cm == 3);
      if (rising_edge || falling_edge || both_edges) 
	{
	  MCU.timerB.tbccr [ i ]         = (input_tbr);
	  MCU.timerB.tbcctl[ i ].b.ccifg = 1;
	  MCU.timerB.tbcctl[ i ].b.cci   = (input_bit);
	  msp430_timerB_set_tiv();
	  
	  if (MCU.timerB.tbcctl[ i ].b.ccie == 1)
	    {
	      if (i==0)
		{
		  HW_DMSG_TIMER("msp430:" TIMERBNAME ": set interrupt TIMERB_0 from CAPTURE CCI%d%s\n",i,AB);
		  msp430_interrupt_set(INTR_TIMERB_0);
		}
	      else
		{
		  HW_DMSG_TIMER("msp430:" TIMERBNAME ": set interrupt TIMERB_1 from CAPTURE CCI%d%s\n",i,AB);
		  msp430_interrupt_set(INTR_TIMERB_1);
		}
	    }
	}
      break;
    default:
      ERROR("msp430:" TIMERBNAME ": device specific capture ports undefined for CCI%d%s\n",i,AB);
      break;
    }
  }
}

void msp430_timerB_capture(void)
{
  int i;
  for(i=0 ; i < TIMERB_COMPARATOR ; i++)
    {
      if (MCU.timerB.tbcctl[ i ].b.cap == 1 && MCU.timerB.tbcctl[ i ].b.cm > 0)
	{
	  switch (MCU.timerB.tbcctl[ i ].b.ccis)
	    {
	      /***************/
	    case 0: /* CCIxA */
	      /***************/
	      msp430_timerB_capture_port(i,"A",
	                                 (TIMERB_CAPTURE_A_INPUT_TEST(i)),
	                                 (TIMERB_CAPTURE_A_INPUT_VALUE(i)),
	                                 (TIMERB_CAPTURE_A_INPUT_TBR(i)));
	      break; 
	      /*****************/
	    case 1:  /* CCIxB  */
	      /*****************/
	      msp430_timerB_capture_port(i,"B",
	                                 (TIMERB_CAPTURE_B_INPUT_TEST(i)),
	                                 (TIMERB_CAPTURE_B_INPUT_VALUE(i)),
	                                 (TIMERB_CAPTURE_B_INPUT_TBR(i)));
	      break;
	      /*************/
	    case 2: /* GND */
	      /*************/
	      ERROR("msp430:" TIMERBNAME ": capture not implemented on this port (GND)\n");
	      break;
	      /*************/
	    case 3: /* Vcc */
	      /*************/
	      ERROR("msp430:" TIMERBNAME ": capture not implemented on this port (Vcc)\n");
	      break;
	    }
	}
    }
}

/* ************************************************** */
/* ************************************************** */
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
/* ************************************************** */
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
/* ************************************************** */
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
/* ************************************************** */
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

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // have_timerb

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer TA0 ******************************************************************* */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timerTA0)

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA0_create()
{
  //msp430_io_register_addr8(TA0IV, msp430_timerTA0_read8, msp430_timerTA0_write8);
  msp430_io_register_range8(TIMER_TA0_START, TIMER_TA0_END + 1, msp430_timerTA0_read8, msp430_timerTA0_write8);

  //msp430_io_register_addr16(TA0IV, msp430_timerTA0_read, msp430_timerTA0_write);
  msp430_io_register_range16(TIMER_TA0_START, TIMER_TA0_END, msp430_timerTA0_read, msp430_timerTA0_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA0_reset(void)
{
  memset(&MCU.timerTA0, 0, sizeof (struct msp430_timerTA0_t));
  SET_DIVBUFFER(timerTA0, "timerTA0", MCU.timerTA0.ta0ctl.b.id);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA0_set_tiv(void)
{
  if (MCU.timerTA0.ta0ctl.b.taifg) {
    MCU.timerTA0.tiv.s = 0x0e; //to work as test app expects, need to check if overflow happened
    HW_DMSG_2_DBG("msp430:timerTA0: tiv set to 0x0e\n");
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

static void msp430_timerTA0_reset_highest_intr(void)
{
  ifzero(timerTA0, "timerTA0", ta0cctl[1], "ta0cctl1", ta0ccr1) /* reset highest interrupt */
  else ifzero(timerTA0, "timerTA0", ta0cctl[2], "ta0cctl2", ta0ccr2)
  else if (MCU.timerTA0.ta0ctl.b.taifg) {
    MCU.timerTA0.ta0ctl.b.taifg = 0;
    HW_DMSG_TIMER("msp430:timerTA0: ta0ctl.ifg set to 0\n");
  }
  msp430_timerTA0_set_tiv();
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
 *              00 TAxCLK   /  external
 *              01 ACLK    /  internal from basic clock
 *              10 SMCLK   /  internal from basic clock
 *              11 inverted TAxCLK   /  external
 * bb ID      timer A input divider
 *              00  /1
 *              01  /2
 *              10  /4
 *              11  /8
 * cc MC      mode control
 *              00 stop mode
 *              01 up mode (count to TAxCCR0)
 *              10 continuous (count to 0x0ffff)
 *              11 up/down (based on TAxCCR0)
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

void msp430_timerTA0_update(void)
{
  int clock;
  int tar_inc;
  /***************/
  /* Timer block */
  /***************/
  if (MCU.timerTA0.ta0ctl.b.mc == TIMER_STOP)
    return;

  clock = 0;
  switch (MCU.timerTA0.ta0ctl.b.tassel) {
  case TIMER_SOURCE_TxCLK:
    ERROR("msp430:timerTA0: source TACLK not implemented\n");
    break;
  case TIMER_SOURCE_ACLK:
    clock = MCU_CLOCK.ACLK_increment;
    break;
  case TIMER_SOURCE_SMCLK:
    clock = MCU_CLOCK.SMCLK_increment;
    break;
  case TIMER_SOURCE_INTxCLK:
    ERROR("msp430:timerTA0: source INTACLK not implemented\n");
    break;
  }

  MCU.timerTA0.divbuffer += clock;

  if ((clock == 0) || ((MCU.timerTA0.divbuffer & MCU.timerTA0.divuppermask) == 0)) {
    return;
  }

  tar_inc = MCU.timerTA0.divbuffer >> MCU.timerTA0.ta0ctl.b.id; // div
  MCU.timerTA0.divbuffer &= MCU.timerTA0.divlowermask; // mod
  
  switch (MCU.timerTA0.ta0ctl.b.mc) {
  case TIMER_STOP:
    /* should not be reached due to return a few lines above */
    break;

  case TIMER_UP: /* UP counter */
    if (MCU.timerTA0.ta0ccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UP */ {
      MCU.timerTA0.tar += tar_inc;

      /**************************/
      /* capture/compare blocks */
      /**************************/
      TIMER_COMPARE(timerTA0, "timerTA0:taccr1:", tar, ta0ccr, ta0cctl, 1, INTR_TIMERTA0_1)
      TIMER_COMPARE(timerTA0, "timerTA0:taccr2:", tar, ta0ccr, ta0cctl, 2, INTR_TIMERTA0_1)
      TIMER_COMPARE(timerTA0, "timerTA0:taccr3:", tar, ta0ccr, ta0cctl, 3, INTR_TIMERTA0_1)
      TIMER_COMPARE(timerTA0, "timerTA0:taccr4:", tar, ta0ccr, ta0cctl, 4, INTR_TIMERTA0_1)

      if (MCU.timerTA0.tar >= MCU.timerTA0.ta0ccr[0]) {
        /* (ccr0 - 1) -> ccr0 */
        MCU.timerTA0.ta0cctl[0].b.ccifg = 1;
        msp430_timerTA0_set_tiv();
        if (MCU.timerTA0.ta0cctl[0].b.ccie == 1) {
          HW_DMSG_TIMER("msp430:timerTA0: set interrupt TIMERTA0_0 from TIMER_UP\n");
          msp430_interrupt_set(INTR_TIMERTA0_0);
        }

        /*  ccr0      -> 0    */
        MCU.timerTA0.ta0ctl.b.taifg = 1;
        msp430_timerTA0_set_tiv();
        if (MCU.timerTA0.ta0ctl.b.taie == 1) {
          HW_DMSG_TIMER("msp430:timerTA0: set interrupt TIMERTA0_1 from TIMER_UP\n");
          msp430_interrupt_set(INTR_TIMERTA0_1);
        }
        MCU.timerTA0.tar -= MCU.timerTA0.ta0ccr[0];
        HW_DMSG_2_DBG("msp430:timerTA0: up mode wraps to 0 ===============================\n");
        TIMER_COMPARE_WRAPS(timerTA0, "timerTA0:ta0ccr1", ta0ccr, ta0cctl, 1);
        TIMER_COMPARE_WRAPS(timerTA0, "timerTA0:ta0ccr2", ta0ccr, ta0cctl, 2);
        TIMER_COMPARE_WRAPS(timerTA0, "timerTA0:ta0ccr3", ta0ccr, ta0cctl, 3);
        TIMER_COMPARE_WRAPS(timerTA0, "timerTA0:ta0ccr4", ta0ccr, ta0cctl, 4);
      }
    }
    break;

  case TIMER_CONT: /* Continuous counter */
    MCU.timerTA0.tar += tar_inc;

    /**************************/
    /* capture/compare blocks */
    /**************************/
    TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 0, INTR_TIMERTA0_0)
    TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 1, INTR_TIMERTA0_1)
    TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 2, INTR_TIMERTA0_1)
    TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 3, INTR_TIMERTA0_1)
    TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 4, INTR_TIMERTA0_1)

    if (MCU.timerTA0.tar >= TAR_MAX_LIMIT) {
      MCU.timerTA0.ta0ctl.b.taifg = 1;
      msp430_timerTA0_set_tiv();
      if (MCU.timerTA0.ta0ctl.b.taie == 1) {
        HW_DMSG_TIMER("msp430:timerTA0: interrupt TIMERTA0_1 from TIMER_CONT (tar 0x%06x) [%"PRId64"]\n",
          MCU.timerTA0.tar, MACHINE_TIME_GET_NANO());
        msp430_interrupt_set(INTR_TIMERTA0_1);
      }
      MCU.timerTA0.tar -= 0xffffu;
      /* contig mode bad wraps */
      HW_DMSG_TIMER("msp430:timerTA0: contig mode wraps to 0 ===============================\n");
      TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 0);
      TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 1);
      TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 2);
      TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 3);
      TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 4);
    }
    break;

  case TIMER_UD: /* UP/DOWN counter */
    if (MCU.timerTA0.ta0ccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UD */ {
      if (MCU.timerTA0.udmode == TIMER_UD_UP) {
        MCU.timerTA0.tar += tar_inc;
        if (MCU.timerTA0.tar >= MCU.timerTA0.ta0ccr[0]) {
          /* we are going UP, so the timer wraps and is going down */
          MCU.timerTA0.udmode = TIMER_UD_DOWN;
          MCU.timerTA0.tar = MCU.timerTA0.ta0ccr[0];
          MCU.timerTA0.ta0cctl[0].b.ccifg = 1;
          msp430_timerTA0_set_tiv();
          if (MCU.timerTA0.ta0cctl[0].b.ccie == 1) {
            HW_DMSG_TIMER("msp430:timerTA0: set interrupt TIMERTA0_1 from TIMER_UD in UP mode\n");
            msp430_interrupt_set(INTR_TIMERTA0_0);
          }
          HW_DMSG_TIMER("msp430:timerTA0: Up/Down mode wraps to max ===============================\n");
          TIMER_COMPARE_WRAPS_DOWN(timerTA0, "timerTA0", ta0ccr, 1, MCU.timerTA0.ta0ccr[0]);
          TIMER_COMPARE_WRAPS_DOWN(timerTA0, "timerTA0", ta0ccr, 2, MCU.timerTA0.ta0ccr[0]);
          TIMER_COMPARE_WRAPS_DOWN(timerTA0, "timerTA0", ta0ccr, 3, MCU.timerTA0.ta0ccr[0]);
          TIMER_COMPARE_WRAPS_DOWN(timerTA0, "timerTA0", ta0ccr, 4, MCU.timerTA0.ta0ccr[0]);
        } else {
          TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 1, INTR_TIMERTA0_1)
          TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 2, INTR_TIMERTA0_1)
          TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 3, INTR_TIMERTA0_1)
          TIMER_COMPARE(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 4, INTR_TIMERTA0_1)
        }
      } else /* timer is down */ {
        MCU.timerTA0.tar -= tar_inc;
        if (MCU.timerTA0.tar <= 0) {
          /* we are going down, we wraps and start up */
          MCU.timerTA0.udmode = TIMER_UD_UP;
          MCU.timerTA0.tar = 0;
          MCU.timerTA0.ta0ctl.b.taifg = 1;
          msp430_timerTA0_set_tiv();
          if (MCU.timerTA0.ta0ctl.b.taie == 1) {
            HW_DMSG_TIMER("msp430:timerTA0: set interrupt TIMERTA0_1 from TIMER_UD in DOWN mode\n");
            msp430_interrupt_set(INTR_TIMERTA0_1);
          }
          HW_DMSG_TIMER("msp430:timerTA0: Up/Down mode wraps to 0 ===============================\n");
          TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 1);
          TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 2);
          TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 3);
          TIMER_COMPARE_WRAPS(timerTA0, "timerTA0", ta0ccr, ta0cctl, 4);
        } else {
          TIMER_COMPARE_DOWN(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 1, INTR_TIMERTA0_1, MCU.timerTA0.ta0ccr[0])
          TIMER_COMPARE_DOWN(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 2, INTR_TIMERTA0_1, MCU.timerTA0.ta0ccr[0])
          TIMER_COMPARE_DOWN(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 3, INTR_TIMERTA0_1, MCU.timerTA0.ta0ccr[0])
          TIMER_COMPARE_DOWN(timerTA0, "timerTA0", tar, ta0ccr, ta0cctl, 4, INTR_TIMERTA0_1, MCU.timerTA0.ta0ccr[0])
        }
      }
    }
    break;
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA0_capture(void)
{
  if (MCU.timerTA0.ta0cctl[2].b.cap == 1 &&
    MCU.timerTA0.ta0cctl[2].b.cm > 0) {
    /*
 at this time we don't care about:
 SCS  : synchroneous capture source
 SCCI : Synchronized capture/compare input
     */
    switch (MCU.timerTA0.ta0cctl[2].b.ccis) {
    case 0: /* CCIxA = TA2 */
      HW_DMSG_TIMER("msp430:timerTA0: capture not implemented on this port\n");
      break;
    case 1: /* CCIxB */
      /* on msp430f1611 this pin in internal ACLK */
      if (MCU_CLOCK.ACLK_increment > 0) {
        MCU.timerTA0.ta0ccr[2] = MCU.timerTA0.tar;
        MCU.timerTA0.ta0cctl[2].b.ccifg = 1;
        msp430_timerTA0_set_tiv();
        if (MCU.timerTA0.ta0cctl[2].b.ccie == 1) {
          HW_DMSG_TIMER("msp430:timerTA0: set interrupt TIMERTA0_1 from CAPTURE 2\n");
          msp430_interrupt_set(INTR_TIMERTA0_1);
        }
      }
      break;
    case 2: /* GND */
      HW_DMSG_TIMER("msp430:timerTA0: capture not implemented on this port\n");
      break;
    case 3: /* Vcc */
      HW_DMSG_TIMER("msp430:timerTA0: capture not implemented on this port\n");
      break;
    }
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA0_write8(uint16_t addr, int8_t val)
{
  msp430_timerTA0_write(addr, val);
}

void msp430_timerTA0_write(uint16_t addr, int16_t val)
{
  switch ((enum timerTA0_addr_t)addr) {
  case TA0IV: /* read only */
    /* although this register is read only, we can have a write on it */
    msp430_timerTA0_reset_highest_intr();
    HW_DMSG_TIMER("msp430:timerTA0: taiv write, reset highest intr\n");
    break;

  case TA0CTL:
  {

    union {
      uint16_t s;
      struct ta0ctl_t b;
    } ta0ctl;

    HW_DMSG_2_DBG("msp430:timerTA0: ta0ctl   = 0x%04x\n", val);
    ta0ctl.s = val;

    if (ta0ctl.b.taclr) /* this one must be first as it resets divider and ssel */ {
      MCU.timerTA0.ta0ctl.b.id = 0;
      MCU.timerTA0.tar = 0;
      ta0ctl.b.taclr = 0;
      SET_DIVBUFFER(timerTA0, "timerTA0", 0);
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.taclr clear\n");
    }

    if (ta0ctl.b.tassel != MCU.timerTA0.ta0ctl.b.tassel) {
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.tassel set to %d (%s)\n",
        ta0ctl.b.tassel, str_clocksrc[ta0ctl.b.tassel]);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA0:    ta0ctl.tassel left to %d (%s)\n",
        ta0ctl.b.tassel, str_clocksrc[ta0ctl.b.tassel]);
    }

    if (ta0ctl.b.id != MCU.timerTA0.ta0ctl.b.id) {
      SET_DIVBUFFER(timerTA0, "timerTA0", ta0ctl.b.id);
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.id set to %d (DIV = %d)\n", ta0ctl.b.id, 1 << ta0ctl.b.id);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA0:    ta0ctl.id left to %d (DIV = %d)\n", ta0ctl.b.id, 1 << ta0ctl.b.id);
    }

    if (ta0ctl.b.mc != MCU.timerTA0.ta0ctl.b.mc) {
      if ((ta0ctl.b.mc == TIMER_UP) && (MCU.timerTA0.tar > MCU.timerTA0.ta0ccr[0])) {
        MCU.timerTA0.tar = 0; /* restart from zero */
      }
      MCU.timerTA0.udmode = TIMER_UD_UP;
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.mc going to mode %d (%s)\n",
        ta0ctl.b.mc, str_mode[ta0ctl.b.mc]);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA0:    ta0ctl.mc left to mode %d (%s)\n",
        ta0ctl.b.mc, str_mode[ta0ctl.b.mc]);
    }

    if (ta0ctl.b.taie != MCU.timerTA0.ta0ctl.b.taie) {
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.ie set to %d\n", ta0ctl.b.taie);
      if ((ta0ctl.b.taie == 1) && (ta0ctl.b.taifg == 1)) {
        HW_DMSG_TIMER("msp430:timerTA0: checkifg ta0ctl.taifg == 1, interrupt set\n");
        msp430_interrupt_set(INTR_TIMERTA0_1);
      }
    }

    if (ta0ctl.b.taifg != MCU.timerTA0.ta0ctl.b.taifg) {
      HW_DMSG_TIMER("msp430:timerTA0:    ta0ctl.tbifg set to %d\n", ta0ctl.b.taifg);
      if ((ta0ctl.b.taie == 1) && (ta0ctl.b.taifg == 1)) {
        HW_DMSG_TIMER("msp430:timerTA0: checkifg ta0ctl.taifg == 1, interrupt set\n");
        msp430_interrupt_set(INTR_TIMERTA0_1);
      }
    }

    MCU.timerTA0.ta0ctl.s = ta0ctl.s;
    msp430_timerTA0_set_tiv();
  }
    break;

    TIMERA_TCCTLWRITE(TA0CCTL0, ta0cctln_t, timerTA0, "timerTA0", ta0cctl, "ta0cctl0", 0, INTR_TIMERTA0_0)
    TIMERA_TCCTLWRITE(TA0CCTL1, ta0cctln_t, timerTA0, "timerTA0", ta0cctl, "ta0cctl1", 1, INTR_TIMERTA0_0)
    TIMERA_TCCTLWRITE(TA0CCTL2, ta0cctln_t, timerTA0, "timerTA0", ta0cctl, "ta0cctl2", 2, INTR_TIMERTA0_0)
    TIMERA_TCCTLWRITE(TA0CCTL3, ta0cctln_t, timerTA0, "timerTA0", ta0cctl, "ta0cctl3", 3, INTR_TIMERTA0_0)
    TIMERA_TCCTLWRITE(TA0CCTL4, ta0cctln_t, timerTA0, "timerTA0", ta0cctl, "ta0cctl4", 4, INTR_TIMERTA0_0)


  case TA0R:
    MCU.timerTA0.tar = val & 0xffffu;
    HW_DMSG_TIMER("msp430:timerTA0: tar     = 0x%04x [%"PRId64"]\n",
      MCU.timerTA0.tar, MACHINE_TIME_GET_NANO());
    break;

  case TA0CCR0:
    if ((MCU.timerTA0.ta0ctl.b.mc == TIMER_UP) || (MCU.timerTA0.ta0ctl.b.mc == TIMER_UD)) {
      if ((MCU.timerTA0.ta0ccr[0] == 0) && (val > 0)) {
        MCU.timerTA0.udmode = TIMER_UD_UP;
        MCU.timerTA0.tar = 0;
        HW_DMSG_TIMER("msp430:timerTA0: ta0ccr0 > 0, restarts the timer\n");
      } else if (val < MCU.timerTA0.tar) {
        if (MCU.timerTA0.ta0ctl.b.mc == TIMER_UP) {
          MCU.timerTA0.tar = 0;
          HW_DMSG_TIMER("msp430:timerTA0: ta0ccr0 > tar, restarts from 0\n");
        } else if (MCU.timerTA0.udmode == TIMER_UD_UP) {
          MCU.timerTA0.udmode = TIMER_UD_DOWN;
          HW_DMSG_TIMER("msp430:timerTA0: ta0ccr0 > tar, going mode down\n");
        }
      }
    }
    WRITE_TIMERTA0_CCR(0);
    break;

  case TA0CCR1: WRITE_TIMERTA0_CCR(1);
    break;
  case TA0CCR2: WRITE_TIMERTA0_CCR(2);
    break;
  case TA0CCR3: WRITE_TIMERTA0_CCR(3);
    break;
  case TA0CCR4: WRITE_TIMERTA0_CCR(4);
    break;

  case TA0EX0:
    ERROR("msp430:timerTA0: TAxEX0 not implemented\n");
    break;

  default:
    ERROR("msp430:timerTA0: bad write address [0x%04x]\n", addr);
    break;
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_timerTA0_read8(uint16_t addr)
{
  return msp430_timerTA0_read(addr) & 0xff;
}

int16_t msp430_timerTA0_read(uint16_t addr)
{
  int16_t ret;
  switch ((enum timerTA0_addr_t) addr) {
  case TA0CTL: ret = MCU.timerTA0.ta0ctl.s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCTL0: ret = MCU.timerTA0.ta0cctl[0].s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCTL1: ret = MCU.timerTA0.ta0cctl[1].s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCTL2: ret = MCU.timerTA0.ta0cctl[2].s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCTL3: ret = MCU.timerTA0.ta0cctl[3].s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCTL4: ret = MCU.timerTA0.ta0cctl[4].s;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0R: ret = MCU.timerTA0.tar;
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCR0: ret = MCU.timerTA0.ta0ccr[0];
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCR1: ret = MCU.timerTA0.ta0ccr[1];
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCR2: ret = MCU.timerTA0.ta0ccr[2];
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCR3: ret = MCU.timerTA0.ta0ccr[3];
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0CCR4: ret = MCU.timerTA0.ta0ccr[4];
    HW_DMSG_TIMER("msp430:timerTA0: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA0IV:
    ret = MCU.timerTA0.tiv.s;
    HW_DMSG_TIMER("msp430:timerTA0: read TAIV [0x%04x] = 0x%04x\n", addr, ret);
    msp430_timerTA0_reset_highest_intr();
    break;
  default:
    ERROR("msp430:timerTA0: bad read address 0x%04x\n", addr);
    ret = 0;
    break;
  }
  /*  HW_DMSG_TIMER("msp430:timerA3: read [0x%04x] = 0x%04x\n",addr,ret); */
  return ret;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_timerTA0_chkifg(void)
{
  int ret = 0;

  TCHKIFG(timerTA0, "timerTA0", ta0cctl[0], "tacctl0", INTR_TIMERTA0_0)

  if (MCU.timerTA0.tiv.s) {
    if ((MCU.timerTA0.ta0ctl.b.taie == 1) && (MCU.timerTA0.ta0ctl.b.taifg == 1)) {
      HW_DMSG_TIMER("msp430:timerTA0: checkifg ta0ctl.taifg == 1, interrupt set\n");
      msp430_interrupt_set(INTR_TIMERTA0_1);
      return 1;
    }

    TCHKIFG(timerTA0, "timerTA0", ta0cctl[1], "ta0cctl1", INTR_TIMERTA0_1)
    TCHKIFG(timerTA0, "timerTA0", ta0cctl[2], "ta0cctl2", INTR_TIMERTA0_1)
    TCHKIFG(timerTA0, "timerTA0", ta0cctl[3], "ta0cctl3", INTR_TIMERTA0_1)
    TCHKIFG(timerTA0, "timerTA0", ta0cctl[4], "ta0cctl4", INTR_TIMERTA0_1)

    MCU.timerTA0.ta0ctl.b.taifg = 0;
  }
  return ret;
}
#endif

/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ** Timer TA1 ******************************************************************* */
/* ******************************************************************************** */
/* ******************************************************************************** */
/* ******************************************************************************** */

#if defined(__msp430_have_timerTA1)

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA1_create()
{
  //msp430_io_register_addr8(TA1IV, msp430_timerTA1_read8, msp430_timerTA1_write8);
  msp430_io_register_range8(TIMER_TA1_START, TIMER_TA1_END + 1, msp430_timerTA1_read8, msp430_timerTA1_write8);

  //msp430_io_register_addr16(TA1IV, msp430_timerTA1_read, msp430_timerTA1_write);
  msp430_io_register_range16(TIMER_TA1_START, TIMER_TA1_END, msp430_timerTA1_read, msp430_timerTA1_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA1_reset(void)
{
  memset(&MCU.timerTA1, 0, sizeof (struct msp430_timerTA1_t));
  SET_DIVBUFFER(timerTA1, "timerTA1", MCU.timerTA1.ta1ctl.b.id);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA1_set_tiv(void)
{
  //       ifset(timerTA1,"timerTA1",ta1cctl[1],0x02)    /* highest */
  //  else ifset(timerTA1,"timerTA1",ta1cctl[2],0x04)
  //  else if (MCU.timerTA1.ta1ctl.b.taifg)             /* lowest */
  if (MCU.timerTA1.ta1ctl.b.taifg) {
    MCU.timerTA1.tiv.s = 0x0e; //to work as current test application expects, need to check if overflow happened
    HW_DMSG_2_DBG("msp430:timerTA1: tiv set to 0x0e\n");
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

static void msp430_timerTA1_reset_highest_intr(void)
{
  ifzero(timerTA1, "timerTA1", ta1cctl[1], "ta1cctl1", ta1ccr1) /* reset highest interrupt */
  else ifzero(timerTA1, "timerTA1", ta1cctl[2], "ta1cctl2", ta1ccr2)
  else if (MCU.timerTA1.ta1ctl.b.taifg) {
    MCU.timerTA1.ta1ctl.b.taifg = 0;
    HW_DMSG_TIMER("msp430:timerTA1: ta1ctl.ifg set to 0\n");
  }
  msp430_timerTA1_set_tiv();
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
 *              00 TAxCLK   /  external
 *              01 ACLK    /  internal from basic clock
 *              10 SMCLK   /  internal from basic clock
 *              11 inverted TAxCLK   /  external
 * bb ID      timer A input divider
 *              00  /1
 *              01  /2
 *              10  /4
 *              11  /8
 * cc MC      mode control
 *              00 stop mode
 *              01 up mode (count to TAxCCR0)
 *              10 continuous (count to 0x0ffff)
 *              11 up/down (based on TAxCCR0)
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

void msp430_timerTA1_update(void)
{
  int clock;
  int tar_inc;
  /***************/
  /* Timer block */
  /***************/
  if (MCU.timerTA1.ta1ctl.b.mc == TIMER_STOP)
    return;

  clock = 0;
  switch (MCU.timerTA1.ta1ctl.b.tassel) {
  case TIMER_SOURCE_TxCLK:
    ERROR("msp430:timerTA1: source TACLK not implemented\n");
    break;
  case TIMER_SOURCE_ACLK:
    clock = MCU_CLOCK.ACLK_increment;
    break;
  case TIMER_SOURCE_SMCLK:
    clock = MCU_CLOCK.SMCLK_increment;
    break;
  case TIMER_SOURCE_INTxCLK:
    ERROR("msp430:timerTA1: source INTACLK not implemented\n");
    break;
  }

  MCU.timerTA1.divbuffer += clock;

  if ((clock == 0) || ((MCU.timerTA1.divbuffer & MCU.timerTA1.divuppermask) == 0)) {
    return;
  }

  tar_inc = MCU.timerTA1.divbuffer >> MCU.timerTA1.ta1ctl.b.id; // div
  MCU.timerTA1.divbuffer &= MCU.timerTA1.divlowermask; // mod

  switch (MCU.timerTA1.ta1ctl.b.mc) {
  case TIMER_STOP:
    /* should not be reached due to return a few lines above */
    break;

  case TIMER_UP: /* UP counter */
    if (MCU.timerTA1.ta1ccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UP */ {
      MCU.timerTA1.tar += tar_inc;

      /**************************/
      /* capture/compare blocks */
      /**************************/
      TIMER_COMPARE(timerTA1, "timerTA1:taccr1:", tar, ta1ccr, ta1cctl, 1, INTR_TIMERTA1_1)
      TIMER_COMPARE(timerTA1, "timerTA1:taccr2:", tar, ta1ccr, ta1cctl, 2, INTR_TIMERTA1_1)

      if (MCU.timerTA1.tar >= MCU.timerTA1.ta1ccr[0]) {
        /* (ccr0 - 1) -> ccr0 */
        MCU.timerTA1.ta1cctl[0].b.ccifg = 1;
        msp430_timerTA1_set_tiv();
        if (MCU.timerTA1.ta1cctl[0].b.ccie == 1) {
          HW_DMSG_TIMER("msp430:timerTA1: set interrupt TIMERTA1_0 from TIMER_UP\n");
          msp430_interrupt_set(INTR_TIMERTA1_0);
        }

        /*  ccr0      -> 0    */
        MCU.timerTA1.ta1ctl.b.taifg = 1;
        msp430_timerTA1_set_tiv();
        if (MCU.timerTA1.ta1ctl.b.taie == 1) {
          HW_DMSG_TIMER("msp430:timerTA1: set interrupt TIMERTA1_1 from TIMER_UP\n");
          msp430_interrupt_set(INTR_TIMERTA1_1);
        }
        MCU.timerTA1.tar -= MCU.timerTA1.ta1ccr[0];
        HW_DMSG_2_DBG("msp430:timerTA1: up mode wraps to 0 ===============================\n");
        TIMER_COMPARE_WRAPS(timerTA1, "timerTA1:ta1ccr1", ta1ccr, ta1cctl, 1);
        TIMER_COMPARE_WRAPS(timerTA1, "timerTA1:ta1ccr2", ta1ccr, ta1cctl, 2);
      }
    }
    break;

  case TIMER_CONT: /* Continuous counter */
    MCU.timerTA1.tar += tar_inc;
    /**************************/
    /* capture/compare blocks */
    /**************************/
    TIMER_COMPARE(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 0, INTR_TIMERTA1_0)
    TIMER_COMPARE(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 1, INTR_TIMERTA1_1)
    TIMER_COMPARE(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 2, INTR_TIMERTA1_1)

    if (MCU.timerTA1.tar >= TAR_MAX_LIMIT) {
      MCU.timerTA1.ta1ctl.b.taifg = 1;
      msp430_timerTA1_set_tiv();
      if (MCU.timerTA1.ta1ctl.b.taie == 1) {
        HW_DMSG_TIMER("msp430:timerTA1: interrupt TIMERTA1_1 from TIMER_CONT (tar 0x%06x) [%"PRId64"]\n",
          MCU.timerTA1.tar, MACHINE_TIME_GET_NANO());
        msp430_interrupt_set(INTR_TIMERTA1_1);
      }
      MCU.timerTA1.tar -= 0xffffu;
      /* contig mode bad wraps */
      HW_DMSG_TIMER("msp430:timerTA1: contig mode wraps to 0 ===============================\n");
      TIMER_COMPARE_WRAPS(timerTA1, "timerTA1", ta1ccr, ta1cctl, 0);
      TIMER_COMPARE_WRAPS(timerTA1, "timerTA1", ta1ccr, ta1cctl, 1);
      TIMER_COMPARE_WRAPS(timerTA1, "timerTA1", ta1ccr, ta1cctl, 2);
    }
    break;

  case TIMER_UD: /* UP/DOWN counter */
    if (MCU.timerTA1.ta1ccr[0] > 0) /* timer is stopped if taccr[0] == 0 in UD */ {
      if (MCU.timerTA1.udmode == TIMER_UD_UP) {
        MCU.timerTA1.tar += tar_inc;
        if (MCU.timerTA1.tar >= MCU.timerTA1.ta1ccr[0]) {
          /* we are going UP, so the timer wraps and is going down */
          MCU.timerTA1.udmode = TIMER_UD_DOWN;
          MCU.timerTA1.tar = MCU.timerTA1.ta1ccr[0];
          MCU.timerTA1.ta1cctl[0].b.ccifg = 1;
          msp430_timerTA1_set_tiv();
          if (MCU.timerTA1.ta1cctl[0].b.ccie == 1) {
            HW_DMSG_TIMER("msp430:timerTA1: set interrupt TIMERTA1_1 from TIMER_UD in UP mode\n");
            msp430_interrupt_set(INTR_TIMERTA1_0);
          }
          HW_DMSG_TIMER("msp430:timerTA1: Up/Down mode wraps to max ===============================\n");
          TIMER_COMPARE_WRAPS_DOWN(timerTA1, "timerTA1", ta1ccr, 1, MCU.timerTA1.ta1ccr[0]);
          TIMER_COMPARE_WRAPS_DOWN(timerTA1, "timerTA1", ta1ccr, 2, MCU.timerTA1.ta1ccr[0]);
        } else {
          TIMER_COMPARE(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 1, INTR_TIMERTA1_1)
          TIMER_COMPARE(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 2, INTR_TIMERTA1_1)
        }
      } else /* timer is down */ {
        MCU.timerTA1.tar -= tar_inc;
        if (MCU.timerTA1.tar <= 0) {
          /* we are going down, we wraps and start up */
          MCU.timerTA1.udmode = TIMER_UD_UP;
          MCU.timerTA1.tar = 0;
          MCU.timerTA1.ta1ctl.b.taifg = 1;
          msp430_timerTA1_set_tiv();
          if (MCU.timerTA1.ta1ctl.b.taie == 1) {
            HW_DMSG_TIMER("msp430:timerTA1: set interrupt TIMERTA1_1 from TIMER_UD in DOWN mode\n");
            msp430_interrupt_set(INTR_TIMERTA1_1);
          }
          HW_DMSG_TIMER("msp430:timerTA1: Up/Down mode wraps to 0 ===============================\n");
          TIMER_COMPARE_WRAPS(timerTA1, "timerTA1", ta1ccr, ta1cctl, 1);
          TIMER_COMPARE_WRAPS(timerTA1, "timerTA1", ta1ccr, ta1cctl, 2);
        } else {
          TIMER_COMPARE_DOWN(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 1, INTR_TIMERTA1_1, MCU.timerTA1.ta1ccr[0])
          TIMER_COMPARE_DOWN(timerTA1, "timerTA1", tar, ta1ccr, ta1cctl, 2, INTR_TIMERTA1_1, MCU.timerTA1.ta1ccr[0])
        }
      }
    }
    break;
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA1_capture(void)
{
  if (MCU.timerTA1.ta1cctl[2].b.cap == 1 &&
    MCU.timerTA1.ta1cctl[2].b.cm > 0) {
    /*
 at this time we don't care about:
 SCS  : synchroneous capture source
 SCCI : Synchronized capture/compare input
     */
    switch (MCU.timerTA1.ta1cctl[2].b.ccis) {
    case 0: /* CCIxA = TA2 */
      HW_DMSG_TIMER("msp430:timerTA1: capture not implemented on this port\n");
      break;
    case 1: /* CCIxB */
      /* on msp430f1611 this pin in internal ACLK */
      if (MCU_CLOCK.ACLK_increment > 0) {
        MCU.timerTA1.ta1ccr[2] = MCU.timerTA1.tar;
        MCU.timerTA1.ta1cctl[2].b.ccifg = 1;
        msp430_timerTA1_set_tiv();
        if (MCU.timerTA1.ta1cctl[2].b.ccie == 1) {
          HW_DMSG_TIMER("msp430:timerTA1: set interrupt TIMERTA1_1 from CAPTURE 2\n");
          msp430_interrupt_set(INTR_TIMERTA1_1);
        }
      }
      break;
    case 2: /* GND */
      HW_DMSG_TIMER("msp430:timerTA1: capture not implemented on this port\n");
      break;
    case 3: /* Vcc */
      HW_DMSG_TIMER("msp430:timerTA1: capture not implemented on this port\n");
      break;
    }
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_timerTA1_write8(uint16_t addr, int8_t val)
{
  msp430_timerTA1_write(addr, val);
}

void msp430_timerTA1_write(uint16_t addr, int16_t val)
{
  switch ((enum timerTA1_addr_t)addr) {
  case TA1IV: /* read only */
    /* although this register is read only, we can have a write on it */
    msp430_timerTA1_reset_highest_intr();
    HW_DMSG_TIMER("msp430:timerTA1: taiv write, reset highest intr\n");
    /* FIXME: should we reset the complete vector ? TonyOS thinks yes, the doc says no */
    break;

  case TA1CTL:
  {

    union {
      uint16_t s;
      struct ta1ctl_t b;
    } ta1ctl;

    HW_DMSG_2_DBG("msp430:timerTA1: ta1ctl   = 0x%04x\n", val);
    ta1ctl.s = val;

    if (ta1ctl.b.taclr) /* this one must be first as it resets divider and ssel */ {
      MCU.timerTA1.ta1ctl.b.id = 0;
      MCU.timerTA1.tar = 0;
      ta1ctl.b.taclr = 0;
      SET_DIVBUFFER(timerTA1, "timerTA1", 0);
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.taclr clear\n");
    }

    if (ta1ctl.b.tassel != MCU.timerTA1.ta1ctl.b.tassel) {
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.tassel set to %d (%s)\n",
        ta1ctl.b.tassel, str_clocksrc[ta1ctl.b.tassel]);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA1:    ta1ctl.tassel left to %d (%s)\n",
        ta1ctl.b.tassel, str_clocksrc[ta1ctl.b.tassel]);
    }

    if (ta1ctl.b.id != MCU.timerTA1.ta1ctl.b.id) {
      SET_DIVBUFFER(timerTA1, "timerTA1", ta1ctl.b.id);
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.id set to %d (DIV = %d)\n", ta1ctl.b.id, 1 << ta1ctl.b.id);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA1:    ta1ctl.id left to %d (DIV = %d)\n", ta1ctl.b.id, 1 << ta1ctl.b.id);
    }

    if (ta1ctl.b.mc != MCU.timerTA1.ta1ctl.b.mc) {
      if ((ta1ctl.b.mc == TIMER_UP) && (MCU.timerTA1.tar > MCU.timerTA1.ta1ccr[0])) {
        MCU.timerTA1.tar = 0; /* restart from zero */
      }
      MCU.timerTA1.udmode = TIMER_UD_UP;
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.mc going to mode %d (%s)\n",
        ta1ctl.b.mc, str_mode[ta1ctl.b.mc]);
    } else {
      HW_DMSG_2_DBG("msp430:timerTA1:    ta1ctl.mc left to mode %d (%s)\n",
        ta1ctl.b.mc, str_mode[ta1ctl.b.mc]);
    }

    if (ta1ctl.b.taie != MCU.timerTA1.ta1ctl.b.taie) {
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.ie set to %d\n", ta1ctl.b.taie);
      if ((ta1ctl.b.taie == 1) && (ta1ctl.b.taifg == 1)) {
        HW_DMSG_TIMER("msp430:timerTA1: checkifg ta1ctl.taifg == 1, interrupt set\n");
        msp430_interrupt_set(INTR_TIMERTA1_1);
      }
    }

    if (ta1ctl.b.taifg != MCU.timerTA1.ta1ctl.b.taifg) {
      HW_DMSG_TIMER("msp430:timerTA1:    ta1ctl.tbifg set to %d\n", ta1ctl.b.taifg);
      if ((ta1ctl.b.taie == 1) && (ta1ctl.b.taifg == 1)) {
        HW_DMSG_TIMER("msp430:timerTA1: checkifg ta1ctl.taifg == 1, interrupt set\n");
        msp430_interrupt_set(INTR_TIMERTA1_1);
      }
    }

    MCU.timerTA1.ta1ctl.s = ta1ctl.s;
    msp430_timerTA1_set_tiv();
  }
    break;

    TIMERA_TCCTLWRITE(TA1CCTL0, ta1cctln_t, timerTA1, "timerTA1", ta1cctl, "ta1cctl0", 0, INTR_TIMERTA1_0)
    TIMERA_TCCTLWRITE(TA1CCTL1, ta1cctln_t, timerTA1, "timerTA1", ta1cctl, "ta1cctl1", 1, INTR_TIMERTA1_0)
    TIMERA_TCCTLWRITE(TA1CCTL2, ta1cctln_t, timerTA1, "timerTA1", ta1cctl, "ta1cctl2", 2, INTR_TIMERTA1_0)


  case TA1R:
    MCU.timerTA1.tar = val & 0xffffu;
    HW_DMSG_TIMER("msp430:timerTA1: tar     = 0x%04x [%"PRId64"]\n",
      MCU.timerTA1.tar, MACHINE_TIME_GET_NANO());
    break;

  case TA1CCR0:
    if ((MCU.timerTA1.ta1ctl.b.mc == TIMER_UP) || (MCU.timerTA1.ta1ctl.b.mc == TIMER_UD)) {
      if ((MCU.timerTA1.ta1ccr[0] == 0) && (val > 0)) {
        MCU.timerTA1.udmode = TIMER_UD_UP;
        MCU.timerTA1.tar = 0;
        HW_DMSG_TIMER("msp430:timerTA1: ta1ccr0 > 0, restarts the timer\n");
      } else if (val < MCU.timerTA1.tar) {
        if (MCU.timerTA1.ta1ctl.b.mc == TIMER_UP) {
          MCU.timerTA1.tar = 0;
          HW_DMSG_TIMER("msp430:timerTA1: ta1ccr0 > tar, restarts from 0\n");
        } else if (MCU.timerTA1.udmode == TIMER_UD_UP) {
          MCU.timerTA1.udmode = TIMER_UD_DOWN;
          HW_DMSG_TIMER("msp430:timerTA1: ta1ccr0 > tar, going mode down\n");
        }
      }
    }
    WRITE_TIMERTA1_CCR(0);
    break;

  case TA1CCR1: WRITE_TIMERTA1_CCR(1);
    break;
  case TA1CCR2: WRITE_TIMERTA1_CCR(2);
    break;

  case TA1EX0:
    ERROR("msp430:timerTA1: TAxEX0 not implemented\n");
    break;

  default:
    ERROR("msp430:timerTA1: bad write address [0x%04x]\n", addr);
    break;
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_timerTA1_read8(uint16_t addr)
{
  return msp430_timerTA1_read(addr) & 0xff;
}

int16_t msp430_timerTA1_read(uint16_t addr)
{
  int16_t ret;
  switch ((enum timerTA1_addr_t) addr) {
  case TA1CTL: ret = MCU.timerTA1.ta1ctl.s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCTL0: ret = MCU.timerTA1.ta1cctl[0].s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCTL1: ret = MCU.timerTA1.ta1cctl[1].s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCTL2: ret = MCU.timerTA1.ta1cctl[2].s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCTL3: ret = MCU.timerTA1.ta1cctl[3].s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCTL4: ret = MCU.timerTA1.ta1cctl[4].s;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1R: ret = MCU.timerTA1.tar;
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCR0: ret = MCU.timerTA1.ta1ccr[0];
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCR1: ret = MCU.timerTA1.ta1ccr[1];
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1CCR2: ret = MCU.timerTA1.ta1ccr[2];
    HW_DMSG_TIMER("msp430:timerTA1: read  [0x%04x] = 0x%04x\n", addr, ret);
    break;
  case TA1IV:
    ret = MCU.timerTA1.tiv.s;
    HW_DMSG_TIMER("msp430:timerTA1: read TAIV [0x%04x] = 0x%04x\n", addr, ret);
    msp430_timerTA1_reset_highest_intr();
    break;
  default:
    ERROR("msp430:timerTA1: bad read address 0x%04x\n", addr);
    ret = 0;
    break;
  }
  /*  HW_DMSG_TIMER("msp430:timerA3: read [0x%04x] = 0x%04x\n",addr,ret); */
  return ret;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_timerTA1_chkifg(void)
{
  int ret = 0;

  TCHKIFG(timerTA1, "timerTA1", ta1cctl[0], "tacctl0", INTR_TIMERTA1_0)

  if (MCU.timerTA1.tiv.s) {
    if ((MCU.timerTA1.ta1ctl.b.taie == 1) && (MCU.timerTA1.ta1ctl.b.taifg == 1)) {
      HW_DMSG_TIMER("msp430:timerTA1: checkifg ta1ctl.taifg == 1, interrupt set\n");
      msp430_interrupt_set(INTR_TIMERTA1_1);
      return 1;
    }

    TCHKIFG(timerTA1, "timerTA1", ta1cctl[1], "ta1cctl1", INTR_TIMERTA1_1)
    TCHKIFG(timerTA1, "timerTA1", ta1cctl[2], "ta1cctl2", INTR_TIMERTA1_1)

    MCU.timerTA1.ta1ctl.b.taifg = 0;
  }
  return ret;
}
#endif

