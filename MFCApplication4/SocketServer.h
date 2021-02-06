#pragma once

#include "pch.h"

#define PACKET_HEAD_LENGTH (sizeof(PACKET_HEAD))


typedef struct _PACKET_HEAD {
	DWORD dwLength;					// 整个封包的长度(不包括这表示长度的4个字节)
	WORD dwCommandId;				// 命令号
	DWORD dwCheckSum;				// 序列号

} PACKET_HEAD, *PPACKET_HEAD;


typedef struct _PACKET {
	PACKET_HEAD PacketHead;			// 包头
	PBYTE PacketBody;				// 包体
} PACKET, *PPACKET;


// 继承自CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	VOID PacketParse();
	VOID PacketCombine();

	VOID SendPacket();
	VOID SendPacketToALLClient();

	VOID InitSocketServer();
	VOID StopSocketServer();

public:
	CTcpPackServerPtr	m_Server;

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


enum {
	SHELL_REMOTE,
	FILE_TRANSFOR,

};