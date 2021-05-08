#include <iomanip>

#pragma pack(1)

typedef struct packet_header{
        uint32_t ICode;   ////固定识别码
        uint16_t Command;   ////命令字
        uint16_t ProtocolVersion;  ////协议版本
        uint32_t TerminalID;     ////终端ID
        uint8_t  TerminalType;   /////终端类型
        uint32_t ReceiverID;     ////转发目标ID
        uint8_t  ReceiverType;   /////转发目标类型
        uint16_t DataLength;     ////数据段长度

        packet_header()
        {
            ICode = 0x7fb96ae9;
            Command = 0;
            ProtocolVersion = 0;
            TerminalID = 0;
            TerminalType = 4;
            ReceiverID = 0x00000000;
            ReceiverType = 1;
            DataLength = 0;
        }

}PACKET_HEADER;

#pragma pack()