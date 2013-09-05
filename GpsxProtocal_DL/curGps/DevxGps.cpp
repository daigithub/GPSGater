/********************************************************************
	created:	2011/12/13
	created:	13:12:2011   14:57
	filename: 	E:\WORKSPACE\PROJECT\DEV-GPS\CODES\DLL_DEV_GPS\CURGPS\DevxGps\DevxGps.cpp
	file path:	E:\WORKSPACE\PROJECT\DEV-GPS\CODES\DLL_DEV_GPS\CURGPS\DevxGps
	file base:	DevxGps
	file ext:	cpp
	author:		areslion@qq.com
	
	purpose:	
*********************************************************************/
#include <memory.h>
#include <string.h>
#include "DevxGps.h"

#include "MGTOOls.h"
#define LOG_NAME "DevxGPS"

void Write_Log(char*pLogName,const char *pLogContent)
{
	::WriteLog(pLogName,logLevelError,pLogContent);

}
void Write_Log(const char *pLogContent)
{
	::WriteLog(LOG_NAME,logLevelError,pLogContent);
}

void buf2HexStr_devx(const char *pSrcbuf,int nSrcLen,char *pDestBuf,int nDestBufLen)
{
	int nCanCopyLen = min(nSrcLen,nDestBufLen);
	if(pSrcbuf[0] =='*')
	{
		memcpy(pDestBuf,pSrcbuf,nCanCopyLen);
		return;
	}

	//����Ҫ����˫�ֽڣ�����ռ�ÿռ�Ҫ*2
	nCanCopyLen = min(nSrcLen,nDestBufLen/2);
	//if( nLen> 1024/2)	nLen = 1024;
	char *pPos = pDestBuf;
	for(int i=0;i<nCanCopyLen;i++)
	{
		sprintf(pPos,"%02x",(unsigned char)pSrcbuf[i]);
		pPos+=2;
	}	
}
HANDLE hTh_GetGPS = NULL;
HANDLE hTh_GetDB = NULL;
DWORD  IDThGPS = 0;
DWORD  IDThDB = 0;
DWORD WINAPI threadGetGPSData(LPVOID lpParameter);
DWORD WINAPI threadGetWriteDB(LPVOID lpParameter);


Protocal	Gdev;

unsigned __int64 *g_pi64LastDataTime =0;
unsigned __int64  g_i64MeiligaoCnt=0;
DWORD	*g_dwMapCount=0;

long start(unsigned __int64 &iLastDataTime,DWORD &dwMapCount)
{
	long nret = 0;
	Write_Log("devxGPS_start()");
	g_pi64LastDataTime = &iLastDataTime;
	g_dwMapCount = &dwMapCount;
	hTh_GetGPS = CreateThread(NULL,0,threadGetGPSData,NULL,0,&IDThGPS);
// 	Sleep(30000);//Ϊ�����쳣��ʱ�رմ����ݿ�ȡ���ݿ�ģ��
// 	hTh_GetDB = CreateThread(NULL,0,threadGetWriteDB,NULL,0,&IDThGPS);	
	
	return nret;
}


long stop()
{
	long nret = 0;

	TerminateThread(hTh_GetDB,0);
	TerminateThread(hTh_GetGPS,0);
	
	return nret;
}




