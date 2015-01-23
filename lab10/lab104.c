/***********************************************************************
; ECE 362 - Experiment 10 - Fall 2013
;***********************************************************************
;	 	   			 		  			 		  		
; Completed by: < Yinuo Li>
;               < 3439-L >
;               < 04 >
;
;
; Academic Honesty Statement:  In entering my name above, I hereby certify
; that I am the individual who created this HC(S)12 source file and that I 
; have not copied the work of any other student (past or present) while 
; completing it. I understand that if I fail to honor this agreement, I will 
; receive a grade of ZERO and be subject to possible disciplinary action.
;                             
;***********************************************************************
;
; The objective of this lab is to control the speed of a small D.C.
; motor using pulse width modulation.  The PWM duty cycle will be
; a function of an input analog D.C. voltage (range: 0 to 5 volts).
; The speed of the motor will be determined by the number of pulses
; detected by the pulse accumulator (estimated over a 1.0 second
; integration period); an updated estimate of the motor RPM will
; be displayed once every second.  The timer (TIM) will be used
; to drive the ATD sample/PWM update rate (every one-tenth second)
; and the display update rate (every second).  The RPM estimate  
; will be based on the number of pulses accumulated from the motor's 
; 64-hole chopper over a 1.0 second integration period (divided by 28,
; to estimate the gear head output shaft speed).  The real time   
; interrupt (RTI) will be used to sample the pushbutton state.
;
; The docking module pushbuttons and LEDs will be used as follows:
; - left pushbutton (PAD7): stop motor (if running)
; - right pushbutton (PAD6): start motor (if stopped)
; - left LED (PT1): on if motor stopped
; - right LED (PT0): on if motor running
;
; The RPM value will be displayed on the terminal screen and on the
; first line of the LCD. A bar graph showing the percent-of-max 
; will be displayed on the second line of the LCD.
;
; All parts needed to build the motor interface circuit are available
; from the Instrument Room (EE 162) - see schematic in lab document.
;
;***********************************************************************/
#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

// All funtions after main should be initialiezed here
void rdisp(void);
void bco(char tbyte);
void shiftout(char inst);
void lcdwait(void);
void send_byte(char tbyte);
void send_i(char inst);
void chgline(char curpos);
void print_c(char tbyte);
void pmsglcd(char *, int);
void outchar(char ch);

//  Variable declarations  	   			 		  			 		

       
int leftpb	= 0;  // left pushbutton flag
int rightpb	= 0;  // right pushbutton flag
int prevpbl = 0;
int prevpbr = 0;
int runstp	= 0;  // motor run/stop flag
int onesec = 0;  //  one second flag
int tenths	= 0;  // tenth of a second flag
int onecnt	= 0; // ONECNT (variable)
int tencnt	= 0; // TENCNT (variable)
int tin	= 0;    // SCI transmit display buffer IN pointer
int tout	= 0;  // SCI transmit display buffer OUT pointer
int tsize	= 8;  // size of transmit buffer
char tbuf[8]	= {0};  // SCI transmit display buffer
long int pulsescnt   = 0;
// Note: int size is 65,536 (It is not big enough to count the pulses each second so use long)!!
// long int size is 4,294,967,296

// ASCII character definitions
int CR = 0x0D;//Return   

//;LCD COMMUNICATION BIT MASKS
int RS = 0x04;     //;RS pin mask (PTT[2])
int RW = 0x08;     //;R/W pin mask (PTT[3])
int LCDCLK  = 0x40;     //;LCD EN/CLK pin mask (PTT[6])

//;LCD INSTRUCTION CHARACTERS
int LCDON = 0x0F;     //;LCD initialization command
int LCDCLR = 0x01;     //;LCD clear display command
int TWOLINE = 0x38;     //;LCD 2-line enable command

int CURMOV = 0xFE;     //;LCD cursor move instruction

int LINE1 = 0x80;     //;LCD line 1 cursor position
int LINE2 = 0xC0;     //;LCD line 2 cursor position

