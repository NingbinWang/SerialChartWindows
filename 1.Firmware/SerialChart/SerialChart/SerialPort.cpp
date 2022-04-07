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

	//临时变量，将制定的参数转化为字符串形式，以构建DCB结构
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopdatabits);
	if (!OpenPort(portNo))
	{
		return false;
	}
	//进入临界区
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
		//将ANSI字符串转换成UNICODE字符串
		DWORD dwNUM = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t* pwText = new wchar_t[dwNUM];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNUM))
		{
			bIsSuccess = TRUE;
		}
		//获取当前串口的配置参数，并且构造串口DCB参数
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		//开启 RTS FLOW
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		delete[] pwText;
	}
	if (bIsSuccess)
	{
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
	//清零串口缓存区
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	//离开临界
	LeaveCriticalSection(&m_csCommunicationSync);
	return bIsSuccess == TRUE;

}
bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB) {
	if (!OpenPort(portNo)) {
		return false;
	}
	//进入临界区
	EnterCriticalSection(&m_csCommunicationSync);
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}
	//清零串口缓存区
	PurgeComm(m_hComm, PURGE_RXCLEAR|PURGE_TXCLEAR|PURGE_RXABORT|PURGE_TXABORT);
	//离开临界
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
	/* 进入临界断*/
	EnterCriticalSection(&m_csCommunicationSync);
	//把串口的变化转化为设备号
	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);
	//打开指定串口
	m_hComm = CreateFileA(szPort,//设备名
		GENERIC_READ | GENERIC_WRITE,//访问模式 同时可读写
		0,                           //共享模式，0不共享
		NULL,                        //安全性设置，一版为NULL
		OPEN_EXISTING,               //表示设备一定要存在
		0,
		0);
	// 打开失败就释放资源，返回
	if (m_hComm == INVALID_HANDLE_VALUE) {
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}
	//退出临界
	LeaveCriticalSection(&m_csCommunicationSync);
	return true;
}

bool CSerialPort::OpenListenThread() {
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		return false;//线程已经打开，直接退出
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
	DWORD dwError = 0;  /** 错误码 */
	COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** 得到本类的指针 */
	CSerialPort* pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// 线程循环,轮询方式读取串口数据  
	while (!pSerialPort->s_bExit)
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** 如果串口输入缓冲区中无数据,则休息一会再查询 */
		if (BytesInQue == 0)
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** 读取输入缓冲区中的数据并输出显示 */
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

	/** 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 从缓冲区读取一个字节的数据 */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
	if ((!bResult))
	{
		/** 获取错误码,可以根据该错误码查出错误原因 */
		DWORD dwError = GetLastError();

		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 离开临界区 */
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

	/** 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 向缓冲区写入指定量的数据 */
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 离开临界区 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}