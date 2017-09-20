/**
 * @file	JsClientSocket.h
 * @class	JsClientSocket
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/24
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#ifndef __JSCPPUTILS_JSCLIENTSOCKET_H__
#define __JSCPPUTILS_JSCLIENTSOCKET_H__

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include "Common.h"

#include <stdlib.h>

#if defined(JSCUTILS_OS_LINUX)
#error "NOT SUPPORTED LINUX, yet..."

#include <unistd.h>
#include <errno.h>
#elif defined(JSCUTILS_OS_WINDOWS)
#include <WinSock2.h>
#include <mswsock.h>
#include <Windows.h>
#include <mstcpip.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "JsThread.h"
#include "SmartPointer.h"

#include <string>

#ifndef JSCUTILS_SOCKET_T
#if defined(JSCUTILS_OS_LINUX)
#define JSCUTILS_SOCKET_T int
#elif defined(JSCUTILS_OS_WINDOWS)
#define JSCUTILS_SOCKET_T SOCKET
#endif
#endif

#ifndef WIN32
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef closesocket
#define closesocket(sock) close(sock)
#endif
#endif

namespace JsCPPUtils
{
	class JsClientSocket
	{
	public:	
		typedef int(*StartWorkerPostHandler_t)(JsClientSocket *psockctx, int threadidx, void **out_pthreaduserctx);
		typedef void(*StopWorkerHandler_t)(JsClientSocket *psockctx, int threadidx, void *pthreaduserctx);

		typedef int(*Client_ConnectedHandler_t)(JsClientSocket *psockctx, void *pthreaduserctx, JSCUTILS_SOCKET_T clientsock);
		typedef int(*Client_RecvHandler_t)(JsClientSocket *psockctx, void *pthreaduserctx, int recv_len, char *recv_pbuf);
		typedef void(*Client_DisconnectedHandler_t)(JsClientSocket *psockctx, int code);
		typedef void(*Client_SentHandler_t)(JsClientSocket *psockctx, int code);

		enum SOCKSTATE
		{
			SOCKSTATE_NOTINITED = -1,
			SOCKSTATE_CLOSED = 0,
			SOCKSTATE_CONNECTREQ,
			SOCKSTATE_CONNECTING,
			SOCKSTATE_CONNECTING_SSL,
			SOCKSTATE_CONNECTED,
			SOCKSTATE_DISCONNECTREQ,
		};

	private:
		class WorkerThreadInternalContext {
		public:
			JsClientSocket *psockctx;
			
			bool inited_userhandler;
			int threadidx;
			void *pthreaduserctx;
			
			char *precvbuf;
			
#if defined(JSCUTILS_OS_WINDOWS)
		HANDLE async_hEvents[2];
#endif

			WorkerThreadInternalContext(JsClientSocket *_psockctx, int _threadidx, void *_pthreaduserctx)
				: psockctx(_psockctx)
				, inited_userhandler(false)
				, threadidx(_threadidx)
				, pthreaduserctx(_pthreaduserctx)
				, precvbuf(NULL)
			{	
#if defined(JSCUTILS_OS_WINDOWS)
			async_hEvents[0] = INVALID_HANDLE_VALUE;
			async_hEvents[1] = INVALID_HANDLE_VALUE;
#endif
			}
		};

		class WorkerThreadMessage {
		public:
			enum COMMAND {
				CMD_UNDEFINED = 0,
				CMD_CONNECT = 1,
				CMD_DATASEND
			};

			COMMAND cmd;
			int retval;

			WorkerThreadMessage() :
				cmd(CMD_UNDEFINED),
				retval(0)
			{}
				
			WorkerThreadMessage(COMMAND _cmd) :
				cmd(_cmd),
				retval(0)
			{}
		};
		
		class WorkerThreadMessage_Connect : public WorkerThreadMessage
		{
		public:
			struct sockaddr_storage local_sockaddr;
			size_t local_sockaddrlen;
			struct sockaddr_storage remote_sockaddr;
			size_t remote_sockaddrlen;
			bool local_bUse;
			bool remote_bUseHostname;
			std::basic_string<JSCUTILS_TYPE_DEFCHAR> remote_strHostname;
			uint16_t remote_port;
			
			WorkerThreadMessage_Connect(
				const struct sockaddr *_local_sockaddr,
				size_t _local_sockaddrlen,
				const struct sockaddr *_remote_sockaddr,
				size_t _remote_sockaddrlen,
				const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& _remote_strHostname,
				uint16_t _remote_port
				) : WorkerThreadMessage(WorkerThreadMessage::CMD_CONNECT)
			{
				memset(&local_sockaddr, 0, sizeof(local_sockaddr));
				memset(&remote_sockaddr, 0, sizeof(remote_sockaddr));
				local_sockaddrlen = sizeof(local_sockaddr);
				remote_sockaddrlen = sizeof(remote_sockaddr);
				remote_bUseHostname = false;

				if((_local_sockaddr != NULL) && (_local_sockaddrlen > 0))
				{
					memcpy(&local_sockaddr, _local_sockaddr, _local_sockaddrlen);
					local_sockaddrlen = _local_sockaddrlen;
					local_bUse = true;
				}else
					local_bUse = false;
				if((_remote_sockaddr != NULL) && (_remote_sockaddrlen > 0))
				{
					memcpy(&remote_sockaddr, _remote_sockaddr, _remote_sockaddrlen);
					remote_sockaddrlen = _remote_sockaddrlen;
				}
				remote_strHostname = _remote_strHostname;
				remote_port = _remote_port;
				if(_remote_strHostname.length() > 0)
					remote_bUseHostname = true;
			}
			WorkerThreadMessage_Connect(
				const struct sockaddr *_local_sockaddr,
				size_t _local_sockaddrlen,
				const struct sockaddr *_remote_sockaddr,
				size_t _remote_sockaddrlen,
				const JSCUTILS_TYPE_DEFCHAR* _remote_cszHostname,
				uint16_t _remote_port
				) : WorkerThreadMessage(WorkerThreadMessage::CMD_CONNECT)
			{
				memset(&local_sockaddr, 0, sizeof(local_sockaddr));
				memset(&remote_sockaddr, 0, sizeof(remote_sockaddr));
				local_sockaddrlen = sizeof(local_sockaddr);
				remote_sockaddrlen = sizeof(remote_sockaddr);
				remote_bUseHostname = false;

				if((_local_sockaddr != NULL) && (_local_sockaddrlen > 0))
				{
					memcpy(&local_sockaddr, _local_sockaddr, _local_sockaddrlen);
					local_sockaddrlen = _local_sockaddrlen;
					local_bUse = true;
				}else
					local_bUse = false;
				if((_remote_sockaddr != NULL) && (_remote_sockaddrlen > 0))
				{
					memcpy(&remote_sockaddr, _remote_sockaddr, _remote_sockaddrlen);
					remote_sockaddrlen = _remote_sockaddrlen;
				}
				remote_port = _remote_port;
				if(_remote_cszHostname != NULL)
				{
					remote_strHostname = _remote_cszHostname;
					remote_bUseHostname = true;
				}
			}
		};

		class WorkerThreadMessage_Send : public WorkerThreadMessage
		{
		public:
			char *data_pbuf;
			int data_size;
			long send_timeout;

			WorkerThreadMessage_Send(const char *_data_pbuf, int _data_size, long timeoutms)
				 : WorkerThreadMessage(WorkerThreadMessage::CMD_DATASEND)
			{
				data_pbuf = (char*)::malloc(_data_size);
				::memcpy(data_pbuf, _data_pbuf, _data_size);
				data_size = _data_size;
				send_timeout = timeoutms;
			}

			~WorkerThreadMessage_Send()
			{
				if(data_pbuf != NULL)
				{
					::free(data_pbuf);
					data_pbuf = NULL;
				}
			}
		};

		class WorkerThreadMessage_Recv : public WorkerThreadMessage
		{
		public:
			char *data_pbuf;
			int data_size;
			long recv_timeout;
			bool readfixedsize;
			int recv_readsize;

			WorkerThreadMessage_Recv(char *_data_pbuf, int _data_size, bool _readfixedsize, long timeoutms)
				 : WorkerThreadMessage(WorkerThreadMessage::CMD_DATASEND)
			{
				data_pbuf = _data_pbuf;
				data_size = _data_size;
				readfixedsize = _readfixedsize;
				recv_timeout = timeoutms;
				recv_readsize = 0;
			}
		};

		int m_sock_domain;
		int m_sock_proto;
		int m_sock_type;
		
		bool m_bUseSSL;
		int m_sslstate;

		JSCUTILS_SOCKET_T m_sock;

#if defined(JSCUTILS_OS_WINDOWS)
	LPFN_CONNECTEX m_win_fnConnectEx;
#endif

#ifdef USE_OPENSSL
		SSL_CTX *m_pSSLCtx;
		SSL *m_sock_pSSL;
#endif

		long m_conf_recvdatabufsize;

		JsCPPUtils::SmartPointer<JsCPPUtils::JsThread::ThreadContext> m_worker_thread;
		JsCPPUtils::JsThread::MessageHandler<WorkerThreadMessage> m_worker_msg;
		JSTHREAD_THREADID_TYPE m_worker_tid;

		void *m_userptr;

		JsCPPUtils::AtomicNum<int> m_sock_state;

		bool m_conf_bautoreconnect;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> m_autoreconn_spmsg;

		StartWorkerPostHandler_t m_startworkerposthandler;
		StopWorkerHandler_t      m_stopworkerhandler;

		Client_ConnectedHandler_t		m_connectedhandler;
		Client_RecvHandler_t			m_recvhandler;
		Client_DisconnectedHandler_t	m_disconnectedhandler;

		static void workerThreadProc_CleanUp(void *param);
		static int workerThreadProc(JsCPPUtils::JsThread::ThreadContext *pThreadCtx, int threadindex, void *threadparam);

		int _newSocket();
		int _closeSocket(int code = 0);
		
		int _worker_send(JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg);
		int _worker_recv(JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg);

	public:
		JsClientSocket(void *userptr = NULL);
		~JsClientSocket();
		int init(
			int sock_domain,
			int sock_type,
			int sock_proto,
			bool bUseSSL,
#ifdef USE_OPENSSL 
			const SSL_METHOD *ssl_method,
#else
			void *ssl_method,
#endif
			long recvdatabufsize,
			StartWorkerPostHandler_t startworkerposthandler,
			StopWorkerHandler_t stopworkerhandler,
			Client_ConnectedHandler_t connectedhandler,
			Client_RecvHandler_t recvhandler,
			Client_DisconnectedHandler_t disconnectedhandler);
		int sslLoadCertificates(const char* szCertFile, const char* szKeyFile);
		int beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const struct sockaddr *server_psockaddr, int server_sockaddrlen, bool bAutoReconnect = false, long timeoutms = 0);
		int beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& report_strHostname, uint16_t remote_port, bool bAutoReconnect = false, long timeoutms = 0);
		int beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const JSCUTILS_TYPE_DEFCHAR* report_cszHostname, uint16_t remote_port, bool bAutoReconnect = false, long timeoutms = 0);
		int beginDisconnect();

		void setUserPtr(void *userptr);
		void *getUserPtr();
		bool isUseSSL();

		JSCUTILS_SOCKET_T getSocket();
#ifdef USE_OPENSSL 
		SSL_CTX *getSSL_CTX();
#endif
			
		int send(char *pdata, int size, int timeoutms = -1, Client_SentHandler_t senthandler = NULL);
		int recv(char *pdata, int size, int *preadbytes, bool readfixedsize = false, int timeoutms = -1);
	};
}

#endif /* __JSCPPUTILS_JSCLIENTSOCKET_H__ */
