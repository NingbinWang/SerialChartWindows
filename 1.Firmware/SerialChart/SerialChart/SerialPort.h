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
	/*串口初始化函数
	* @param: UINT portNo
	* @param: const LPDCB& plDCB
	* @return: bool 初始化是否成功
	*/
	bool InitPort(UINT portNo, const LPDCB& plDCB);
	/*开启监听线程
	* @return: bool
	*/
	bool OpenListenThread();
	/*关闭监听线程
	* @return bool 操作是否成功
	*/
	bool CloseListenThread();
	/*向串口写数据
	* @param: unsigned char * pData 指向需要写入串口的数据缓存区
	* @param：unsigned int length 需要写入的数据长度
	* @return: bool 操作是否成功
	*/
	bool WriteData(unsigned char* pData, unsigned int length);
	/*获取串口缓存区
	* @reture: 获取数据的长度
	*
	*/
	UINT GetBytesInCOM();
	/*读取串口接收缓存中一个字节的数据
	* @param: char & cRecved
	* @return: bool
	*/
	bool ReadChar(char& cRecved);
private:
	/*打开串口
	* @param: UINT portNo 串口设备号
	* @reture: bool 打开是否正常
	*/
	bool OpenPort(UINT portNo);
	/* 关闭串口
	* 
	*/
	void ClosePort();
	/* 串口监听线程
	* @param: void * pParam 线程参数
	* @return：UINT WINAPI 线程返回值
	*/
	static UINT WINAPI ListenThread(void* pParam);
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
