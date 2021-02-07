#include "pch.h"
#include "Packet.h"

// 统一声明：
// 本类中的封包包括封包包头和封包包体
// 封包包头的定义详见PACKET_HEAD结构体
// 并不包括开头的4字节长度哦，这个是由HP-Socket负责的


//CPacket::CPacket(PBYTE pbData, DWORD dwLength) {
//
//	// TODO：解密封包
//
//
//	// 封包长度
//	m_dwPacketLength = dwLength;
//	m_dwPacketBodyLength = dwLength - PACKET_HEAD_LENGTH;
//
//	// 解析包头
//	m_PacketHead = PACKET_HEAD((PBYTE)pbData);
//
//	// 拷贝包体
//	m_pPacketBody = CopyBuffer(pbData, dwLength - PACKET_HEAD_LENGTH, PACKET_HEAD_LENGTH);
//	if (m_pPacketBody == NULL){
//		printf("xmalloc(%d) PacketBody failed\n", dwLength);
//	}
//	else {
//		printf("命令号 = 0x%x\n校验和 = 0x%x\n分片数 = 0x%x\n", m_PacketHead.wCommandId, m_PacketHead.dwCheckSum, m_PacketHead.bySplitNum);
//		printf("封包长度 = 0x%x\n包体长度 = 0x%x\n包体明文数据: \n", m_dwPacketLength, m_dwPacketBodyLength);
//		PrintChars((CHAR*)(m_pPacketBody), m_dwPacketBodyLength);
//	}
//}
//
//// bySplitNum表示封包分片数量
//CPacket::CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum) {
//
//	
//
//	// TODO：加密封包
//
//}


CPacket::CPacket(CONNID dwConnID) {
	m_dwConnId					= dwConnID;			// socket连接的ID，HP-Socket用此来抽象不同socket号

	m_dwPacketLength			= 0;				// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	m_PacketHead				= PACKET_HEAD();	// 包头
	m_pbPacketBody				= NULL;				// 包体
	m_dwPacketBodyLength		= 0;				// 包体长度

	m_pbPacketPlainData			= NULL;				// 封包明文数据（照例不包括开头表示长度的4字节）
	m_pbPacketCipherData		= NULL;				// 封包密文数据（照例不包括开头表示长度的4字节）
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {		// 如果这里不是用xmalloc，而是直接用栈，那这里free必然崩溃。还在想怎么改。暂时先不允许栈吧，PacketBody都得xmalloc申请。
		xfree(m_pbPacketBody);
	}

	if (m_pbPacketPlainData) {
		xfree(m_pbPacketPlainData);
	}

	if (m_pbPacketCipherData) {
		xfree(m_pbPacketCipherData);
	}
}


// 解析封包，不可与PacketCombine在同一个Packet对象中同时使用
VOID CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {
	m_dwPacketLength = dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	m_pbPacketCipherData = CopyBuffer(pbData, dwPacketLength);

	// TODO : 解密封包
	m_pbPacketPlainData = CopyBuffer(m_pbPacketCipherData, m_dwPacketLength);

	// 解析包头
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlainData);

	// 拷贝包体
	m_pbPacketBody = CopyBuffer(m_pbPacketPlainData, m_dwPacketBodyLength, PACKET_HEAD_LENGTH);
}


// 组装封包，不可与PacketParse在同一个Packet对象中同时使用
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// 暂不引入校验
	m_PacketHead.bySplitNum = 0;				// 暂不考虑分片

	m_pbPacketBody = pbPacketBody;

	// 包头转成buffer形式
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	// 组包，不过不包括前4字节，因为HP-Socket的Pack模式，在收发数据的时候会自动添上或删去
	m_pbPacketPlainData = (PBYTE)xmalloc(m_dwPacketLength);					// 整条封包明文内容
	memcpy(m_pbPacketPlainData, pbPacketHead, PACKET_HEAD_LENGTH);
	memcpy(m_pbPacketPlainData + PACKET_HEAD_LENGTH, m_pbPacketBody, m_dwPacketBodyLength);


	// TODO : 加密封包
	m_pbPacketCipherData = (PBYTE)xmalloc(m_dwPacketLength);
	memcpy(m_pbPacketCipherData, m_pbPacketPlainData, m_dwPacketLength);
}