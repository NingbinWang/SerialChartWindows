#include <process.h>
#include <iostream>
#include "SerialPort.h"

bool CSerialPort::s_bExit = false;

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

	return TRUE;

}