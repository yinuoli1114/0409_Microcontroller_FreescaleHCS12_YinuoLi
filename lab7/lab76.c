  /***********************************************************************
; ECE 362 - Experiment 6 - Fall 2013     
;
; Dual-channel LED bar graph display                    
;***********************************************************************
;	 	   			 		  			 		  		
; Completed by: < Yinuo Li >
;               < 3439 - L >
;               < div4 >
;
;
; Academic Honesty Statement:  In entering my name above, I hereby certify
; that I am the individual who created this HC(S)12 source file and that I 
; have not copied the work of any other student (past or present) while 
; completing it. I understand that if I fail to honor this agreement, I will 
; receive a grade of ZERO and be subject to possible disciplinary action.
;                             
;
;***********************************************************************/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

// All funtions after main should be initialized here

// Note: inchar and outchar can be used for debugging purposes

char inchar(void);
void outchar(char x);
			 		  		
//  Variable declarations  	   			 		  			 		       
int tenthsec = 0;  // One-tenth second flag
int leftpb = 0;    // left pushbutton flag
int rightpb = 0;    // right pushbutton flag
int runstp = 0;    // run/stop flag                         
int rticnt = 0;    // RTICNT (variable)
int prevpbl = 0;    // previous state of pushbuttons (variable)
int prevpbr = 0;

int thresh1 = 0x1A;
int thresh2 = 0x4D;
int thresh3 = 0x80;
int thresh4 = 0xB4;
int thresh5 = 0xE6;


//int renum = 3;
	 	   		
/***********************************************************************
Initializations
***********************************************************************/
void  initializations(void) {

// Set the PLL speed (bus clock = 24 MHz)
  		CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  		PLLCTL = PLLCTL | 0x40; // turn on PLL
  		SYNR = 0x02;            // set PLL multiplier
  		REFDV = 0;              // set PLL divider
  		while (!(CRGFLG & 0x08)){  }
  		CLKSEL = CLKSEL | 0x80; // engage PLL
  
// Disable watchdog timer (COPCTL register)
          COPCTL = 0x40;    //COP off - RTI and COP stopped in BDM-mode

// Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts
         SCIBDH =  0x00; //set baud rate to 9600
         SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
         SCICR1 =  0x00; //$9C = 156
         SCICR2 =  0x0C; //initialize SCI for program-driven operation
         
//  Initialize Port AD pins 7 and 6 for use as digital inputs
	   DDRAD = 0; 		//program port AD for input mode
         ATDDIEN = 0xC0; //program PAD7 and PAD6 pins as digital inputs

         
//  Add additional port pin initializations here  (e.g., Other DDRs, Ports) 
    DDRT = 0XFF;
   // PTT_PTT4 = 0;
    
//  Define bar graph segment thresholds (THRESH1..THRESH5)
//  NOTE: These are binary fractions



//  Add RTI/interrupt initializations here
         RTICTL =0X70;
         CRGINT_RTIE = 1;
         asm {
          CLI
         }
         rticnt = 0;
         
}

void clocki() {
  PTT_PTT4 = 1;
  PTT_PTT4 = 0;
}

void print(int renum){
   int i;
   //PTT_PTT3 = 0;
   for(i = 0; i < 5 - renum; i++){
    PTT_PTT3 = 0;
    clocki();
   }                                           
         
  
   for(i = 0; i < renum; i++) {
    PTT_PTT3 = 1;
    clocki();
   }
   
 
}
  
  
	 		  			 		  		
