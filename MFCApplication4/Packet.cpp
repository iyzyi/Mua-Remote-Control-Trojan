#include "pch.h"
#include "Packet.h"
#include "ClientManage.h"

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


CPacket::CPacket(CClient* pClient) {
	m_pClient					= pClient;			// 客户端对象
	m_dwConnId					= pClient->m_dwConnectId;			// socket连接的ID，HP-Socket用此来抽象不同socket号

	m_dwPacketLength			= 0;				// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	m_PacketHead				= PACKET_HEAD();	// 包头
	m_pbPacketBody				= NULL;				// 包体

	m_dwPacketBodyLength		= 0;				// 包体长度

	m_pbPacketPlaintext			= NULL;				// 封包明文数据（照例不包括开头表示长度的4字节）
	m_pbPacketCiphertext		= NULL;				// 封包密文数据（照例不包括开头表示长度的4字节）
}


CPacket::CPacket() {
	m_dwConnId					= 0;	

	m_dwPacketLength			= 0;			
	m_PacketHead				= PACKET_HEAD();	
	m_pbPacketBody				= NULL;		

	m_dwPacketBodyLength		= 0;

	m_pbPacketPlaintext			= NULL;	
	m_pbPacketCiphertext		= NULL;
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {		// 如果这里不是用xmalloc，而是直接用栈，那这里free必然崩溃。还在想怎么改。暂时先不允许栈吧，PacketBody都得xmalloc申请。// 现在改成new和delete
		delete m_pbPacketBody;
	}

	if (m_pbPacketPlaintext) {
		delete m_pbPacketPlaintext;
	}

	if (m_pbPacketCiphertext) {
		delete m_pbPacketCiphertext;
	}
}


// 解析封包，不可与PacketCombine在同一个Packet对象中同时使用
VOID CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {
	m_dwPacketLength = dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// 复制封包（密文）
	m_pbPacketCiphertext = new BYTE[dwPacketLength];
	memcpy(m_pbPacketCiphertext, pbData, dwPacketLength);

	// 解密封包。接收到的封包是密文，解密出来的明文长度一定比密文短，
	// 所以这里直接用密文的长度dwPacketLength就足够了
	m_pbPacketPlaintext = new BYTE[dwPacketLength];
	DWORD dwPacketCiphertextLength = dwPacketLength;
	DWORD dwPacketPlaintextLength = m_pClient->m_Crypto.Decrypt(m_pbPacketCiphertext, dwPacketCiphertextLength, m_pbPacketPlaintext);

	// 封包长度更新为解密后的明文封包的长度
	m_dwPacketLength = dwPacketPlaintextLength;	
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;
	
	// 解析包头
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlaintext);

	// 拷贝包体
	m_pbPacketBody = new BYTE[m_dwPacketBodyLength];
	memcpy(m_pbPacketBody, m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_dwPacketBodyLength);
}


// 组装封包，不可与PacketParse在同一个Packet对象中同时使用
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	// 设置（计算）包头
	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// 暂不引入校验
	m_PacketHead.bySplitNum = 0;				// 暂不考虑分片

	// 包头转成buffer形式
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	// 拷贝包体
	m_pbPacketBody = new BYTE[dwPacketBodyLength];
	memcpy(m_pbPacketBody, pbPacketBody, dwPacketBodyLength);

	// 组包，不过不包括前4字节，因为HP-Socket的Pack模式，在收发数据的时候会自动添上或删去
	m_pbPacketPlaintext = new BYTE[m_dwPacketLength];
	memcpy(m_pbPacketPlaintext, pbPacketHead, PACKET_HEAD_LENGTH);
	memcpy(m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_pbPacketBody, m_dwPacketBodyLength);

	// 加密封包
	DWORD dwPacketPlaintextLength = m_dwPacketLength;
	DWORD dwPacketCiphertextLength = m_pClient->m_Crypto.GetCiphertextLength(dwPacketPlaintextLength);
	m_pbPacketCiphertext = new BYTE[dwPacketCiphertextLength];
	m_pClient->m_Crypto.Encrypt(m_pbPacketPlaintext, dwPacketPlaintextLength, m_pbPacketCiphertext);

	// 封包长度更新为加密后的密文长度，此时m_dwPacketBodyLength包体长度用不到，就不更新了。
	m_dwPacketLength = dwPacketCiphertextLength;
}