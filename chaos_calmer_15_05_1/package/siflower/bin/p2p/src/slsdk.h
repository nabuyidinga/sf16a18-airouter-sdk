/** \file slsdk.h
 * \brief Sunlogin SDK api define
 * \author Oray
 */

#ifndef __ORAY_SLSDK_H__
#define __ORAY_SLSDK_H__


/** \brief SLAPI */
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define SLAPI __stdcall
#include <windows.h>
#else
#define SLAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief SLAPI�汾��
 */
#define SLAPI_VERSION 1

/**
 * \brief �����ƶ˻���
 */
typedef unsigned long SLCLIENT;

/**
 * \brief ���ƶ˻���
 */
typedef unsigned long SLREMOTE;

/**
 * \brief �����ƶ˻Ự
 */
typedef unsigned int SLSESSION;

/** \brief 64 and 32 integer size type definde
 *
 */
#ifndef _WIN32
typedef unsigned long long SLUINT64;
typedef long long SLINT64;
#else
typedef unsigned __int64 SLUINT64;
typedef __int64 SLINT64;
#endif
typedef unsigned int SLUINT32;
typedef int SLINT32;


/**
 * \brief ��Ч�����ƶ˻���
 */
#define SLCLIENT_INVAILD 0

/**
 * \brief ��Ч���ƶ˻���
 */
#define SLREMOTE_INVAILD 0

/**
 * \brief ��Ч�Ự
 */
#define SLSESSION_INVAILD (-1)

/** \brief Error code
 *
 */
enum SLERRCODE {
  //�ɹ�
  SLERRCODE_SUCCESSED = 0, 

  //�ڲ�����
  SLERRCODE_INNER = 1, 

  //δ��ʼ��
  SLERRCODE_UNINITIALIZED = 2, 

  //��������
  SLERRCODE_ARGS = 3,

  //��֧��
  SLERRCODE_NOTSUPPORT = 4,

  //��������ʧ��
  SLERRCODE_CONNECT_FAILED = 5, 

  //�������ӳ�ʱ
  SLERRCODE_CONNECT_TIMEOUT = 6,

  //�Ự������
  SLERRCODE_SESSION_NOTEXIST = 7,

  //�Ự���
  SLERRCODE_SESSION_OVERFLOW = 8,

  //�Ự���ʹ���
  SLERRCODE_SESSION_WRONGTYPE = 9,

  //OPENID����
  SLERRCODE_EXPIRED = 10,
};

/** \brief Session's option
 *
 */
enum ESLSessionOpt {
  eSLSessionOpt_window = 1,					/*!< Window container */
  eSLSessionOpt_callback = 2,				/*!< Callback */
  eSLSessionOpt_deviceSource = 3,			/*!< Device's source */
  eSLSessionOpt_connected = 4,				/*!< Connect status */
  eSLSessionOpt_desktopctrl_listener = 5,	/*!< Desktop control listener */
  eSLSessionOpt_ipport = 6,					/*!< Port forward ip and port */
  eSLSessionOpt_savepath = 7, /*!< File transfer save path */
};

/** \brief Session's event
 *
 */
enum ESLSessionEvent {
  eSLSessionEvent_OnConnected = 1, 		  /*!< Session connected event */
  eSLSessionEvent_OnDisconnected = 2, 	/*!< Session disconnected event */
  eSLSessionEvent_OnDisplayChanged = 3,	/*!< Display resolution is changed */
  eSLSessionEvent_OnNewFiletrans = 4,   /*!< Recv a new file transfer item */
};

/** \brief Session callback
 *
 * \param session - Id of the session
 * \param evt - Type of event
 * \param sdata - String format data
 * \param custom - User data bind to the callback
 */
typedef void ( SLAPI *SLSESSION_CALLBACK )( SLSESSION session, ESLSessionEvent evt, const char* sdata, unsigned long custom );

/**
 * \brief �Ự�ص�����
 */
