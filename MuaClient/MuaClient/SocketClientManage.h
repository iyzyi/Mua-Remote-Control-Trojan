#pragma once


#include "pch.h"
#include "SocketClient.h"

class CSocketClient;

// 管理所有客户端
class CSocketClientManage {
public:
	CSocketClientManage();
	~CSocketClientManage();


	// 双向链表
	VOID AddNewSocketClientToList(CSocketClient *pSocketClient);

	// 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
	BOOL DeleteSocketClientFromList(CONNID dwConnectId);
	VOID DeleteSocketClientFromList(CSocketClient *pSocketClient);

	VOID DeleteAllSocketClientFromList();

	// Client是否存在于链表中，存在则返回Client地址，不存在返回NULL
	CSocketClient* SearchSocketClient(CONNID dwConnectId);


public:

	// Client链表的第一个结点，这个结点的所有数据都是空的，
	// 仅用来索引链表的头部，只有m_pNextClient这个数据是非空的。
	// m_pClientListHead->m_pNextClient存放真正的第一个Client的地址
	CSocketClient *m_pSocketClientListHead;

	// 该结点指向链表最后一个结点，这个结点是有数据的结点
	//（除非链表为空，此时m_pClientListHead=m_pClientListTail）
	CSocketClient *m_pSocketClientListTail;

private:
	CRITICAL_SECTION m_Lock;					// 链表操作的锁

	DWORD m_dwClientNum;
	DWORD m_dwMainSocketClientNum;				// 必然只有一个主socket
	DWORD m_dwChildSocketClientNum;
};