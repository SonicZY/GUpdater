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


//  AppDelegate.m
//  test
//
//  Created by Xie Sonic on 14/10/1.
//  Copyright (c) 2014年 Sonic.Xie All rights reserved.
//




#import "AppDelegate.h"


@interface AppDelegate ()
{
    ClassGocciaUpdaterCore *GocciaUpdater;
    NSThread* updateThread;
}

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    
    GocciaUpdater = [[ClassGocciaUpdaterCore alloc] init];
    kern_return_t result;
    result =initSerialPort();
    switch (result) {
        case EX_UNAVAILABLE:
            NSLog(@"SerialPort Notfound");
            //        self.tipsText.setTitleWithMnemonic("Wrring:SerialPort nofound!!")
            //        self.bnStart.enabled = false
            break;
        case EX_OK:
            NSLog(@"initialize SerialPort Scuess");
            //        self.tipsText.setTitleWithMnemonic("Please choose the file you want to update into Goccia")
            break;
        default:
            NSLog(@"SerialPort initialize false");
            //        self.tipsText.setTitleWithMnemonic("SerialPort initialize false")
            //        self.bnStart.enabled = false
    }
    
}

- (IBAction)start:(id)sender {
    
    //get firmware
    int result = [GocciaUpdater readFirmware];
    if (result == 0) {
        NSLog(@"readFirmware ok");
    }
    
    //prepare firmware data
    result = [GocciaUpdater doParse];
    if (result == 0) {
        NSLog(@"doParse ok");
    }
    
    //now data is ready,let's open a new thread to handle the send job
    updateThread = [[NSThread alloc]initWithTarget:self selector:@selector(doUpdate) object:nil];
    [updateThread start];
    
}

-(void)doUpdate{
    //find goccia by send "$GO\r\n"
    BOOL result= false;
    unsigned int tryTime = 0;
    
    NSThread*  currentThread = [NSThread currentThread];
    
    const char *s = [@"$GO\r\n" UTF8String];
    tryTime = 20;
    while(!result){
        result = sendMSG(s,10);
        tryTime--;
        if(tryTime==0){
            break;
        }
        if (currentThread.isCancelled) {
            [NSThread exit];
        }
    }
    
    //goccia found,then send "$START,len,CRC\r\n" to Start it
    char crcData[6];
    crcData[0] = 0;
    crcData[1] = 0;
    crcData[2] = 0;
    crcData[3] = 0;
    crcData[4] = 1;
    crcData[5] = GocciaUpdater->segNum;
    uint16_t crc = crc16(crcData, 6);
    
    NSString* STARTstr = [NSString stringWithFormat:@"$START,%02X,%04X\r\n",GocciaUpdater->segNum,crc];
    const char *ss = [STARTstr UTF8String];
    tryTime = 20;
    result = false;
    while(!result){
        result = sendMSG(ss,1);
        tryTime--;
        if(tryTime==0){
            break;
        }
        if (currentThread.isCancelled) {
            [NSThread exit];
        }
    }
    
    //ok,now send data.
    for (int i =0; i<GocciaUpdater->segNum; i++) {
        tryTime = 20;
        result = false;
        while(!result){
            CmdBlock *strTosend = [GocciaUpdater->cmdList objectAtIndex:i];
            const char *sss = [strTosend->DatatoSend UTF8String];
            result = sendMSG(sss,30);
            tryTime--;
            if(tryTime==0){
                break;
            }
            if (currentThread.isCancelled) {
                [NSThread exit];
            }
        }
    }
    
    
    
}




-(void)cancelThread{
    
    [self performSelector:@selector(operationDone) onThread:updateThread withObject:nil waitUntilDone:NO]; /// 向子线程发送事件
    
}

- (void)operationDone
{
    [updateThread cancel];
    [NSThread exit];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