typedef struct tagSLSESSION_CALLBACK_PROP {
	SLSESSION_CALLBACK pfnCallback;	//!< �ص�����
	unsigned long nCustom;			//!< �Զ�������
} SLSESSION_CALLBACK_PROP;

/** \brief �Ự����
 *
 */
enum ESLSessionType {
	eSLSessionType_Desktop,		/*!< Remote Desktop session */
	eSLSessionType_File,		/*!< Remote File session */
	eSLSessionType_Cmd,			/*!< Remote CMD session */
	eSLSessionType_Sound,		/*!< Remote sound session */
	eSLSessionType_DataTrans,	/*!< Data transfer session */
	eSLSessionType_DesktopView,	/*!< Remote desktop view mode session */
	eSLSessionType_Port,		/*!< Port forward session */
  eSLSessionType_FileTrans, /*!< File transfer session */
};

enum SLProxyType
{
	SLProxy_None,
	SLProxy_HTTP,
	SLProxy_Socks5,
	SLProxy_Socks4,
	SLProxy_IE,
};

/**
* ��������
*/
struct SLPROXY_INFO
{
	char ip[20];
	char port[10];
	char user[20];
	char pwd[20];
	char domain[200];
	SLProxyType type;	//ProxyType		
};


typedef enum __slmode {
  UI = 0,
  SERVICE = 1,
} SLMODE;


/** \brief Initialize SLSDK's enviroment
 *
 * \return �Ƿ��ʼ���ɹ�
 */
bool SLAPI SLInitialize();

/*
 * \brief �˳�SLAPI����
 * \desc ���������������˳�ǰ���ã����ͷ�SLAPI������ʹ�õ���Դ
 * \return �Ƿ��˳��ɹ�
 */
bool SLAPI SLUninitialize();

/*
 * \brief ��ȡ���Ĵ�����
 * \return ����SLERRCODE������
 */
SLERRCODE SLAPI SLGetLastError();

/*
 * \brief �������Ĵ�����
 * \param errCode ������
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetLastError(SLERRCODE errCode);

/*
 * \brief ��ȡ��������ϸ˵��
 * \return ��ϸ��Ϣ����������벻�����򷵻ء�δ֪����
 */
const char* SLAPI SLGetErrorDesc(SLERRCODE errCode);




/** \brief �����ƶ��¼�
 *
 */
enum SLCLIENT_EVENT
{
	SLCLIENT_EVENT_ONCONNECT = 0,	//!< ���ӳɹ�
	SLCLIENT_EVENT_ONDISCONNECT,	//!< �Ͽ�����
	SLCLIENT_EVENT_ONLOGIN,			//!< ��¼�ɹ�
	SLCLIENT_EVENT_ONLOGINFAIL,		//!< ��¼ʧ��
};

/** \brief Create a new client with ui mode
 *
 * \return if success return a new instance, else SLCLIENT_INVAILD
 */
SLCLIENT SLAPI SLCreateClient(void);

/** \brief Create a new client with service mode or ui mode
 *
 * \param mode - Client's mode {@see SLMODE}
 * \return if success return a new instance, else SLCLIENT_INVAILD
 */
SLCLIENT SLAPI SLCreateClientEx( SLMODE mode );


/*
 * \brief ����һ�������ƶ˻���
 * \param client Ҫ���ٵı����ƶ˻���
 */
bool SLAPI SLDestroyClient( SLCLIENT client );

/*
 * \brief �����ƶ˻ص��¼�
 * \param client �����ƶ˻���
 * \param event �¼�
 * \param custom �û��Զ������
 */
typedef void (SLAPI *SLCLIENT_CALLBACK)(SLCLIENT client, SLCLIENT_EVENT event, unsigned long custom);

/*
 * \brief ���ñ����ƶ��¼��ص�����
 * \param client �����ƶ˻���
 * \param pfnCallback �ص�������ַ
 * \param custom �û��Զ���������ص�ʱ�ڲ�����Ὣ�˲���һ���ص�
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetClientCallback(SLCLIENT client, SLCLIENT_CALLBACK pfnCallback, unsigned long custom);

/*
 * \brief �����ƶ˵�¼������
 * \param client �����ƶ˻���
 * \param pstrOpenID �����ߵ�ID��
 * \param pstrOpenKey ������ID��Ӧ����֤��
 * \return �Ƿ��¼�ɹ�
 */
