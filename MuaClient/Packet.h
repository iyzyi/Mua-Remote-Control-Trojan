#pragma once

#include "pch.h"
#include "Misc.h"
#include "Crypto.h"


class CClient;


// 这里的包头长度不包含表示封包长度的那4个字节
// 不能用(sizeof(PACKET_HEAD))，结构体元素不一定连续
#define PACKET_HEAD_LENGTH 7

// 封包长度受HP-Socket的限制，最大为0x3FFFFF
#define PACKET_MAX_LENGTH 0x3FFFFF

// 包体最大长度
#define PACKET_BODY_MAX_LENGTH ((PACKET_MAX_LENGTH) - (PACKET_HEAD_LENGTH) - sizeof(DWORD))



typedef struct _PACKET_HEAD {
	WORD		wCommandId;					// 命令号
	DWORD		dwCheckSum;					// 序列号
	BYTE		bySplitNum;					// 封包分片数量, 最多255个分片，使得最大能够传输将近1G的数据。
											//	 BYTE就够用了，更大的数据量的话还是换个协议吧，这个通信协议没有校验机制。

	_PACKET_HEAD(PBYTE pbData) {
		wCommandId = GetWordFromBuffer(pbData, 0);
		dwCheckSum = GetDwordFromBuffer(pbData, 2);
		bySplitNum = GetByteFromBuffer(pbData, 6);
	}

	_PACKET_HEAD() {
		wCommandId = 0;
		dwCheckSum = 0;
		bySplitNum = 0;
	}

	VOID StructToBuffer(PBYTE pbOutBuffer) {
		WriteWordToBuffer(pbOutBuffer, wCommandId, 0);
		WriteDwordToBuffer(pbOutBuffer, dwCheckSum, 2);
		WriteByteToBuffer(pbOutBuffer, bySplitNum, 6);
	}

}PACKET_HEAD, *PPACKET_HEAD;

//
//typedef struct _PACKET {
//	DWORD				dwPacketLength;			// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
//	PACKET_HEAD			PacketHead;				// 包头
//	PBYTE				pPacketBody;			// 包体
//
//	DWORD				dwPacketBodyLength;		// 包体长度
//
//
//	_PACKET(DWORD dwPacketLength, PACKET_HEAD PacketHead, PBYTE pPacketBody) {
//		this->dwPacketLength			= dwPacketLength;
//		this->PacketHead				= PacketHead;
//		this->pPacketBody				= pPacketBody;
//
//		this->dwPacketBodyLength		= dwPacketLength - PACKET_HEAD_LENGTH;
//	}
//
//	// 一个引起C2512错的原因:当主类(类A)含有其他类（类B)的对象（注意是对象），
//	// 且未定义构造参数的时候，运行开始时调用了类A编译器自动给的默认构造函数，
//	// 该构造函数会自动调用类A里的所有成员的默认构造函数，
//	// 此时若类B无默认构造函数（比如当只定义了带参数的构造函数时，编译器为保证
//	// 构造函数的唯一性使得对象使用起来安全，是不会有自动给的隐藏的默认构造函数的）
//	_PACKET() {
//		this->dwPacketLength			= 0;
//		this->PacketHead				= PACKET_HEAD();
//		this->pPacketBody				= NULL;
//
//		this->dwPacketBodyLength		= 0;
//	}
//
//} PACKET, *PPACKET;





class CPacket {

public:

	// 接收到的封包用这个构造函数
	//CPacket(PBYTE pbData, DWORD dwLength);

	// 要发送的封包用这个构造函数
	//CPacket(COMMAND_ID wCommandId, PBYTE pbPacketBody, BYTE bySplitNum = 0);

	CPacket(CCrypto* pCrypto);
	CPacket();

	VOID PacketParse(PBYTE pbData, DWORD dwLength);
	VOID PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);


	~CPacket();

public:
	CONNID				m_dwConnId;

	CCrypto*			m_pCrypto;

	DWORD				m_dwPacketLength;			// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	PACKET_HEAD			m_PacketHead;				// 包头
	PBYTE				m_pbPacketBody;				// 包体

	DWORD				m_dwPacketBodyLength;		// 包体长度

	PBYTE				m_pbPacketPlaintext;
	PBYTE				m_pbPacketCiphertext;
};