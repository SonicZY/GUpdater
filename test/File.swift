//
//  CoreGocciaUpdate.swift
//  VLC_update
//
//  Created by Xie ZuYong on 14-6-21.
//  Copyright (c) 2014年 mac. All rights reserved.
//

import Foundation

class CoreGocciaUpdate :NSObject {
    
    let hexString :[String] = ["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"]
    
    struct CMD_Struct {
        var cmd: String = ""
        var addr: uint16 = 0
        var index: uint = 0
        var len: uint = 0
        var data = uint8[]()
        var CRCValue: uint16 = 0
        
        mutating func CRC_calc() {
            var crc_data = uint8[]()
            switch cmd {
            case "$DATA":
                CRCValue = calcCRC16(data)
            default:
                CRCValue = 0
            }
        }
        func detail(){
            println("cmd = \(cmd)")
            println("cmd = \(addr)")
            println("cmd = \(index)")
            println("cmd = \(len)")
            println("cmd = \(data)")
            println("cmd = \(CRCValue)")
        }
    }
    
    struct FirmwearInfo_Struct{
        //the number of the segments
        var segQuantity: uint8
        var MD5: String
        var size: UInt
    }
    
    var tempString :String = ""
    var arry_char2Send = CUnsignedChar[]()
    var cmd_struct = CMD_Struct()
    var firmwearInfo = FirmwearInfo_Struct(segQuantity: 0, MD5: "", size: 0)
    var tryTime = 0
    var result :Bool = false
    var delay:uint8 = 0
    
    var retry_times: uint8 = 0
    var readyToUpdate: Bool = false
    
    init () {
        super.init()
    }
    
    func getAddrAndDate(str:NSString,index:uint8) ->Bool{
        
        let myStr:NSString = str
        //cmd_struct.cmd = "$DATA," + hexNumToHexChar_8(index) + ","
        
        cmd_struct.cmd = "$DATA," + NSString(format:"%02X",index) + ","
        
        if let hexBytes  = myStr.componentsSeparatedByString(" "){
            
            let addrStr  = hexBytes[0] as NSString
            //取地址
            if addrStr.length == 4{
                cmd_struct.cmd += addrStr + ","
            }else{
                return false
            }
            
            //取长度
            cmd_struct.cmd += hexNumToHexChar_16(uint16(hexBytes.count)) + ","
            //取数据
            
            for x in 1..hexBytes.count{
                tempString = hexBytes[x] as NSString
                cmd_struct.cmd += tempString
                //cmd_struct.data.append(value)
            }
            //取crc
            cmd_struct.CRCValue = calcCRC16(cmd_struct.data)
            //命令类型
            cmd_struct.cmd += "," + hexNumToHexChar_16(cmd_struct.CRCValue) + "\r\n"
            
            println("getAddrAndDate() sucess")
            return true
        }else{
            println("invalid string")
            return false
        }
    }
    
    func sendGo(){
        cmd_struct.cmd = "$GO\r\n"
        cmd_struct.addr = 0
        cmd_struct.index = 0
        arry_char2Send.removeAll()
        for x in cmd_struct.cmd.utf8 {
            arry_char2Send.append(x)
        }
        //可以发送了
        tryTime = 100
        result = false
        delay = 0
        
    }
    
    func sendStart(){
        var crc_data = CUnsignedChar[]()
        cmd_struct.cmd = "$START,"
        cmd_struct.addr = 0
        cmd_struct.index = 0
        //清空数组
        arry_char2Send.removeAll()
        //填充数据数据
        crc_data.append(0x00)  // index
        crc_data.append(0x00)  // addr
        crc_data.append(0x00)  // addr
        crc_data.append(0x00)  // len
        crc_data.append(0x01)  // len
        crc_data.append(firmwearInfo.segQuantity)  // data
        cmd_struct.CRCValue = calcCRC16(crc_data)
        //cmd_struct.cmd = "$START," + hexNumToHexChar_8(firmwearInfo.segQuantity) + "," + hexNumToHexChar_16(cmd_struct.CRCValue) + "\r\n"
        cmd_struct.cmd = String(format:"$START,%02X,%04X\r\n",firmwearInfo.segQuantity,cmd_struct.CRCValue)
        for x in cmd_struct.cmd.utf8 {
            arry_char2Send.append(x)
        }
        //可以发送了
        tryTime = 20
        result = false
        delay = 1
    }
}