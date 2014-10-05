//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>


//extern int  fileDescriptor;
char *dataToSend;
// Hold the original termios attributes so we can reset them
static struct termios gOriginalTTYAttrs;

// Function prototypes
//static kern_return_t findModems(io_iterator_t *matchingServices);
//static kern_return_t getModemPath(io_iterator_t serialPortIterator, char *bsdPath, CFIndex maxPathSize);
//static int openSerialPort(const char *bsdPath);
//static char *logString(char *str);
//static Boolean initializeModem(int fileDescriptor);
//static void closeSerialPort(int fileDescriptor);
int initSerialPort();
bool sendMSG(unsigned char *str,uint8_t delay);
int closeSerialPortForSafe();
uint16_t  crc16(uint8_t *data_p, uint16_t length);
void perform();

