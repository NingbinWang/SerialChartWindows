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

	//临时变量，将制定的参数转化为字符串形式，以构建DCB结构

	return TRUE;

}