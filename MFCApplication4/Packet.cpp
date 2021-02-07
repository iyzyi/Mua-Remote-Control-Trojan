#include "pch.h"
#include "Packet.h"


CPacket::CPacket(PBYTE pbData, DWORD dwLength) {

	// TODO：解密封包


	// 封包长度
	m_dwPacketLength = dwLength;
	m_dwPacketBodyLength = dwLength - PACKET_HEAD_LENGTH;

	// 解析包头
	m_PacketHead = PACKET_HEAD((PBYTE)pbData);

	// 拷贝包体
	m_pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
	if (m_pPacketBody == NULL){
		printf("xmalloc(%d) PacketBody failed\n", dwLength);
	}
	else {
		printf("命令号 = 0x%x\n校验和 = 0x%x\n分片数 = 0x%x\n", m_PacketHead.wCommandId, m_PacketHead.dwCheckSum, m_PacketHead.bySplitNum);
		printf("封包长度 = 0x%x\n包体长度 = 0x%x\n包体明文数据: \n", m_dwPacketLength, m_dwPacketBodyLength);
		PrintChars((CHAR*)(m_pPacketBody), m_dwPacketBodyLength);
	}
}

// bySplitNum表示封包分片数量
CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {

	// 代码

	// TODO：加密封包

}
	
	
CPacket::~CPacket() {

}