bool SLAPI SLClientLoginWithOpenID(SLCLIENT client, const char* pstrOpenID, const char* pstrOpenKey);
  
/** \brief Short name for SLClientLoginWithOpenID
 *
 */
bool SLAPI SLLoginWithOpenID(SLCLIENT client, const char* pstrOpenID, const char* pstrOpenKey);

  
/*
 * \brief �����ƶ˵�¼������
 * \param client �����ƶ˻���
 * \param szAddr ��������ַ
 * \param szLic lincense
 * \return �Ƿ��¼�ɹ�
 */
bool SLAPI SLClientLoginWithLicense(SLCLIENT client, const char* szAddr, const char* szLic);

/*
 * \brief �����ƶ��Ƿ��¼��
 * \param client �����ƶ˻���
 */
bool SLAPI SLClientIsOnLoginned(SLCLIENT client);
/*
 * \brief �ڱ����ƶ˻����д���һ���Ự
 * \param client �����ƶ˻���
 * \return �Ựֵ���������ʧ�ܣ��򷵻�SLSESSION_INVAILD
 */
SLSESSION SLAPI SLCreateClientSession(SLCLIENT client, ESLSessionType eType);

/*
 * \brief ����һ���Ự
 * \param client �����ƶ˻���
 * \param session �Ự
 * \return �Ƿ����ٳɹ�
 */
bool SLAPI SLDestroyClientSession(SLCLIENT client, SLSESSION session);

/*
* \brief �򿪱��ض���־
* \param client �����ƶ˻���
* \param path ·��
* \return �Ƿ����óɹ�
*/
bool SLAPI SLOpenClientLog(SLCLIENT client, const char* path);

/*
* \brief ���ô���
* \param client �����ƶ˻���
* \param proxy ��������
* \return �Ƿ����óɹ�
*/
bool SLAPI SLSetClientProxy(SLCLIENT client, const SLPROXY_INFO& proxy);

/*
 * \brief ö�ٱ��ض˵�ǰ�ж��ٸ��Ự
 * \param client �����ƶ˻���
 * \param pSessionArray �Ự���飨���������
 * \param nArraySize ���鳤��
 * \return һ���ж��ٸ��Ự
 */
unsigned int SLAPI SLEnumClientSession(SLCLIENT client, SLSESSION* pSessionArray, unsigned int nArraySize);

/*
 * \brief ��ȡ�����ƶ����ӵ�ַ
 * \remark �����ڵ�¼�ɹ����ٻ�ȡ���ܻ�ȡ��ȷ��ֵ�������ص��¼�SLCLIENT_EVENT_ONLOGIN��������á�ͨ����ֵ�����ƶ˲���ʹ�øûỰ�ķ���
 * \return ��ַ
 */
const char* SLAPI SLGetClientAddress(SLCLIENT client);

/*
 * \brief ��ȡ�����ƶ�ĳ���Ự��ֵ
 * \remark ͨ����ֵ�����ƶ˲���ʹ�øûỰ�ķ���
 * \return �Ựֵ������Ự�������򷵻�NULL
 */
const char* SLAPI SLGetClientSessionName(SLCLIENT client, SLSESSION session);

/*
 * \brief �����ƶ�ĳ���Ự��������
 * \param client �����ƶ˻���
 * \param session �Ự
 * \param lpData ���͵�����
 * \param nLen ���͵����ݳ���
 * \return ���͵��ֽ���
 * \remark Ŀǰֻ������DataTrans���͵ĻỰ
 */
unsigned long SLAPI SLClientSessionSendData(SLCLIENT client, SLSESSION session, const char* lpData, unsigned long nLen);

