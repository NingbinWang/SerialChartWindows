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
