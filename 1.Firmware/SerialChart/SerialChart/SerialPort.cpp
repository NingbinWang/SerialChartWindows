#include <process.h>
#include <iostream>
#include "SerialPort.h"

bool CSerialPort::s_bExit = false;

const UINT SLEEP_TIME_INTERVAL = 5;

CSerialPort::CSerialPort(void)
	:m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&m_csCommunicationSync);
}

CSerialPort::~CSerialPort(void)
{

}

bool CSerialPort::InitPort(UINT portNo , UINT baud , char parity , UINT databits, UINT stopdatabits, DWORD dwCommEvents) {

	//��ʱ���������ƶ��Ĳ���ת��Ϊ�ַ�����ʽ���Թ���DCB�ṹ
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopdatabits);
	if (!OpenPort(portNo))
	{
		return false;
	}
	//�����ٽ���
	EnterCriticalSection(&m_csCommunicationSync);
	bool bIsSuccess = TRUE;
	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	if (bIsSuccess) {
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}
	DCB dcb;
	if (bIsSuccess) {
		//��ANSI�ַ���ת����UNICODE�ַ���
		DWORD dwNUM = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t* pwText = new wchar_t[dwNUM];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNUM))
		{
			bIsSuccess = TRUE;
		}
		//��ȡ��ǰ���ڵ����ò��������ҹ��촮��DCB����
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		//���� RTS FLOW
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		delete[] pwText;
	}
	if (bIsSuccess)
	{
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
	//���㴮�ڻ�����
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	//�뿪�ٽ�
	LeaveCriticalSection(&m_csCommunicationSync);
	return bIsSuccess == TRUE;

}
bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB) {
	if (!OpenPort(portNo)) {
		return false;
	}
	//�����ٽ���
	EnterCriticalSection(&m_csCommunicationSync);
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}
	//���㴮�ڻ�����
	PurgeComm(m_hComm, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT);
	//�뿪�ٽ�
	LeaveCriticalSection(&m_csCommunicationSync);
	return true;
}

void CSerialPort::ClosePort()
{
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::OpenPort(UINT portNo) {
	/* �����ٽ��*/
	EnterCriticalSection(&m_csCommunicationSync);
	//�Ѵ��ڵı仯ת��Ϊ�豸��
	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);
	//��ָ������
	m_hComm = CreateFileA(szPort,//�豸��
		GENERIC_READ | GENERIC_WRITE,//����ģʽ ͬʱ�ɶ�д
		0,                           //����ģʽ��0������
		NULL,                        //��ȫ�����ã�һ��ΪNULL
		OPEN_EXISTING,               //��ʾ�豸һ��Ҫ����
		0,
		0);
	// ��ʧ�ܾ��ͷ���Դ������
	if (m_hComm == INVALID_HANDLE_VALUE) {
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}
	//�˳��ٽ�
	LeaveCriticalSection(&m_csCommunicationSync);
	return true;
}

bool CSerialPort::OpenListenThread() {
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		return false;//�߳��Ѿ��򿪣�ֱ���˳�
	}
	s_bExit = false;
	UINT threadId;
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread) {
		return false;
	}
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL)) {
		return false;
	}
	return true;
}

bool CSerialPort::CloseListenThread() {
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		s_bExit = true;
		Sleep(10);
		CloseHandle(m_hListenThread);
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;  /** ������ */
	COMSTAT  comstat;   /** COMSTAT�ṹ��,��¼ͨ���豸��״̬��Ϣ */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ����־ */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** ��ȡ�����뻺�����е��ֽ��� */
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** �õ������ָ�� */
	CSerialPort* pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// �߳�ѭ��,��ѯ��ʽ��ȡ��������  
	while (!pSerialPort->s_bExit)
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** ����������뻺������������,����Ϣһ���ٲ�ѯ */
		if (BytesInQue == 0)
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** ��ȡ���뻺�����е����ݲ������ʾ */
		char cRecved = 0x00;
		do
		{
			cRecved = 0x00;
			if (pSerialPort->ReadChar(cRecved) == true)
			{
				std::cout << cRecved;
				continue;
			}
		} while (--BytesInQue);
	}

	return 0;
}

bool CSerialPort::ReadChar(char& cRecved)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �ӻ�������ȡһ���ֽڵ����� */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
	if ((!bResult))
	{
		/** ��ȡ������,���Ը��ݸô�����������ԭ�� */
		DWORD dwError = GetLastError();

		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return (BytesRead == 1);

}

bool CSerialPort::WriteData(unsigned char* pData, unsigned int length)
{
	BOOL   bResult = TRUE;
	DWORD  BytesToSend = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �򻺳���д��ָ���������� */
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}