/*
 * \brief �����ƶ�ĳ���Ự��������
 * \param client �����ƶ˻���
 * \param session �Ự
 * \param lpData �������ݵĻ�����
 * \param nLen ׼�����յ����ݳ���
 * \return ʵ�ʽ��յ����ֽ���
 * \remark Ŀǰֻ������DataTrans���͵ĻỰ
 */
unsigned long SLAPI SLClientSessionRecvData(SLCLIENT client, SLSESSION session, char* lpData, unsigned long nLen);

/*
 * \brief ��ȡ�����ƶ�ĳ���Ựĳ������ֵ
 * \return �Ƿ��ȡ�ɹ�
 */
bool SLAPI SLGetClientSessionOpt(SLCLIENT client, SLSESSION session, ESLSessionOpt eOpt, char* pOptVal, unsigned int nOptLen);

/*
 * \brief ���ñ����ƶ�ĳ���Ựĳ������ֵ
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetClientSessionOpt(SLCLIENT client, SLSESSION session, ESLSessionOpt eOpt, const char* pOptVal, unsigned int nOptLen);

/*
 * \brief ����WEB����
 * \return �Ƿ�ɹ�
 */
bool SLAPI SLStartWebServer(SLCLIENT client, unsigned int nPort=0);

/*
 * \brief �ر�WEB����
 * \return �Ƿ�ɹ�
 */
bool SLAPI SLStopWebServer(SLCLIENT client);

/*
 * \brief web������˷���������true��ʾ�Ѿ������˵�ǰ�¼����ײ㽫�����ٴ���
 * \param client �����ƶ˻���
 * \param data ָ�����ݵ�ָ��
 * \param size ���ݳ���
 */
typedef bool (SLAPI *SLWEB_FILTER)(SLCLIENT client,const void* data,unsigned int size);

/*
 * \brief ����web������˷���
 * \param client �����ƶ˻���
 * \param filter ����ָ��
 */
bool SLAPI SlSetWebServerFilter(SLCLIENT client,SLWEB_FILTER filter);

/*
 * \brief ��web�ͻ��˷�������
 * \param client �����ƶ˻���
 * \param data ָ�����ݵ�ָ��
 * \param size ���ݳ���
 */
bool SLAPI SlWebServerSend( SLCLIENT client,const void* pdata,unsigned int size );

/** \brief Send file to peer
 *
 * \param client - Client
 * \param session - Specified session
 * \param filepath - File to be sent 
 * \param resume - Resume transfer
 *
 * \return transfer id of file.
 */
SLUINT32 SLAPI SLClientSendFile( SLCLIENT client, SLSESSION session, const wchar_t* filepath, bool resume );


/** \brief Kill the file item with fid
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is ok else failed.
 */
bool SLAPI SLClientKillFile( SLCLIENT client, SLSESSION session, SLUINT32 fid );


/** \brief Get name of file item with fid
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return Name of file item
 */
const wchar_t* SLAPI SLClientGetFileName( SLCLIENT client, SLSESSION session, SLUINT32 fid );

/** \brief Get file size  
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return File item's size.
 */
SLUINT64 SLAPI SLClientGetFileSize( SLCLIENT client, SLSESSION session, SLUINT32 fid );


/** \brief Get file transfered
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return File item's transfered size.
 */
SLUINT64 SLAPI SLClientGetFileTransfered(  SLCLIENT client, SLSESSION session, SLUINT32 fid );


/** \brief File state is in transfering or not
 *
 * \param client - Client 
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is transfering else not.
 */
bool SLAPI SLClientFileIsTransfering( SLCLIENT client, SLSESSION session, SLUINT32 fid );


/** \brief File state is done or not
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is done else not.
 */
bool SLAPI SLClientFileIsDone( SLCLIENT client, SLSESSION session, SLUINT32 fid );

/** \brief File state is killed or not
 *
 * \param client - Client
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is killed else not.
 */
bool SLAPI SLClientFileIsKilled( SLCLIENT client, SLSESSION session, SLUINT32 fid );



/************************************************************************/
/* ���ƶ����API                                                        */
/************************************************************************/
/**
 * \brief �����ƶ��¼�
 */