#include <time.h>
DWORD WINAPI threadGetGPSData(LPVOID lpParameter)
{
	char *buf = NULL;
	char strFilename[20]="Console";
	int nlen = sizeof(GPSINFO)+20;
	nlen = 50*1024*1024;//��CGPS_Socket.m_pDataList ��С����һ�¡�
	buf = (char *)malloc(nlen);	
	if(!buf) 
	{
		MessageBox(NULL,("threadGetGPSData-����buf�ռ�ʧ��"),("Warning"),0);
		return 0;
	}
	GPSCommand *gpsCommand = new GPSCommand();

	char *pTempBuffer = buf;//new char[1024*3];
	char *pstrCommandLine = pTempBuffer;//0---1024/2
	char *pCommandPara = pTempBuffer + 1024/2;//1024/2---1024
	char *pGpsGateDataBuf = pTempBuffer+1024;//1024---1024*3/2
	char *pCommandXMl  = pTempBuffer +1024+1024/2;//1024*3/2---1024*3
	GPSClass *pGpsClass = NULL;


	while (TRUE)
	{
		ZeroMemory(buf,nlen);
		int nRet = Gdev.startGPS(buf,nlen);

		//��������ȡ��������������
		ZeroMemory(gpsCommand,sizeof(GPSCommand));
		gpsCommand->strCommandLine		=	pstrCommandLine;
		gpsCommand->commandParameters	=	pCommandPara;
		gpsCommand->pstrCommandXMl		=	pCommandXMl;
		Gdev.Process_Command(gpsCommand,pGpsGateDataBuf);

		Sleep(50);
	}
	
	if(buf!=NULL) free(buf);
	return 0;
}


long Protocal::stop()
{
	long nret = 0;
	return nret;
}





Protocal::Protocal()
{
	//m_pGps = NULL;
	memset(&m_gpsInfo,0x00,sizeof(GPSINFO));
	memset(m_strLogName,0x00,N_LEN);
	strcpy(m_strLogName,"aaaaa");
	strcpy(m_strConsole,"Console");
	InitializeCriticalSectionAndSpinCount(&m_mapLock, 0xFA0);
	
// 	m_gpsIPMap.clear();
}

GPSClass * Protocal::getCurGPSDevice(char *buf,GPSINFO&gpsInfo)
{
	GPSClass * gpsCurDevice=NULL;
	gpsCurDevice = GPSClass::getProtocol(buf,&gpsInfo);
	return gpsCurDevice;	
}
/*
bool Protocal::setCurGpsDev( char *buf,GPSINFO &gpsInfo )
{
	char	strTmp[100] = "";
	int		nlen = 0;

	if(buf[0]=='*'||buf[0]=='$'||buf[0]=='X')
	{
		gpsInfo.nDevID += GPS_RUIXING;
		gpsInfo.nDevID += GPSID_TID;
		m_pGps = &m_gpsRuiXing;

		if(buf[0]=='*') nlen = 77;
		else nlen = 32;
		memcpy(strTmp,buf,nlen);
		memcpy(buf,strTmp,nlen);
	}
	else if(0)
	{
		gpsInfo.nDevID += GPS_YOUHAO;
		gpsInfo.nDevID += GPSID_SIM;
		m_pGps = &m_gpsYouHao;
	}
	else return false;
	m_nDataLen = nlen;
	return true;
}
/**/
void buf2HexStr2(const char *pSrcbuf,char *pDestBuf,int nLen)
{
	if(pSrcbuf[0] =='*')
		return;
	if( nLen> 1024/2)	nLen = 1024;
	char *pPos = pDestBuf;
	*pPos = '_';
	pPos++;
	for(int i=0;i<nLen;i++)
	{
		sprintf(pPos,"%02x",(unsigned char)pSrcbuf[i]);
		pPos+=2;
	}	
}
void wlog( char *buf,int nLen,GPSINFO &gpsInfo,BOOL bToGps/*=TRUE*/ )
{
	char strGpsInfo[1024]="";

	if(bToGps)	sprintf(strGpsInfo,"%s-","[Console]��[GPS]");
	else 
		sprintf(strGpsInfo,"%s-","[GPS]��[Console]");

	strcat(strGpsInfo,buf);
	if(!bToGps)
	{
		//��buf ת����16������ʾ
		buf2HexStr2(buf,strGpsInfo+strlen(strGpsInfo),nLen);
	}
	//TODO: NEEDLOG
	//wlog(gpsInfo.COMMADDR,strGpsInfo);
	Write_Log(gpsInfo.COMMADDR,strGpsInfo);
}
long Protocal::getGpsInfo( GPSClass *pCurGPSClass, char *buf,int nbufLen,GPSINFO &gpsInfo )
{
	long nret = 0;
	BOOL bToGps = FALSE;

	if(!strlen(buf)) bToGps = TRUE;
	//nret = m_pGps->getGpsInfo(buf,gpsInfo);
	nret = pCurGPSClass->getGpsInfo(buf,nbufLen,gpsInfo);
	//TODO: NEEDLOG
	//m_pGps->wlog(buf,m_nDataLen,gpsInfo,bToGps);	
	return nret;
}