unsigned char A;
	 	   		
/***********************************************************************
Initializations
***********************************************************************/
void  initializations(void) {

//; Set the PLL speed (bus clock = 24 MHz)
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

// Disable watchdog timer (COPCTL register)
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

// Initialize asynchronous serial port (SCI) for 9600 baud, no interrupts
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  dec=26
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
         
         
/*; Initialize TIM Ch 7 (TC7) for periodic interrupts every 10.0 ms
;    Enable timer subsystem                         
;    Set channel 7 for output compare
;    Set appropriate pre-scale factor and enable counter reset after OC7
;    Set up channel 7 to generate 10 ms interrupt rate
;    Initially disable TIM Ch 7 interrupts	 	   			 		  		

	 		  		
*/	 	   			 		  			 		  		
//;  < add TIM initialization code here >
  TSCR1_TEN = 1; //   Enable timer subsystem
  TIOS_IOS7 = 1; //   Set channel 7 for output compare
  //  Set appropriate pre-scale factor and enable counter reset after OC7  
  TSCR2 = 0x0c;  //prescale factor to 16 and enable counter reset after OC7
  TC7 = 15000;  //   Set up channel 7 to generate 1 ms interrupt rate
  TIE_C7I = 1; //   Initially disable TIM Ch 7 interrupts

/*
; Initialize the PWM unit to produce a signal with the following
; characteristics on PWM output channel 3:
;   - sampling frequency of approximately 100 Hz
;   - left-aligned, negative polarity
;   - period register = $FF (yielding a duty cycle range of 0% to 100%,
;     for duty cycle register values of $00 to $FF 
;   - duty register = $00 (motor initially stopped)
;                         
; IMPORTANT: Need to set MODRR so that PWM Ch 3 is routed to port pin PT3
;
; BE CAREFUL NOT TO SHORT THE MOTOR COIL DRIVE SIGNAL TO GROUND!
;*/ 	   			 		  			 		  		
//;  < add PWM initialization code here >
  MODRR_MODRR3 = 1; //PT3 used as PWM Ch 3 output
  PWME_PWME3 = 1; //enable PWM ch 3
  PWMPOL_PPOL3 = 1; //set ch 3 with active high polarity
  PWMCTL = 0x00; //no concatenate (8-bit)
  PWMCAE = 0x00; //left-aligned output mode
  PWMPER3 = 0xFF; //set max 8-bit period 255
  PWMDTY3 = 0x00; //initially clear duty register
  PWMPRCLK_PCKB = 0x06; //set clock b = 375000 Hz prescale = 64
  PWMSCLB = 0x07; //set Clock SB = 105 Hz
  PWMCLK_PCLK3 = 1; //select Clock SB for ch3

/*; Initialize the ATD to sample a D.C. input voltage (range: 0 to 5V)
; on Channel 0 (connected to a 10K-ohm potentiometer). The ATD should
; be operated in a program-driven (i.e., non-interrupt driven), normal
; flag clear mode using nominal sample time/clock prescaler values,
; 8-bit, unsigned, non-FIFO mode.
;                         
; Note: Vrh (the ATD reference high voltage) is connected to 5 VDC and
;       Vrl (the reference low voltage) is connected to GND on the 
;       9S12C32 kit.  An input of 0v will produce output code 00h,
;       while an input of 5.00 volts will produce output code FFh.
*/	 	   			 		  			 		  		
//;  < add ATD initialization code here >
  ATDCTL2 = ATDCTL2 | 0x80; //enable ATD converter
  ATDCTL3_FIFO = 0; //None-FIFO mode
  //Conversions per Sequence is 2 due to 2 input
  ATDCTL3_S8C = 0;
  ATDCTL3_S4C = 0;
  ATDCTL3_S2C = 0;
  ATDCTL3_S1C = 1; 
	ATDDIEN = 0xc0; 	//enable digital input for on chip buttons   			 		

  			 		  		

/*; Initialize the pulse accumulator (PA) for event counting mode,
; and to increment on negative edges (no PA interrupts will be utilized,
; since overflow should not occur under normal operating conditions).
*/		     
//;  < add PA initialization code here >
  PACTL_PAEN = 1; //enable PA

//; Initialize the RTI for an 8.192 ms interrupt rate.
//;  < add RTI initialization code here >
  CRGINT = CRGINT | 0x80; //enable RTI
  RTICTL = 0x70; //set interrupt rate 8.192 ms   

//; Initialize SPI for baud rate of 6.25MHz, MSB first
//; - Consult 3.1.3 of the SPI Block User Guide     
//;  < add SPI initialization code here >
  SPICR1 = 0x50; //enable SPI,Master mode, interrupts off, CPOL = 0, CPHA = 0, slave select disabled, 

//data transferred most significant bit first
  // set SPR to 1
  SPIBR_SPR0 = 0;
  SPIBR_SPR1 = 0;
  SPIBR_SPR2 = 0;
  // set SPPR to 1
  SPIBR_SPPR0 = 0;
  SPIBR_SPPR1 = 0;
  SPIBR_SPPR2 = 1;


//; General port pin/data direction register initializations, etc.
   DDRT_DDRT3 = 1; //enablt output of PWM
   DDRT_DDRT4 = 1; //enable register select output
   DDRT_DDRT5 = 1; //enable LCD read/write selection output
   DDRT_DDRT6 = 1; //enable LCD Clock output
  
   DDRT_DDRT0 = 1; //enable onchip LED
   DDRT_DDRT1 = 1; //enable onchip LED
   DDRM_DDRM4 = 1; //enable MOSI output
   DDRM_DDRM5 = 1; //enable SCK output
   DDRAD = 0;
   PTT = 0;
   PTM = 0;
   
//; Initialize the LCD
   PTT = PTT | LCDCLK; // - pull LCDCLK high (idle)
   PTT_PTT5 = 0;   // - pull R/W' low (write state)
   send_i(LCDON); // - turn on LCD (LCDON instruction)
   send_i(TWOLINE); // - enable two-line mode (TWOLINE instruction)
   send_i(LCDCLR);   // - clear LCD (LCDCLR instruction)
   lcdwait();  // - wait for 2ms so that the LCD can wake up   			 		  		

	 		  		

//;  < add any additional initialization code needed here >
   DDRT_DDRT7 = 0; //enable PT7 encoder input

 
	      
}
	 		  			 		  		