enum SLREMOTE_EVENT
{
  SLREMOTE_EVENT_ONCONNECT = 0, 		//!< ���ӳɹ�
  SLREMOTE_EVENT_ONDISCONNECT, 			//!< �Ͽ�����
  SLREMOTE_EVENT_ONDISCONNECT_FOR_FULL, //!< �Ͽ�����(��Ϊ����������)  
};

/*
 * \brief ����һ�����ƶ˻���
 * \return ���ر����ƶ˻���ֵ���������ʧ���򷵻�SLREMOTE_INVAILD
 */
SLREMOTE SLAPI SLCreateRemote(void);

/*
 * \brief ����һ�����ƶ˻���
 * \param remote ���ƶ˻���
 * \return �Ƿ����ٳɹ�
 */
bool SLAPI SLDestroyRemote(SLREMOTE remote);

/*
* \brief �򿪿��ƶ���־
* \param remote ���ƶ˻���
* \param path ·��
* \return �Ƿ����óɹ�
*/
bool SLAPI SLOpenRemoteLog(SLREMOTE remote, const char* path);

/*
* \brief ���ô���
* \param client �����ƶ˻���
* \param remote ���ƶ˻���
* \return �Ƿ����óɹ�
*/
bool SLAPI SLSetRemoteProxy(SLREMOTE remote, const SLPROXY_INFO& proxy);

/*
 * \brief �����ƶ˻ص��¼�
 * \param remote �����ƶ˻���
 * \param event �¼�
 * \param custom �û��Զ������
 */
typedef void (SLAPI *SLREMOTE_CALLBACK)(SLREMOTE remote, SLSESSION session, SLREMOTE_EVENT event, unsigned long custom);

/*
 * \brief ���������ƶ��¼��ص�����
 * \param remote �����ƶ˻���
 * \param pfnCallback �ص�������ַ
 * \param custom �û��Զ���������ص�ʱ�ڲ�����Ὣ�˲���һ���ص�
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetRemoteCallback(SLREMOTE remote, SLREMOTE_CALLBACK pfnCallback, unsigned long custom);

/*
 * \brief ���������ƶ˻Ự
 * \param remote ���ƶ˻���
 * \param eType �Ự����
 * \param pstrAddress Զ�̱����ƶ˵�ַ
 * \param pstrSession Զ������Ự��
 * \return �Ự
 */
SLSESSION SLAPI SLCreateRemoteSession(SLREMOTE remote, ESLSessionType eType, const char* pstrAddress, const char* pstrSession);

/*
 * \brief ���������ƶ˿ջỰ(������)
 * \param remote ���ƶ˻���
 * \param eType �Ự����
 * \remark ��SLCreateRemoteSession��ͬ���Ǵ���һ���ջỰ�����������ӣ����������ʹ��SLConnectRemoteSession�����ӻỰ
 * \return �Ự
 */
SLSESSION SLAPI SLCreateRemoteEmptySession(SLREMOTE remote, ESLSessionType eType);

/*
 * \brief �������ض˻Ự
 * \param remote ���ƶ˻���
 * \param session �Ự
 * \param pstrAddress Զ�̱����ƶ˵�ַ
 * \param pstrSession Զ������Ự��
 * \return �Ự
 */
bool SLAPI SLConnectRemoteSession(SLREMOTE remote, SLSESSION session, const char* pstrAddress, const char* pstrSession);

/*
 * \brief ����һ���Ự
 * \param remote ���ƶ˻���
 * \param session �Ự
 * \return �Ƿ����ٳɹ�
 */
bool SLAPI SLDestroyRemoteSession(SLREMOTE remote, SLSESSION session);

/*
 * \brief �����ƶ�ĳ���Ự��������
 * \param remote �����ƶ˻���
 * \param session �Ự
 * \param lpData ���͵�����
 * \param nLen ���͵����ݳ���
 * \return ���͵��ֽ���
 * \remark Ŀǰֻ������DataTrans���͵ĻỰ
 */
unsigned long SLAPI SLRemoteSessionSendData(SLREMOTE remote, SLSESSION session, const char* lpData, unsigned long nLen);