long Protocal::SynchronGPSData(GPSClass **pCurGPSClass,char *pInData,int nLenInData,GPSINFO &gpsInfo)
{
	long	nret = 0;

	*pCurGPSClass = getCurGPSDevice(pInData,gpsInfo);
	if(*pCurGPSClass==NULL)
		return -1;
	//if(!setCurGpsDev(pInData,gpsInfo))	return 1;//txt do not fitable any protocal
	nret = getGpsInfo(*pCurGPSClass,pInData,nLenInData,gpsInfo);
	
	return nret<0?(nret-1000):nret;
}



//GPSGATEDATA gpsData
long Protocal::readGPS(GPSGATEDATA &gpsData)
{
	long nret = 0;
	
	nret = getGPS(&gpsData);
	return nret;
}

long Protocal::updateHaxiMap(char *strSIM,GPSGATEDATA gpsData)
{
	long nret = 0;
	char strTmp[512]="";
	
	pair<RUIX_MAP::iterator, bool> inserted;
	
	
	EnterCriticalSection(&m_mapLock);
	m_gpsRuiXMap.erase(strSIM);
	inserted = m_gpsRuiXMap.insert(RUIX_MAP::value_type(strSIM,gpsData));
	if(inserted.second==true) sprintf(strTmp,"updateHaxiMap-��ϣ���гɹ�����SIM��Ϣ-%s",strSIM);
	else sprintf(strTmp,"updateHaxiMap-��ϣ���в���SIM��Ϣʧ��-%s",strSIM);
	//TODO: NEEDLOG
	//m_pGps->wlog("Console",strTmp);
	LeaveCriticalSection(&m_mapLock);
	return nret;
}

//BOOL getIPPort(char *strSIM,GPSGATEDATA gpsData);
BOOL Protocal:: getIPPort(char *strSIM,GPSGATEDATA &gpsData)
{
	BOOL bret = FALSE;
	char strTmp[100]="";
	char strFilname[20]="DevxGps.cpp";
	
	RUIX_MAP::iterator findItem;
	
	
	EnterCriticalSection(&m_mapLock);
	//m_gpsRuiXMap.

 	findItem = m_gpsRuiXMap.find(strSIM);
	if(findItem==m_gpsRuiXMap.end())
	{
		sprintf(strTmp,"getIPPort-��IPӳ������û���ҵ�SIM�ĵ�ַ��Ϣ-SIM:%s",strSIM);
		//TODO: NEEDLOG
		//	m_pGps->wlog(m_strConsole,strTmp);
		Write_Log(strTmp);
		bret= FALSE;
	}
	else
	{
		gpsData =  findItem->second;
		sprintf(strTmp,"getIPPort-��IPӳ�������ҵ���Ϣ-%s",strSIM);
		//TODO: NEEDLOG
		//	m_pGps->wlog(m_strConsole,strTmp);
		bret = TRUE;
	}
	LeaveCriticalSection(&m_mapLock);	
	
	return bret;	
}



BOOL Protocal::writeDataBase( GPSINFO gpsInfo )
{
	return writedb(&gpsInfo);
}

