#pragma once

#include "pch.h"
#include "Crypto.h"
#include "ModuleManage.h"
#include "SocketClient.h"


// 管理所有客户端
class CClientManage {
public:
	CClientManage();
	~CClientManage();



// 存放CSocket的链表
public:

	// Client链表的第一个结点，这个结点的所有数据都是空的，
	// 仅用来索引链表的头部，只有m_pNextClient这个数据是非空的。
	// m_pClientListHead->m_pNextClient存放真正的第一个Client的地址
	CClient*						m_pClientListHead;

	// 该结点指向链表最后一个结点，这个结点是有数据的结点
	//（除非链表为空，此时m_pClientListHead=m_pClientListTail）
	CClient*						m_pClientListTail;

	// 客户端数量，即主socket数量
	DWORD							m_dwClientNum;							

	// 链表操作的锁
	CRITICAL_SECTION				m_ClientLock;					


	// 双向链表
	VOID AddNewClientToList(CClient *pClient);

	// 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
	BOOL DeleteClientFromList(CONNID dwConnectId);
	VOID DeleteClientFromList(CClient *pClient);

	VOID DeleteAllClientFromList();

	// Client是否存在于链表中，存在则返回Client地址，不存在返回NULL
	CClient* SearchClient(CONNID dwConnectId);

	// CSocketClient是否存在，存在则返回其地址，不存在返回nullptr
	// 注意，这个查找的是CSocketClient，原理是遍历每个CClient中的存放CSocketClient的链表
	CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId);

	// 我们假定一台机子只运行一个Mua客户端（被控端），所以一个IP就是一个客户端。
	// 子socket属于哪个客户端，完全就是靠IP来分辨的
	// TODO 以后可以改成TOKEN分辨。主socket上线前发来TOKEN，然后后续的子socket都用这个TOKEN。实现起来不麻烦的。
	CClient* CClientManage::SearchClientByIp(CONNID dwConnectId);
};





