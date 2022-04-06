#ifndef _SERIALPORT_H_
#define _SERIALPORT_H_
#include <Windows.h>


class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:
	/** 初始化串口函数
	* 
	* @param: UINT portNo 串口编号，默认值为1，即COM1
	* @param: UINT boud 波特率，默认为9600
	* @param: char parity 是否进行奇偶校验
	* @param: UINT databits 数据为的个数
	* @param: UINT stopdatabits 停止位使用格式
	* @param: DWORD dwCommEvents 默认为EV_RXCHAR,即只要发送任意一个字符，则会产生一个事件
	* @return: bool 初始化是否成功
	* @note: 使用其他本类中的函数请先调用此函数进行初始化
	*/
	bool InitPort(UINT portNo = 1 , UINT baud = CBR_115200 , char parity = 'N', UINT databits = 8 , UINT stopdatabits = 1 , DWORD dwCommEvents = EV_RXCHAR);

private:
	/*串口句柄*/
	HANDLE m_hComm;
	/*线程退出标志变量*/
	static bool s_bExit;
	/* 线程句柄*/
	volatile HANDLE m_hListenThread;
	/*同步互斥，临界区保护*/
	CRITICAL_SECTION m_csCommunicationSync;

};








#endif // !_SERIALPORT_H_