/***********************************************************************
Main
***********************************************************************/
void main(void) {
  DisableInterrupts
	initializations(); 		  			 		  		
	EnableInterrupts;
    bco('Y');bco('i');bco('n');bco('u');bco('o');
	  bco(' ');
	  bco('L');bco('i');
	  
	  
 while(1) {

  
//; If right pushbutton pressed, start motor and turn on right LED (left LED off)
  if (rightpb){
    rightpb = 0;
    runstp = 1;
    PTT_PTT0 = 1; //turn on right LED
    PTT_PTT1 = 0; //turn off left LED
    
    outchar(tbuf[0]);
  }
//; If left pushbutton pressed, stop motor and turn on left LED (right LED off)
  if (leftpb){
    leftpb = 0;
    runstp = 0;
    PTT_PTT0 = 0; //turn off right LED
    PTT_PTT1 = 1; //turn on left LED
  }
  
//; Every one-tenth second (when "tenths" flag set):
  if (tenths == 1 && runstp == 1) {
//;    - perform ATD conversion on Ch 0 
  ATDCTL5_MULT = 0; //start conversion
  while (!ATDSTAT0_SCF){}
//;    - copy the converted value to the PWM duty register for Ch 0
	PWMDTY3 = ATDDR0H;
	tenths = 0; //reset tenths flag
  }
  
  if(runstp == 0){
    PWMDTY3 = 0;
  }
	 	   			 		  			 		  		
//; Every second (when "onesec" flag set):
  if (onesec){
    onesec = 0;
    pulsescnt = PACNT;//  - read (and clear) the PACNT value
    PACNT = 0;
	  rdisp();
	  bco('R');bco('P');bco('M');bco(' ');bco('=');
	  bco((pulsescnt / 100) + 48);
	  bco(((pulsescnt / 10) % 10) + 48);
	  bco((pulsescnt % 10) + 48);
	  bco('LF');
    //;    - convert the PACNT value to BCD and display it
    //;      + on terminal screen using bco
    //;      + on first line of LCD in "                          RPM" format
    //;      + on second line of LCD display "    
 }
   
    
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}




/***********************************************************************                       
; RTI interrupt service routine: RTI_ISR
;
; Initialized for 8.192 ms interrupt rate
;
;  Samples state of pushbuttons (PAD7 = left, PAD6 = right)
;
;  If change in state from "high" to "low" detected, set pushbutton flag
;     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
;     Recall that pushbuttons are momentary contact closures to ground	
;***********************************************************************/
interrupt 7 void RTI_ISR(void)
{
  	// set CRGFLG bit 
  	CRGFLG = CRGFLG | 0x80;
      
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
// No need to add anything in the .PRM file, the interrupt number is included above

}

/***********************************************************************                       
;  TIM interrupt service routine
;
;  Initialized for 10.0 ms interrupt rate
;
;  Uses variable "tencnt" to track if one-tenth second has accumulated
;     and sets "tenths" flag 
;                         
;  Uses variable "onecnt" to track if one second has accumulated and
;     sets "onesec" flag		 		  			 		  		
;***********************************************************************/
interrupt 15 void TIM_ISR( void)
{
  // set TFLG1 bit 
 	TFLG1 = TFLG1 | 0x80; 
// No need to add anything in the .PRM file, the interrupt number is included above
  tencnt++;
  onecnt++;
  if (tencnt == 10) {
    tencnt = 0;
    tenths = 1;
  }
  if (onecnt == 100) {
    onesec = 1;
    onecnt = 0;
  }
}

/***********************************************************************                       
;  SCI (transmit section) interrupt service routine
;                         
;  Read status register to enable TDR write
;  Check status of TBUF
;  If EMPTY, disable SCI transmit interrupts and exit; else, continue
;  Access character from TBUF
;  Output character to SCI TDR
;  Increment TOUT mod TSIZE		  			 		  		
;***********************************************************************/
interrupt 20 void SCI_ISR( void)
{
  if(SCISR1_TDRE){
    SCIDRL = tbuf[tout];
    tout = (tout + 1) % tsize;
    SCICR2_SCTIE = 0;
  } else {
    SCICR2_SCTIE = 0;
  }
// No need to add anything in the .PRM file, the interrupt number is included above
  
}
/***********************************************************************                              
;  SCI buffered character output routine "bco"
;
;  Outputs character passed in the A register to the screen
;
;  Check TBUF status
;  If FULL, wait for space
;  Place character in TBUF
;  Increment TIN mod TSIZE
;  Enable SCI transmit interrupts
;***********************************************************************/
void bco(char tbyte)
{
  while(tout == (tin + 1) % tsize);
  tbuf[tin] = tbyte;
  tin = (tin + 1) % tsize;  
  SCICR2_SCTIE = 1;
}
     


/***********************************************************************                              
; RPM display routine - rdisp
;                         
; This routine starts by reading (and clearing) the 16-bit PA register.
; It then calculates an estimate of the RPM based on the number of
; pulses accumulated from the 64-hole chopper over a one second integration
; period and divides this value by 28 to estimate the gear head output
; shaft speed. Next, it converts this binary value to a 3-digit binary coded
; decimal (BCD) representation and displays the converted value on the
; terminal as "RPM = NNN" (updated in place).  Finally this RPM value, along
; with a bar graph showing ther percent-of-max of the current RPM value
; are shifted out to the LCD using pmsglcd.                
;
;***********************************************************************/
void rdisp()
{
	int i = 0; //bar graph loop counter
	int num = 0; //number of solid rectangles
	send_i(LCDCLR);  //clear the lcd display
	//display the rpm value
	pmsglcd("RPM = ",1);
  pulsescnt = pulsescnt * 60 / 28 / 64;
	print_c(((pulsescnt / 100) % 10) + 48);
	print_c(((pulsescnt / 10) % 10) + 48);
	print_c((pulsescnt % 10) + 48);
	
	//display the bar gragh
	num = pulsescnt * 10 / 308;
	if (num > 10){
		num = 10;
	}
	chgline(LINE2);
	for(i = 0; i < num; i++){
		print_c(0xFF);
	}
	//send_i(CURMOV);
	//send_i()
	for(i = 0; i < (13 - num); i++){  //keep printing spaces to move cursor
		print_c(' ');
	}
	//display the percentage
	if (num < 10) {	  
	num = num * 10;
	print_c(((num / 10) % 10) + 48);
	print_c((num % 10) + 48);
	print_c('%');
	} else {
	  send_i(CURMOV);
	  send_i(0xcc);
	  print_c('1');
	  print_c('0');
    print_c('0');
    print_c('%');
	}
}

/***********************************************************************                              
;  shiftout: Transmits the contents of register A to external shift 
;            register using the SPI.  It should shift MSB first.  
;             
;            MISO = PM[4]
;            SCK  = PM[5]
;***********************************************************************/
void shiftout(char inst)  //SPI automatically shiftout a string of 8-bit data
{
  int i;
  while (SPISR_SPTEF != 1) { } //read the SPTEF bit, continue if bit is 1
  SPIDR = inst;  
  for (i = 1;i <40; i++){
  }
 //write data to SPI data register
	//wait for 30 cycles for SPI data to shift out 
}

/***********************************************************************                              
;  lcdwait: Delay for 2 ms
;***********************************************************************/
void lcdwait(void)
{
 asm{
    pshy
    ldy #7000
loop:
    nop
    nop
    nop
    nop
    nop
    nop
    dey 
    dbeq y,exit
    bra loop
exit:
    puly    
        
 //write assembly code here (if needed!)
 }
 
}

/***********************************************************************                              
;  send_byte: writes contents of register A to the LCD
;***********************************************************************/
void send_byte(char character)
{
     //Shift out character
     //Pulse LCD clock line low->high
     //Wait 2 ms for LCD to process data
     outchar('3');
     shiftout(character);
     PTT_PTT6 = 0;
     PTT_PTT6 = 1;
     lcdwait();
     outchar('5');
     
}
/***********************************************************************                              
;  send_i: Sends instruction passed in register A to LCD  
;***********************************************************************/
void send_i(char character)
{
        //Set the register select line low (instruction data)
        //Send byte
        PTT_PTT4 = 0;
        outchar('2');
        send_byte(character);
        outchar('7');
        
}
/***********************************************************************                        
;  chgline: Move LCD cursor to the cursor position passed in A
;  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
;***********************************************************************/
void chgline(char position)
{
    send_i((char)CURMOV);
    send_i(position);
}

/***********************************************************************                       
;  print_c: Print character passed in register A on LCD ,            
;***********************************************************************/
void print_c(char character)
{
    
     PTT_PTT4 = 1;
     outchar('0');
     send_byte(character);
     
}

/***********************************************************************                              
;  pmsglcd: pmsg, now for the LCD! Expect characters to be passed
;           by call.  Registers should return unmodified.  Should use
;           print_c to print characters.  
;***********************************************************************/
void pmsglcd(char *mesg, int linenum)
{
    int i = 0;
    if(linenum == 1){
      chgline((char)LINE1);
    }
    if(linenum == 2){
      chgline((char)LINE2);
    }
    
    while(mesg[i] != 0){
      print_c(mesg[i]);
      i++;
    }

}


/*************************************
; Name:         outchar
; Description:  outputs ASCII character passed in outchar()
;                  to the SCI serial port
; Example:      outchar('x');
;***********************************************************************/
void outchar(char ch) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = ch;
}