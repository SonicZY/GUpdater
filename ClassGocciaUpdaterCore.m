        /*,.／ ,:ｸ,,. -‐'"´　 ,.
 　　 ／i,. ‐'´　　　　　　　￣￣￣￣Z
 　 /　　　　　　　　　/　　　　　　　　＿二=-
 . /　　　　　　　　　/　　　　　　　　￣`ヽ､＿,,
 .|　　　　　　　　　/　　　　　　　　　　　　　／
 |＿＿＿　　　　　　　　　　　　　　　　＜~
 ||`',''''ｰ-｀'＝=r-- ､＿　　　　　　　　＿三=-
 ||)）　　　　　　.i　　　　｀"''‐-､＿＿　二=‐
 ヾ';ｰ=======::'､＿＿＿＿＿＿__/＼'~
 　　]‐ [ - }----/`''ｰ､ i--`i　　　 |　　 ヽ,
 　 /　 ￣　　　　　　 彡;, | |　　　 ＼　 l~
 　i,＿,,　　　　　　　　彡ﾉ丿　　　　 L`''ヽ
 　　　L＿　　　　　　 ミ／ `ヾ' ,'-亠='＝'''''ヽ,
 　 ／f,_　　　 ,.ril;ﾐ'' ,,.. !-'"´　　　　　　　　ヽ
 　 ヾ､､write the code,change the world// ＿i
 　　　　　|sonic.xie'"´　　　＿＿,,,.. --‐‐'''"￣　　ヽ
 　　　　　　r"　　　, -‐''"*/


//  ClassGocciaUpdaterCore.m
//  GocciaUpdaterOC
//
//  Created by Xie Sonic on 14/10/1.
//  Copyright (c) 2014年 Xie Sonic. All rights reserved.


#import "ClassGocciaUpdaterCore.h"
#import "cmdblock.h"
#import "SerialPortSample.h"

@implementation ClassGocciaUpdaterCore

int segNum;//segment(begin with @) number of firmware
NSMutableArray  *cmdList;
NSString* fileContentString;
int localVersion;
int remoteVersion;
uint16_t crcValue;
uint8_t indexValue;

-(id)init
{
    if(self=[super init])
    {
        localVersion =0;
        remoteVersion =-1;
    }
    return self;
}

/*
 get the firmware from serverDB or local disk
 if localVersion same with remoteVersion,read
 firmware from local disk,otherwise get firmware
 from serverDB.
 if internet is unconnected,then get remoteVersion
 false,ask user weather to use the local firmware.
 */
-(int)readFirmware
{
    NSError* error=nil;
    
    //first of all ,get the remoteVersion from ServerDB.
    
    if (remoteVersion == -1) {
        NSURL *url = [NSURL URLWithString:@"http://115.29.4.187/fireware/Goccia_V73_930_NCAS.txt"];
        fileContentString = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:&error];
        if(nil!=error){
            NSLog(@"%@", error);
            exit(-1);
        }else{
            NSLog(@"%@",fileContentString);
            remoteVersion = 1;
        }
    }
    //    if ((remoteVersion== -1)||(localVersion == remoteVersion)) {
    //        //can't connect to server
    //
    //        //openfile from local disk
    //        NSString* path=[NSString stringWithUTF8String:PATH];
    //        fileContentString=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&error];
    //
    //        if (nil!=error) {
    //            NSLog(@"%@", error);
    //            exit(-1);
    //        }
    //
    //    }
    //    else if(localVersion!=remoteVersion) {
    //        //openfile from server
    //        NSURL *url = [NSURL URLWithString:@"file:///Users/gx/Desktop/test_utf8.txt"];
    //        fileContentString = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:&error];
    //
    //        if (nil!=error) {
    //            NSLog(@"%@", error);
    //            exit(-1);
    //        }
    //    }
    
    //make sure the string is valid
    NSString * regex = @"^[A-F0-9@\r\n ]+$";
    NSPredicate *pred = [NSPredicate predicateWithFormat:@"SELF MATCHES %@", regex];
    BOOL isMatch = [pred evaluateWithObject:fileContentString];
    
    if (!isMatch) {
        NSLog(@"the firmware file content invalid character");
        return -1;
    }
    return 0;
    
}

-(int)doParse
{
    indexValue = 0;
    cmdList = [[NSMutableArray alloc]init];
    //split contentString to seg String separate by @
    NSArray* strlist= [fileContentString componentsSeparatedByString:@"@"];
    if (([strlist count]==0)||([strlist count]>=70)) {
        exit(-1);
    }
    for(NSString* cmd in strlist)
    {
        if (![cmd  isEqual: @""]) {
            indexValue++;
            CmdBlock *cmdb = [[CmdBlock alloc] init];
            cmdb->index = indexValue;
            cmdb->DatatoSend =[self get_AddrLenData_String:cmd];
            cmdb->crc16 = crcValue;
            cmdb->DatatoSend = [NSString stringWithFormat:@"$DATA,%02X,%@,%04X%@",indexValue,cmdb->DatatoSend,crcValue,@"\r\n"];
            [cmdList addObject:cmdb];
        }
    }
    segNum = indexValue;
    
    //now we can send cmd->DatatoSend to com port.
    return 0;
}

-(NSString *) get_AddrLenData_String:(NSString *)s
{
    NSString* addr;
    NSString* data;
    int16_t len;
    NSString* s1;
    
    //remove skpace character
    s1 = [s stringByReplacingOccurrencesOfString:@" " withString:@""];
    
    //remove Newline character
    s1 = [s1 stringByReplacingOccurrencesOfString:@"\r" withString:@""];
    s1 = [s1 stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    
    addr = [s1 substringWithRange:NSMakeRange(0,4)];//addr
    data = [s1 substringFromIndex:4];//data
    len = [data length]/2; // 2 character/byte
    union{
        int16_t a;
        char b[2];
    }un_len;
    
    char calc_crc[520] = {};
    int i =0;
    //index
    calc_crc[i++] = indexValue;
    //addr
    un_len.a = strtol([addr UTF8String],0,16);
    calc_crc[i++] = un_len.b[0];
    calc_crc[i++] = un_len.b[1];
    //len
    un_len.a = len;
    calc_crc[i++] = un_len.b[0];
    calc_crc[i++] = un_len.b[1];
    
    //data
    NSString* s2 = [s substringFromIndex:6];
    s2 = [s2 stringByReplacingOccurrencesOfString:@"\r" withString:@""];
    s2 = [s2 stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    s2 = [s2 stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSArray* numlist= [s2 componentsSeparatedByString:@" "];
    
    for(NSString* num in numlist)
    {
        calc_crc[i++] = strtol([num UTF8String],0,16);
    }
    
    crcValue = crc16(calc_crc,i);
    return [NSString stringWithFormat:@"%@,%04X,%@",addr,len,data];
}

-(int)getLocalVersion
{
    return 0;
}

-(int)getRemoteversion
{
    return 0;
}

@end
