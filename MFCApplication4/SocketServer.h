#pragma once

#include "pch.h"
#include "Packet.h"
#include "Misc.h"
#include "DoubleLinkedList.h"


// 继承自CTcpServerListener
class CSocketServer : public CTcpServerListener {
public:

	CSocketServer();
	~CSocketServer();

	//VOID PacketParse(PBYTE pbData, DWORD dwLength);
	//VOID PacketCombine();

	VOID SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);
	VOID SendPacketToALLClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength);

	VOID StartSocketServer();
	VOID StopSocketServer();

public:
	CTcpPackServerPtr			m_Server;

	//doubleLinkedList<CONNID>	m_ClientList;			// 双向链表，用于记录连接中的ConnectId

protected:
	// 与管理Client连接相关
	VOID ListAddClient(CONNID ConnectId);
	VOID ListDeleteClient(CONNID ConnectId);

	// CTcpServerListener的抽象函数（用于回调）全都得实现，不然会报错：不能实例化抽象类
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnHandShake(ITcpServer* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
};