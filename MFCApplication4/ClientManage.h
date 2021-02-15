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
	CClient* CClientManage::SearchClientByIp(CONNID dwConnectId);



	// 删除一个主socket对应的全部子socket
	// 在Client链表中搜索与之相同IP的其他client, 如果不是主socket，那么就认定为是这个主socket的子socket, 一同断开连接
	// 这里假定一个IP只上线一个主socket, 不然其他的子socket实在无法区分所属的主socket.
	//VOID DeleteAllChildClientByOneIP(CClient *pClient);




	



//
//
//// 存放CSocketClient的链表，均为子socket。如果需要用到主socket，请遍历存放CClient的链表，然后找到m_pMainSocketClient
//
//// 先删掉主socket，再删子socket时，由于CClient析构了，存子socket的链表就没了，就没法删子socket了。
//// 如果删主socket前，先把子socket全删了。这样的话，子socket OnClose()回调时，又要删一次，
//// 此时主socket没了，所以由于存CSocketClient的链表没了，找不到相应的子socket，程序崩溃
//// 所以这里还要搞一个链表，存所有的socket，包括主socket和子socket
//public:
//	CSocketClient*			m_pChildSocketClientListHead;
//	CSocketClient*			m_pChildSocketClientListTail;
//	DWORD					m_dwChildSocketClientNum;
//	CRITICAL_SECTION		m_ChildSocketClientLock;			// 锁
//	
//
//	// 双向链表，新结点插在列表尾部
//	VOID AddNewChildSocketClientToList(CSocketClient *pSocketClient);
//
//	// 删除SocketClient, 返回是否删除成功
//	BOOL DeleteChildSocketClientFromList(CONNID dwConnectId);
//
//	// 内含CSocketClient的析构，所以有令子socket下线的作用
//	VOID DeleteChildSocketClientFromList(CSocketClient *pSocketClient);
//
//	// 内含CSocketClient的析构，所以有令全部子socket下线的作用
//	VOID DeleteAllChildSocketClientFromList();
//
//	// SocketClient是否存在于链表中，存在则返回SocketClient地址，不存在返回nullptr
//	CSocketClient* SearchSocketClient(CONNID dwConnectId);

};





