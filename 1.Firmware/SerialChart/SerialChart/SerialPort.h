#ifndef _SERIALPORT_H_
#define _SERIALPORT_H_
#include <Windows.h>


class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:
	/** ��ʼ�����ں���
	* 
	* @param: UINT portNo ���ڱ�ţ�Ĭ��ֵΪ1����COM1
	* @param: UINT boud �����ʣ�Ĭ��Ϊ9600
	* @param: char parity �Ƿ������żУ��
	* @param: UINT databits ����Ϊ�ĸ���
	* @param: UINT stopdatabits ֹͣλʹ�ø�ʽ
	* @param: DWORD dwCommEvents Ĭ��ΪEV_RXCHAR,��ֻҪ��������һ���ַ���������һ���¼�
	* @return: bool ��ʼ���Ƿ�ɹ�
	* @note: ʹ�����������еĺ������ȵ��ô˺������г�ʼ��
	*/
	bool InitPort(UINT portNo = 1 , UINT baud = CBR_115200 , char parity = 'N', UINT databits = 8 , UINT stopdatabits = 1 , DWORD dwCommEvents = EV_RXCHAR);
	/*���ڳ�ʼ������
	* @param: UINT portNo
	* @param: const LPDCB& plDCB
	* @return: bool ��ʼ���Ƿ�ɹ�
	*/
	bool InitPort(UINT portNo, const LPDCB& plDCB);
	/*���������߳�
	* @return: bool
	*/
	bool OpenListenThread();
	/*�رռ����߳�
	* @return bool �����Ƿ�ɹ�
	*/
	bool CloseListenThread();
	/*�򴮿�д����
	* @param: unsigned char * pData ָ����Ҫд�봮�ڵ����ݻ�����
	* @param��unsigned int length ��Ҫд������ݳ���
	* @return: bool �����Ƿ�ɹ�
	*/
	bool WriteData(unsigned char* pData, unsigned int length);
	/*��ȡ���ڻ�����
	* @reture: ��ȡ���ݵĳ���
	*
	*/
	UINT GetBytesInCOM();
	/*��ȡ���ڽ��ջ�����һ���ֽڵ�����
	* @param: char & cRecved
	* @return: bool
	*/
	bool ReadChar(char& cRecved);
private:
	/*�򿪴���
	* @param: UINT portNo �����豸��
	* @reture: bool ���Ƿ�����
	*/
	bool OpenPort(UINT portNo);
	/* �رմ���
	* 
	*/
	void ClosePort();
	/* ���ڼ����߳�
	* @param: void * pParam �̲߳���
	* @return��UINT WINAPI �̷߳���ֵ
	*/
	static UINT WINAPI ListenThread(void* pParam);
private:
	/*���ھ��*/
	HANDLE m_hComm;
	/*�߳��˳���־����*/
	static bool s_bExit;
	/* �߳̾��*/
	volatile HANDLE m_hListenThread;
	/*ͬ�����⣬�ٽ�������*/
	CRITICAL_SECTION m_csCommunicationSync;

};








#endif // !_SERIALPORT_H_