//long Protocal::startProcess_GPSData()
//{
//	const int nDataBufMaxSize=1024;
//	char *pDataBuf = new char[nDataBufMaxSize];
//
//	long nret = 0;
//	char strTmp[512]="";
//	char strFilename[20]="";
//	GPSINFO gpsInfo;
//	GPSGATEDATA  gpsData;
//
//	strcpy(strFilename,"Console");
//	memset(&gpsInfo,0x00,sizeof(GPSINFO));
//	memset(&gpsData,0x00,sizeof(GPSGATEDATA));
//	gpsData.pDatabuf = pDataBuf;
//	BOOL bExit=FALSE;
//
//	while(!bExit)
//	{
//		long nLenGpsData = readGPS(gpsData);
//		if(nLenGpsData<1)
//		{
//			returnStar(0,"",strFilename);
//			Sleep(50);
//			continue;
//		}
//
//		sprintf(strTmp,"[GPS]��[Console]-nDataLen:%d",gpsData.nDataLen);
//		m_pGps->wlog(strFilename,gpsData.pDatabuf,gpsData.nDataLen);
//
//		long nRet = 0;
//		nRet = SynchronGPSData(pDataBuf,gpsData.nDataLen,gpsInfo);
//		if(nRet<1)
//		{
//			returnStar(2,"startGPS-����GPS���Ķ��еı��Ĳ����ϱ��ĵĹ淶",strFilename);
//			continue;
//		}
//		nRet = updateHaxiMap(gpsInfo.COMMADDR,gpsData);
//
//		nRet = writeDataBase(gpsInfo);
//		if(nRet <0)
//		{
//			m_pGps->wlog(gpsInfo.COMMADDR,"[Protocal]��[DB]-Fail to write DB");
//			continue;
//		}
//
//		nRet = m_pGps->getResMsg(pDataBuf,gpsInfo);
//		if(nRet >0)
//		{
//			nRet = writeGPSx(gpsData);
//			if(nRet<1)
//			{
//				sprintf(strTmp,"[Console]��[GPS]%s-ʧ��",gpsData.pDatabuf);			
//			}
//			else 
//				sprintf(strTmp,"[Console]��[GPS]-%s",gpsData.pDatabuf);
//			m_pGps->wlog(gpsInfo.COMMADDR,strTmp);
//		}
//		sprintf(strTmp,"starGps-��ǰ���Ĳ���Ҫ��Ӧ--SIM:%s",gpsInfo.COMMADDR);
//		
//
//	}
//	return 0;
//
//}
long Protocal::startGPS(char *buf,int nlen)
{
	long nret = 0;
	GPSGATEDATA  gpsData;	
	memset(&gpsData,0x00,sizeof(GPSGATEDATA));	
	gpsData.pDatabuf = buf;
	gpsData.nDataLen = nlen;
	int ntotalBufLen = 0;

	if((ntotalBufLen = readGPS(gpsData)) <1) 
	{
		return -500;
	//	return returnStar(0,"",strFilename);
	}
	_time64((__time64_t*)g_pi64LastDataTime);
	int iTimeCount=0;
	char *pEndBuf = buf+ntotalBufLen;

	char *pDataBuf =buf;
	do{
		if(pDataBuf>=pEndBuf)
			break;
		if(gpsData.nDataLen<1)
			break;
		int nDataLen =0;
		if(doGpsData(pDataBuf,gpsData,nDataLen,iTimeCount)==-2002)
			break;

		gpsData.nDataLen =gpsData.nDataLen-nDataLen;
		pDataBuf = pDataBuf+nDataLen;

		iTimeCount++;
	}while(1);
	return 1;
}
int Protocal::doGpsData(char *buf,GPSGATEDATA gpsData,int &nDataLen,int iTimeCount)
{
// 	strcpy(buf,"*HQ,6120108162,V1,044541,A,2624.7708,N,10317.3470,E,0.00,354,270312,FFFFFBFF#");
// 	sprintf(strTmp,"[GPS]��[Console]-nDataLen:%d",gpsData.nDataLen);
// 	m_pGps->wlog(strFilename,gpsData.pDatabuf,gpsData.nDataLen);
	//2461203243550703540304132504827200102438156e000123fffffbffff00c62461203243550704240304132504814400102438424e000079fffffbffff00c72461203243550704590304132504805800102438584e000136fffffbffff00c8
	int nret=0;
	GPSINFO gpsInfo;

	char strTmp[1024]="";
	int nLen_StrTmp=1024;
	memset(&gpsInfo,0x00,sizeof(GPSINFO));

	GPSClass *pCurGPSClass=NULL;
	gpsData.pCurGPSClass =NULL;
	nDataLen =0;
	if((nret = SynchronGPSData(&pCurGPSClass,buf,gpsData.nDataLen,gpsInfo) )<0) 
	{
		if(iTimeCount==0)
		{
			Write_Log("startGPS-����GPS���Ķ��еı��Ĳ����ϱ��ĵĹ淶");		
			buf2HexStr_devx(buf,gpsData.nDataLen,strTmp,nLen_StrTmp);
			Write_Log(strTmp);
		}
		return -2002;
	//	return returnStar(2,"startGPS-����GPS���Ķ��еı��Ĳ����ϱ��ĵĹ淶",strFilename);
	}
	(*g_dwMapCount)++;
	//nret ����ʵ��ʹ�����ݳ���
	nDataLen = nret;
/*	{
		sprintf(strTmp,"recvgpsData,commandr=%s,valid=%d,datalen=%d",gpsInfo.COMMADDR,gpsInfo.bValid,gpsData.nDataLen);
		char *pLogName = LOG_NAME;
		if(gpsInfo.COMMADDR[0]!='\0')
			pLogName = gpsInfo.COMMADDR;
		Write_Log(pLogName,strTmp);				
		buf2HexStr_devx(buf,gpsData.nDataLen,strTmp,nLen_StrTmp);
		Write_Log(pLogName,strTmp);
	}
	
	if(!gpsInfo.bValid)
	{
		char *pLogName = LOG_NAME;
		if(gpsInfo.COMMADDR[0]!='\0')
			pLogName = gpsInfo.COMMADDR;
		Write_Log(pLogName,"startGPS-gpsInfo.bValid=false");		
		buf2HexStr_devx(buf,strTmp,gpsData.nDataLen);
		Write_Log(pLogName,strTmp);
	}
	/**/
	gpsData.pCurGPSClass = pCurGPSClass;
	//���ݱ������������������Ĳ���
	if(gpsInfo.nMsgID == MSG_LOGIN)
	{
		/*if(nret)*/ updateHaxiMap(gpsInfo.COMMADDR,gpsData);
	}
	else if(gpsInfo.nMsgID == MSG_TERMINALFEEDBACK)
	{
		sprintf(strTmp,"v4-->%s->%s",gpsInfo.CMDID,gpsInfo.CMDARGUS);
		//TODO: NEEDLOG
		//	m_pGps->wlog(gpsInfo.COMMADDR,strTmp);
		Write_Log(gpsInfo.COMMADDR,strTmp);
	}
	
	{
		//normal����

		int nRet = writeDataBase(gpsInfo);
		if(nRet<1) 
		{
			sprintf(strTmp,"[Protocal]��[DB]-Fail to write DB.ret=%d,commandr=%s",nRet,gpsInfo.COMMADDR);
			//TODO: NEEDLOG
			//		m_pGps->wlog(gpsInfo.COMMADDR,strTmp);	

			Write_Log(strTmp);
			Write_Log(gpsInfo.COMMADDR,strTmp);		
			buf2HexStr_devx(buf,gpsData.nDataLen,strTmp,nLen_StrTmp);
			Write_Log(gpsInfo.COMMADDR,strTmp);
		}
		//else m_pGps->wlog(gpsInfo.COMMADDR,"[Protocal]��[DB]-success to write DB");

		int nLen = pCurGPSClass->getResMsg(buf,gpsInfo);
		if(nLen>0)
		{
			gpsData.nDataLen = nLen;
			nRet =writeGPSx(gpsData); 
			if(nRet<1)
			{
				sprintf(strTmp,"ret [Console]��[GPS]%s-ʧ��.%d",gpsData.pDatabuf,nRet);			
			}
			else sprintf(strTmp,"ret [Console]��[GPS]-%s",gpsData.pDatabuf);			
			//TODO: NEEDLOG
			//		m_pGps->wlog(gpsInfo.COMMADDR,strTmp);
			Write_Log(gpsInfo.COMMADDR,strTmp);

			buf2HexStr_devx(buf,gpsData.nDataLen,strTmp,nLen_StrTmp);
			Write_Log(gpsInfo.COMMADDR,strTmp);

			buf2HexStr_devx(gpsData.pDatabuf,gpsData.nDataLen,strTmp,nLen_StrTmp);
			Write_Log(gpsInfo.COMMADDR,strTmp);


		}
		else sprintf(strTmp,"starGps-��ǰ���Ĳ���Ҫ��Ӧ--SIM:%s",gpsInfo.COMMADDR);

	}
	return nret;	
}

