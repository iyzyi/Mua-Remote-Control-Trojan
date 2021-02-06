#pragma once

#include "pch.h"
#include "Misc.h"

// 这里的包头长度不包含表示封包长度的那4个字节
#define PACKET_HEAD_LENGTH (sizeof(PACKET_HEAD))

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
		this->wCommandId = GetWordFromBuffer(pbData, 0);
		this->dwCheckSum = GetDwordFromBuffer(pbData, 2);
		this->bySplitNum = GetByteFromBuffer(pbData, 6);
	}

	_PACKET_HEAD(){
		this->wCommandId = 0;
		this->dwCheckSum = 0;
		this->bySplitNum = 0;
	}

}PACKET_HEAD, *PPACKET_HEAD;


typedef struct _PACKET {
	DWORD				dwPacketLength;			// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	PACKET_HEAD			PacketHead;				// 包头
	PBYTE				pPacketBody;			// 包体

	DWORD				dwPacketBodyLength;		// 包体长度

	// pbPacketBuffer是从包头开始的缓冲区（当然也不含开头4个字节的长度）
	_PACKET(PBYTE pbData, DWORD dwLength) {
		this->dwPacketLength			= dwLength;
		this->PacketHead				= PACKET_HEAD((PBYTE)pbData);
		this->pPacketBody				= pbData + PACKET_HEAD_LENGTH;
		this->dwPacketBodyLength		= dwPacketLength - PACKET_HEAD_LENGTH;
	}

} PACKET, *PPACKET;


// 继承自CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	VOID PacketParse(PBYTE pbData, DWORD dwLength);
	VOID PacketCombine();

	VOID SendPacket();
	VOID SendPacketToALLClient();

	VOID InitSocketServer();
	VOID StopSocketServer();

public:
	CTcpPackServerPtr		m_Server;

protected:
	// CTcpServerListener的抽象函数（用于回调）全都得实现，不然会报错：不能实例化抽象类
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnHandShake(ITcpServer* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
};


enum COMMAND{
	SHELL_REMOTE,			// 远程shell	
	FILE_TRANSFOR,			// 文件传输
	SCREEN_MONITOR			// 屏幕监控

};