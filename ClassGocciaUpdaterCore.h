//
//  ClassGocciaUpdaterCore.h
//  GocciaUpdaterOC
//
//  Created by Xie Sonic on 14/10/1.
//  Copyright (c) 2014年 Xie Sonic. All rights reserved.
//

#import <Foundation/Foundation.h>

#define PATH "/Users/kj/Desktop/dict.txt"

@interface ClassGocciaUpdaterCore : NSObject
{
@public
    int segNum;//segment(begin with @) number of firmware
    NSMutableArray  *cmdList;
@private
    
}

-(NSString *) get_AddrLenData_String:(NSString *)s;
-(int)readFirmware;
-(int)doParse;
-(int)getLocalVersion;
-(int)getRemoteversion;
@end
