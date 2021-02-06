#include "pch.h"
#include "Packet.h"


CPacket::CPacket(PBYTE pbData, DWORD dwLength) {

	// TODO：解密封包

	// 解析包头
	PACKET_HEAD PacketHead((PBYTE)pbData);

	// 拷贝包体
	PBYTE pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
	if (pPacketBody == NULL){
		printf("xmalloc(%d) failed\n", dwLength);
	}
	else {
		// 封包以PACKET结构体的形式存在
		m_Packet = PACKET(dwLength, PacketHead, pPacketBody);

		printf("命令号 = 0x%x\n校验和 = 0x%x\n分片数 = 0x%x\n", m_Packet.PacketHead.wCommandId, m_Packet.PacketHead.dwCheckSum, m_Packet.PacketHead.bySplitNum);
		printf("封包长度 = 0x%x\n包体长度 = 0x%x\n包体明文数据: \n", m_Packet.dwPacketLength, m_Packet.dwPacketBodyLength);
		PrintChars((CHAR*)(m_Packet.pPacketBody), m_Packet.dwPacketBodyLength);
	}
}

// bySplitNum表示封包分片数量
CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {

	// 代码

	// TODO：加密封包

}
	
	
CPacket::~CPacket() {

}