long Protocal::start()
{
	return 0;
}

long Protocal::Process_Command(GPSCommand *gpsCommand,char *pGpsDataBuf)
{
	///��GPSGateAdpter��ȡ����
	//�����xml��ʽ���ŵ�GPS�豸�н�����������buffer
	//��buffer ͨ��socket ���͸� ��Ӧ��gps
	
	int nRet = getCmd(gpsCommand);
	if(nRet < 1)
		return nRet;
	char strTmp[100];
	do{
		if(gpsCommand->commandType < cmdType_ToService)
		{
			GPSGATEDATA gpsData;

			//ͨ��deviceID ��ȡ �豸��ǰ�� socket��Ϣ
			int iCnt = 0;
			BOOL bGetIPPort  = false;
			while(!bGetIPPort)
			{
				bGetIPPort = getIPPort(gpsCommand->strDevID,gpsData);
				if(iCnt++ > 5)
					break;
			}
			iCnt=0;
			while(!bGetIPPort)
			{
				bGetIPPort = getIPPort(gpsCommand->strSim,gpsData);
				if(iCnt++ > 5)
					break;
			}
			if(!bGetIPPort)
			{
				//��ʱ��û�л�ȡ����Ӧsim ��ip��port��ֱ�Ӷ���
				//�����������������ȥ���´���ʹ��
				nRet = -2;
				break;
			}
			gpsData.pDatabuf = pGpsDataBuf;
			
			if(gpsCommand->nLenCommandLine <1)			
			{
				//�Ӷ�Ӧgps �豸�� ��������
				GPSClass *pCurGPSClass=(GPSClass*)gpsData.pCurGPSClass;
				nRet = pCurGPSClass->getConsole2GPSData(gpsCommand->pstrCommandXMl,gpsCommand);
				if(nRet<1)
				{
					nRet-=1000;
					break;
				}
				//gpsCommand->pstrCommandXMl;
				//gpsCommand->nLenCommandXML = nRet;
			}
			{
				memcpy(gpsData.pDatabuf,gpsCommand->strCommandLine,gpsCommand->nLenCommandLine);
				gpsData.nDataLen = gpsCommand->nLenCommandLine;
			}

			if(gpsCommand->commandType <cmdType_ToService)
			{
				nRet = writeGPSx(gpsData);	
				if(nRet<1)
				{
					sprintf(strTmp,"[Console]��[GPS]%s-ʧ��",gpsData.pDatabuf);			
				}
				else 
					sprintf(strTmp,"[Console]��[GPS]-%s",gpsData.pDatabuf);
			}
		}
		// ������Ҫ���յ������� �ŵ����ݿ��У����Ի���Ҫ����WriteCommand
		//�������Ƿ���GPS����������
		nRet= WriteCommand(gpsCommand);

	}while(0);

 	char strLog[100];
 	sprintf(strLog,"cmd:%d,ret:%d--",gpsCommand->commandType,nRet);
	//TODO: NEEDLOG
	//	m_pGps->wlog(m_strConsole,strLog);
	Write_Log(strLog);

	return nRet;
}
//GPSGATEDATA gpsData
/*
*	���͸�gps �����õ��Ľṹ����Ա
	pGpsGateData->curSocketInfo.tcpSocketHandle;//��map���Ѿ���ֵ
	pGpsGateData->oldSocketInfo.tcpSocketHandle;//��map���Ѿ���ֵ
	pGpsGateData->pDatabuf,//��Ҫ��������
	pGpsGateData->nDataLen//��Ҫ��������
*/
long Protocal::writeGPSx(GPSGATEDATA gpsData)
{
	long nret = 0;
	
	//	nret = writeGPS(buf,addr,port);
	nret = writeGPS(&gpsData);
	
	return nret;
}
