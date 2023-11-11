/*
============================================================================
 Name        : ZM_xx.c
 Author      : PeterMaria van Herpen
 Version     : 1 (also update Version variable !)
 Description : This program communicates with the zoutmeter.
 	 	 	   It takes two parameters
 	 	 	   first parameter:
 	 	 	   	   n = normal operation
 	 	 	   	   v = verbose operation
 	 	 	   	   t = test mode In this mode a fake message is printed without communicating to the ZM
 	 	 	   	   h = help
 	 	 	   second parameter is the device name (/dev/ttyxxx)
 	 	 	   The return code is 0 for success, 1 for error.
			   In normal mode, the output is:
			   {"level":"12345","lock":"0","open":"0","wasOpen":"0","temp":"12345","version":"0"}<\n>
			   1234567890123456789012345678901234567890123456789012345678901234567890123456789012  3
			   		    1		  2			3		  4			5		  6			7         8    8
 ============================================================================
 */

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

char Version[]="1";		// Version of the program
char HelpText[]="This program communicates with the zoutmeter (ZM).\n"
				   "It takes two parameters\n first parameter:\n"
	 	 	 	   "   n = normal operation\n"
	 	 	 	   "   v = verbose operation\n"
				   "   t = test mode in which a fake message is printed without communicating to the zm"
	 	 	 	   "   h = help\n"
				   "second parameter is the device name (/dev/ttyxxx)\n"
		 		   "The return code is 0 for success, 1 for error.\n"
		 	       "In normal mode, the output is: \n"
				   "{\"level\":\"12345\",\"lock\":\"0\",\"open\":\"0\",\"wasOpen\":\"0\",\"temp\":\"12345\",\"version\":\"0\"}\n";

int rxBytes;			// Counter for teh number of received bytes
char zminput[100];		// Receive buffer for zm input data

int zm_handle;			// Handle for the serial I/O to ZM
char tx[]="R\n";		// Transmitbuffer to the ZM with the trigger command
	
char rxbuf[100];		// Buffer to receive data from zm


int main (int argc, char *argv[])
{
	// First check if the mandatory parameters are present
	if (argc!=3)
	{
		printf("Number of parameters is wrong. Received: %d should be 2.\n",argc);
		return(1);
	}
	if(strcmp(argv[1],"h")==0)
	{
		printf("ZM reader program, Version %s \n %s \n",Version, HelpText);
		return(0);
	}
	if(strcmp(argv[1],"t")==0)
	{
		// Testmode, produces a demo output
	
	    return(0);
	}
	// Part 1
	// Prepare the communication structures
	// termios is a nasty thing ...
	if(strcmp(argv[1],"v")==0){printf("parameters, mode = %s, device = %s\n", argv[1], argv[2]);}
	struct termios tio;						    // This structure holds all the configuration flags
	zm_handle=open(argv[2],O_RDWR | O_NOCTTY | O_NDELAY) ; // NonBLOCK is not set, so device wil block
	
	/* iflag word:
	 * BRKINT	Signal interrupt on break				no
	 * ICRNL	Map CR to NL on input					no
	 * IGNBRK	Ignore break condition					yes
	 * IGNCR	Ignore CR								yes
	 * IGNPAR	Ignore characters with parity errors	no
	 * INLCR	Map NL to CR on input					no
	 * INPCK	Enable input parity check				no
	 * ISTRIP	Strip character							no
	 * IXOFF	Enable start/stop input control			no
	 * IXON	Enable start/stop output control			no
	 * PARMRK	Mark parity errors						no
	 */
	tio.c_iflag = 0;
	tio.c_iflag = ( IGNBRK | IGNCR );
	
	/* oflag word:
	 * OPOST	Perform output processing				no
	 * OLCUC	Map lower case to upper on output		no
	 * ONLCR	Map NL to CR-NL on output				no
	 * OCRNL	Map CR to NL on output					no
	 * ONOCR	No CR output at column 0				yes
	 * ONLRET	NL performs CR function					no
	 * OFILL	Use fill characters for delay			no
	 * OFDEL	Fill is DEL else NUL.					no
	 * NLDLY	Select new-line delays: NL0 NL1			none (NL0)
	 * CRDLY	Select carriage-return delays: CR0 CR1 CR2 CR3 none (CR0)
	 * TABDLY	Select horizontal-tab delays: TAB0 TAB1 TAB2	none (TAB0)
	 * XTABS	Expand tabs to spaces					no
	 * BSDLY	Select backspace delays: BS0 BS1		none (BS0)
	 * VTDLY	Select vertical tab delays: VT0 VT1		none (VT0)
	 * FFDLY	Select form-feed delays: FF0 FF1		none (FF0)
	 */
	tio.c_oflag = 0 ; 		// Output flags - Turn off output processing completely.
	tio.c_oflag = ONOCR;

	/* cflag fields
	 * CLOCAL	Ignore modem status lines					yes
	 * CREAD	Enable receiver								yes
	 * CSIZE	Number of bits per byte CS5 CS6 CS7 CS8		CS7
	 * CSTOPB	Send two stop bits else one					no
	 * HUPCL	Hang up on last close						no
	 * PARENB	Parity enable								yes
	 * PARODD	Odd parity else even 						Even (=no)
	 */
	tio.c_cflag = 0; // eset all bits
	tio.c_cflag = (CS7|CREAD|CLOCAL|PARENB);
	
	
	/* lflag fields:
	 * ECHO	Enable echo											no
	 * ECHOE	Echo ERASE as an error-correcting backspace		no
	 * ECHOK	Echo KILL										no
	 * ECHONL	Echo \n											no
	 * ICANON	Canonical input (erase and kill processing)		no
	 * IEXTEN	Enable extended functions						no
	 * ISIG	Enable signals										no
	 * NOFLSH	Disable flush after interrupt, quit or suspend  no
	 * TOSTOP	Send SIGTTOU for background output				no
	 */
	tio.c_lflag=0;
	
	cfsetospeed(&tio,B9600);
	cfsetispeed(&tio,B9600);
	tcflush(zm_handle,TCIOFLUSH);
	tio.c_cc[VMIN]  = 1;				// read teh reply byte by byte
	tio.c_cc[VTIME] = 0; 
	tcsetattr(zm_handle,TCSANOW,&tio);
	/*******************************************************************/
	fcntl(zm_handle,F_SETFL,0);
	
	// Communication structure set up, send the initialisation command to the zm
	write(zm_handle,&tx,2);		// Send the initialisation character 'R\n'
	if(strcmp(argv[1],"v")==0){	printf("Initialisation sent \n");}
		
	// Receive the 83 or so bytes of the second reply. The transmission ends with a \n chracter. 
	// If an overflow happens, it is basiclly ignored. the actual content of the buffer are printed and that's it.
	if(strcmp(argv[1],"v")==0){printf("\nReceiving \n");}
	int i=0;
	bool endOfTransmission = false;
	while (! endOfTransmission)
	{
	    read(zm_handle,rxbuf,1);
	    zminput[i]=rxbuf[0];
	    if (rxbuf[0]==0x0A) { endOfTransmission = true;}
	    if (i> 90) { endOfTransmission = true;}
	    i++;
	}
	
	if(strcmp(argv[1],"v")==0){printf("\n Receive buffer is now(1):  %s \n",zminput);}
	zminput[84]=0; // to print the string as a string, end it with a null.
	if(strcmp(argv[1],"v")==0){printf("data zm is %s\n", zminput);}

	// all variables are now set up, print the output line
    printf("%s",zminput);
	return 0;
}


