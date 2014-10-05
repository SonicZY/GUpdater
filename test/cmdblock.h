#ifndef CMDBLOCK_H
#define CMDBLOCK_H

#import <Foundation/Foundation.h>
#import <stdint.h>

@interface CmdBlock : NSObject
{
    @public
    uint8_t index;
    NSString * DatatoSend;
//    NSString* addr;
//    uint16_t len;
//    NSString* data;
    uint16_t crc16;
    
}
    -(void)run;

@end

#endif // CMDBLOCK_H
