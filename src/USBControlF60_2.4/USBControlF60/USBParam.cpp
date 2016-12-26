#include "StdAfx.h"
#include "USBParam.h"

CUSBParam::CUSBParam(void)
{
	
	memset(WriteBuffer, 0, WRITEBUF_LEN);
	writebuflen = 0;
	memset(ReadBuffer, 0, READBUFLEN);
	UCHAR *pRead = NULL;
	readbuflen = 0;

	memset(&m_sendcmd, 0, sizeof(SendPackage));
	memset(&m_answercmd, 0, sizeof(ReceivePackage));
}


CUSBParam::~CUSBParam(void)
{
}

//合成发送命令
void CUSBParam::ComposeSendPackage(SendPackage &sendPackage)
{
	 CLock lock(g_Lock);
	m_sendcmd = sendPackage;
	memset(WriteBuffer, 0, sizeof(WriteBuffer));
	WriteBuffer[0] = 0x00;
	WriteBuffer[1] = m_sendcmd.m_head;
	WriteBuffer[2] = m_sendcmd.m_cmd;

	//WriteBuffer[3] = 0x02;
	WriteBuffer[3] = m_sendcmd.m_len;

	memcpy(&(WriteBuffer[4]), m_sendcmd.m_data, m_sendcmd.m_len);

// 	WriteBuffer[4 + m_sendcmd.m_len] = m_sendcmd.m_checksum;
// 	WriteBuffer[4 + m_sendcmd.m_len+1] = m_sendcmd.m_end;
	WriteBuffer[63] = m_sendcmd.m_checksum;
	WriteBuffer[64] = m_sendcmd.m_end;

	writebuflen = 65;//6 + m_sendcmd.m_len;
}

//解析命令
ReceivePackage* CUSBParam::AnalyzeRecvPackage(void)
{
	 CLock lock(g_Lock);
	//格式 ReadBuffer : 0x00(1Byte), head(1Byte), cmd(1Byte), len(1Byte), data(59 Byte), check(1Byte), end(1Byte)
	if (ReadBuffer[1] != CMD_RECVHEADER)
	{
		memset(&m_answercmd, 0, sizeof(m_answercmd));
		m_answercmd.m_cmd = USB_RECEIVE_ERR ;
		return &m_answercmd;
	}

	if (ReadBuffer[64] != CMD_RECVEND)
	{//帧尾校验
		memset(&m_answercmd, 0, sizeof(m_answercmd));
		m_answercmd.m_cmd = USB_RECEIVE_ERR ;
		return &m_answercmd;
	}

	unsigned char checksum = 0;
	for (int i = 0; i < PARAM_DATA_LEN+4; ++i)
	{
		checksum += ReadBuffer[i];
	}	
	if (checksum != ReadBuffer[63])
	{//校验和
		memset(&m_answercmd, 0, sizeof(m_answercmd));
		m_answercmd.m_cmd = USB_RECEIVE_ERR ;
		return &m_answercmd;
	}

	m_answercmd.m_cmd = ReadBuffer[2];
	m_answercmd.m_len = ReadBuffer[3];

	memcpy(m_answercmd.m_data, &ReadBuffer[4], m_answercmd.m_len);

	return &m_answercmd;
}