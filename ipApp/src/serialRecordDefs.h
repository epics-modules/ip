/* This file defines the actual values associated with the menus in the serialRecord.dbd file. 
 * The .dbd file and this file must be consistent!! 
 */
#define NUM_BAUD_CHOICES 11
static int baud_choices[NUM_BAUD_CHOICES]={300,600,1200,2400,4800,9600,19200,38400,57600,115200,230400};
#define NUM_PARITY_CHOICES 3
static char parity_choices[NUM_PARITY_CHOICES]={'N','E','O'};
#define NUM_DBIT_CHOICES 4
static int data_bit_choices[NUM_DBIT_CHOICES]={5,6,7,8};
#define NUM_SBIT_CHOICES 2
static int stop_bit_choices[NUM_SBIT_CHOICES]={1,2};
#define NUM_FLOW_CHOICES 2
static char flow_control_choices[NUM_FLOW_CHOICES]={'N','H'};