/***********************************************************************
Main (non-terminating loop)
***********************************************************************/
void main(void) {
	initializations(); 		  			 		  		
	EnableInterrupts;


  for(;;) {


/*; Main program loop (state machine)
 Start of main program-driven polling loop
;
main
	 	   			 		  			 		  		
;  If the "tenth second" flag is set, then
;    - clear the "tenth second" flag */

    if(tenthsec == 1)
    {
       tenthsec = 0;
       //PTT_PTT4 = 0;
    ATDCTL2 = 0xc0;
    ATDCTL3 = 0x10;
    ATDCTL4 = 0x80;
    ATDCTL5 = 0x10;
    if(runstp == 1){
       //runstp = 0;
      if(ATDDR0H <= thresh1){
        print(0);
      }
      else if((ATDDR0H > thresh1) && (ATDDR0H <= thresh2)){
        print(1);
    } 
      else if((ATDDR0H > thresh2) && (ATDDR0H <= thresh3)){
        print(2);
     }
      else if((ATDDR0H > thresh3) && (ATDDR0H <= thresh4)){
        print(3);
     }
      else if((ATDDR0H > thresh4) && (ATDDR0H <= thresh5)){
        print(4);
      }
      else if(ATDDR0H > thresh5){
        print(5);
      }
      
       if(ATDDR1H <= thresh1){
        print(0);
      }
      else if((ATDDR1H > thresh1) && (ATDDR1H <= thresh2)){
        print(1);
     } 
      else if((ATDDR1H > thresh2) && (ATDDR1H <= thresh3)){
        print(2);
     }
      else if((ATDDR1H > thresh3) && (ATDDR1H <= thresh4)){
        print(3);
     }
      else if((ATDDR1H > thresh4) && (ATDDR1H <= thresh5)){
        print(4);
      }
      else if(ATDDR1H > thresh5){
        print(5);
      }
     }
    }
  
 /*   
;    - if "run/stop" flag is set, then
;       - initiate ATD coversion sequence
;       - apply thresholds to converted values
;       - determine 5-bit bar graph bit settings for each input channel
;       - transmit 10-bit data to external shift register
;    - endif
;  Endif

	 	   			 		  			 		  		
;  If the left pushbutton ("stop BGD") flag is set, then:
;    - clear the left pushbutton flag
;    - clear the "run/stop" flag (and "freeze" BGD)
;    - turn on left LED/turn off right LED (on docking module)
;  Endif*/
    if(leftpb == 1) 
    {
      leftpb = 0;
      runstp = 0;
      PTT_PTT1 = 1;
      PTT_PTT0 = 0;
      
    }
    

	 	   			 		  			 		  		
/*
;  If the right pushbutton ("start BGD") flag is set, then
;    - clear the right pushbutton flag
;    - set the "run/stop" flag (enable BGD updates)
;    - turn off left LED/turn on right LED (on docking module)
;  Endif  */
	 	 if(rightpb == 1) 
	 	 {
      rightpb = 0;
      runstp = 1;
      PTT_PTT1 = 0;
      PTT_PTT0 = 1;
      outchar(ATDDR0H);
    }
   
 

  			 		  			 		  		

    
    
    _FEED_COP(); /* feeds the watchdog timer */
  } /* loop forever */
  /* make sure that you never leave main */
}



/***********************************************************************                       
; RTI interrupt service routine: rti_isr
;
;  Initialized for 5-10 ms (approx.) interrupt rate - note: you need to
;    add code above to do this
;
;  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
;
;  If change in state from "high" to "low" detected, set pushbutton flag
;     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
;     Recall that pushbuttons are momentary contact closures to ground
;
;  Also, keeps track of when one-tenth of a second's worth of RTI interrupts
;      accumulate, and sets the "tenth second" flag         	   			 		  			 		  		
;***********************************************************************/
interrupt 7 void RTI_ISR( void)
{
  	// set CRGFLG bit to clear RTI device flag
  	CRGFLG = CRGFLG | 0x80;
  	//CRGFLG = CRGFLG & 0x7f;
  //	rticnt++;
  	tenthsec = 1;
  
  	
  	if(prevpbl == 1 && PTAD_PTAD7 == 0){
  	  leftpb = 1;
  	  prevpbl = 0;
  	}
  	if(PTAD_PTAD7 == 1){
  	  prevpbl = 1;
  	}
  	
  	if(prevpbr == 1 && PTAD_PTAD6 == 0){
  	  rightpb = 1;
  	  prevpbr = 0;
  	}
  	if(PTAD_PTAD6 == 1){
  	  prevpbr = 1;
  	}

}


/***********************************************************************
; Character I/O Library Routines for 9S12C32 (for debugging only)
;***********************************************************************
; Name:         inchar
; Description:  inputs ASCII character from SCI serial port and returns it
;***********************************************************************/
char  inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
 
}

/***********************************************************************
; Name:         outchar
; Description:  outputs ASCII character passed in outchar()
;                  to the SCI serial port
;***********************************************************************/
void outchar(char ch) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = ch;
}