/*
 * \brief �����ƶ�ĳ���Ự��������
 * \param remote �����ƶ˻���
 * \param session �Ự
 * \param lpData �������ݵĻ�����
 * \param nLen �������ݻ���������
 * \return ʵ�ʽ��յ����ֽ���
 * \remark Ŀǰֻ������DataTrans���͵ĻỰ
 */
unsigned long SLAPI SLRemoteSessionRecvData(SLREMOTE remote, SLSESSION session, char* lpData, unsigned long nLen);

/*
 * \brief ��ȡ�����ƶ�ĳ���Ựĳ������ֵ
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLGetRemoteSessionOpt(SLREMOTE remote, SLSESSION session, ESLSessionOpt eOpt, char* pOptVal, unsigned int nOptLen);

/*
 * \brief ���������ƶ�ĳ���Ựĳ������ֵ
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetRemoteSessionOpt(SLREMOTE remote, SLSESSION session, ESLSessionOpt eOpt, const char* pOptVal, unsigned int nOptLen);

/*
 * \brief ����Զ�����洰�ڵĴ�С
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetDesktopSessionPos(SLREMOTE remote, SLSESSION session, int x,int y,int width,int height);

/*
 * \brief Show desktop window
 * \return �Ƿ����óɹ�
 */
bool SLAPI SLSetDesktopSessionVisible( SLREMOTE remote, SLSESSION session );

  
/** \brief	Get original desktop size
 * \return	
 */
bool SLAPI SLGetDesktopSessionOriginSize( SLREMOTE remote, SLSESSION session, int* width, int* height );


/** \brief Start desktop record, Only one file in recording.
 * 
 * \param remote - Peer
 * \param session - Specified session
 * \param filepath - Desktop record file 
 *
 * \return true is ok else failed.
 */
bool SLAPI SLRemoteDesktopStartRecord( SLREMOTE remote, SLSESSION session, const char* filepath );


/** \brief Stop desktop record
 * 
 * \param remote - Peer
 * \param session - Specified session
 *
 */
void SLAPI SLRemoteDesktopStopRecord( SLREMOTE remote, SLSESSION session );





/** \brief Send file to peer
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param filepath - File to be sent 
 * \param resume - Resume transfer
 *
 * \return transfer id of file.
 */
SLUINT32 SLAPI SLRemoteSendFile(SLREMOTE remote, SLSESSION session, const wchar_t* filepath, bool resume );


/** \brief Kill the file item with fid
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is ok else failed.
 */
bool SLAPI SLRemoteKillFile( SLREMOTE remote, SLSESSION session, SLUINT32 fid );

/** \brief Get name of file item with fid
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return Name of file item
 */
const wchar_t* SLAPI SLRemoteGetFileName( SLREMOTE client, SLSESSION session, SLUINT32 fid );


/** \brief Get file size  
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return File item's size.
 */
SLUINT64 SLAPI SLRemoteGetFileSize(  SLREMOTE remote, SLSESSION session, SLUINT32 fid );


/** \brief Get file transfered
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return File item's transfered size.
 */
SLUINT64 SLAPI SLRemoteGetFileTransfered(  SLREMOTE remote, SLSESSION session, SLUINT32 fid );


/** \brief File state is in transfering or not
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is transfering else not.
 */
bool SLAPI SLRemoteFileIsTransfering( SLREMOTE remote, SLSESSION session, SLUINT32 fid );


/** \brief File state is done or not
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is done else not.
 */
bool SLAPI SLRemoteFileIsDone( SLREMOTE remote, SLSESSION session, SLUINT32 fid );

/** \brief File state is killed or not
 *
 * \param remote - Peer
 * \param session - Specified session
 * \param fid - Id of file item 
 *
 * \return true is killed else not.
 */
bool SLAPI SLRemoteFileIsKilled( SLREMOTE remote, SLSESSION session, SLUINT32 fid );



#ifdef __cplusplus
}
#endif


#endif //__ORAY_SLSDK_H__
