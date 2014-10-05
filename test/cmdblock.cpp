#include "cmdblock.h"

CmdBlock::CmdBlock()
    :
    index(0), addr(0), len(0), data(0), crc16(0)
{
}

CmdBlock::~CmdBlock()
{
    if (data) delete data;
}
