/**
 * @file	JsClientSocket.cpp
 * @class	JsClientSocket
 * @author	Jichan (development@jc-lab.net / http://ablog.jc-lab.net/category/JsCPPUtils )
 * @date	2017/04/24
 * @brief	TCP Client 소켓을 사용하기 쉽게 만든 클래스 (SSL지원).
 * @warning	아직 Windows만 지원함...
 * @copyright Copyright (C) 2016 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the MIT license.  See the LICENSE file for details.
 */

#include "JsClientSocket.h"

#include <stdio.h>
#include <string.h>

namespace JsCPPUtils
{
	JsClientSocket::JsClientSocket(void *userptr)
	{
		m_userptr = userptr;

		m_sock = INVALID_SOCKET;

		m_conf_bautoreconnect = false;

#ifdef USE_OPENSSL
		m_pSSLCtx = NULL;
		m_sock_pSSL = NULL;
#endif
	}
	
	JsClientSocket::~JsClientSocket()
	{
		_closeSocket();

		
#ifdef USE_OPENSSL
		if(m_pSSLCtx != NULL)
		{
			::SSL_CTX_free(m_pSSLCtx);
			m_pSSLCtx = NULL;
		}
#endif
	}

	int JsClientSocket::init(
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
			Client_DisconnectedHandler_t disconnectedhandler)
	{
		int retval = 0;
		int rc;
		
		int nErr;

		JSCUTILS_SOCKET_T win_tmpsock = INVALID_SOCKET;
		
#ifdef USE_OPENSSL
		if(m_pSSLCtx != NULL)
		{
			::SSL_CTX_free(m_pSSLCtx);
			m_pSSLCtx = NULL;
		}
#else
		if(bUseSSL)
		{
			return 0;
		}
#endif

		_closeSocket();
		m_sock_state = SOCKSTATE_NOTINITED;

		m_sock_domain = sock_domain;
		m_sock_type = sock_type;
		m_sock_proto = sock_proto;
		m_bUseSSL = bUseSSL;

		m_sock = INVALID_SOCKET;
		
		m_conf_bautoreconnect = false;
		m_conf_recvdatabufsize = recvdatabufsize;

		m_startworkerposthandler = startworkerposthandler;
		m_stopworkerhandler = stopworkerhandler;

		m_connectedhandler = connectedhandler;
		m_recvhandler = recvhandler;
		m_disconnectedhandler = disconnectedhandler;

		m_worker_tid = 0;

		do {
#if defined(JSCUTILS_OS_WINDOWS)
			GUID win_connectex_guid = WSAID_CONNECTEX;
			//m_win_fnConnectEx
			DWORD dwBytes;

			win_tmpsock = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, NULL, 0);
			if(win_tmpsock == INVALID_SOCKET)
			{
				nErr = ::WSAGetLastError();
				retval = -nErr;
				break;
			}

			dwBytes = 0;
			rc = ::WSAIoctl(win_tmpsock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                      &win_connectex_guid, sizeof(win_connectex_guid),
                      &m_win_fnConnectEx, sizeof(m_win_fnConnectEx),
                      &dwBytes, NULL, NULL);
			if(rc != 0)
			{
				nErr = ::WSAGetLastError();
				retval = -nErr;
				break;
			}
#endif

#ifdef USE_OPENSSL
			if (m_bUseSSL)
			{
				m_pSSLCtx = ::SSL_CTX_new(ssl_method);
				if (m_pSSLCtx == NULL)
				{
					ERR_print_errors_fp(stderr);
					retval = -1;
					break;
				}
			}
#endif

			retval = _newSocket();
		}while(0);

		if(win_tmpsock != INVALID_SOCKET)
		{
			::closesocket(win_tmpsock);
			win_tmpsock = INVALID_SOCKET;
		}

		if(retval <= 0)
		{
			_closeSocket();
		}else{
			m_sock_state = SOCKSTATE_CLOSED;
			rc = JsCPPUtils::JsThread::start(&m_worker_thread, workerThreadProc, 0, this, &m_worker_tid);
		}

		return retval;
	}

	int JsClientSocket::_newSocket()
	{
		int nErr;
		int retval = 0;
		JSCUTILS_SOCKET_T sock = INVALID_SOCKET;
		
#ifdef USE_OPENSSL
		SSL *pSSL = NULL;
#endif

		do {
#if defined(JSCUTILS_OS_WINDOWS)
			sock = ::WSASocket(m_sock_domain, m_sock_type, m_sock_proto, NULL, NULL, 0);
			if(sock == INVALID_SOCKET)
			{
				nErr = ::WSAGetLastError();
				retval = -nErr;
				break;
			}
#elif defined(JSCUTILS_OS_LINUX)
			sock = ::WSASocket(m_sock_domain, m_sock_type, m_sock_proto, NULL, NULL, 0);
			if(sock == INVALID_SOCKET)
			{
				nErr = errno;
				retval = -nErr;
				break;
			}
#endif
			
#ifdef USE_OPENSSL
			if(m_bUseSSL)
			{
				pSSL = SSL_new(m_pSSLCtx);
				if(pSSL == NULL)
				{
					retval = -1;
					break;
				}
			}
#endif

			retval = 1;
		}while(0);

		if(retval <= 0)
		{
#ifdef USE_OPENSSL
			if(pSSL != NULL)
			{
				SSL_free(pSSL);
				pSSL = NULL;
			}
#endif
			if(sock != INVALID_SOCKET)
			{
				::closesocket(sock);
				sock = INVALID_SOCKET;
			}
		}else{
			_closeSocket();
			m_sock = sock;
#ifdef USE_OPENSSL
			m_sock_pSSL = pSSL;
#endif
		}

		return retval;
	}

	int JsClientSocket::_closeSocket(int code)
	{

		if(m_sock_state >= SOCKSTATE_CONNECTED)
		{
			if(m_disconnectedhandler)
			{
				m_disconnectedhandler(this, code);
			}
		}

#ifdef USE_OPENSSL
		if(m_sock_pSSL != NULL)
		{
			SSL_free(m_sock_pSSL);
			m_sock_pSSL = NULL;
		}
#endif
		if(m_sock != INVALID_SOCKET)
		{
			::shutdown(m_sock, SD_BOTH);
			::closesocket(m_sock);
			m_sock = INVALID_SOCKET;
		}
		if(m_sock_state > SOCKSTATE_CLOSED)
		{
			m_sock_state = SOCKSTATE_CLOSED;
		}

		return 1;
	}

	int JsClientSocket::beginDisconnect()
	{
		return 1;
	}

	int JsClientSocket::sslLoadCertificates(const char* szCertFile, const char* szKeyFile)
	{
		return 1;
	}

	void JsClientSocket::workerThreadProc_CleanUp(void *param)
	{
		int i;
		WorkerThreadInternalContext *pmyctx = (WorkerThreadInternalContext*)param;
		if(pmyctx->precvbuf != NULL)
		{
			free(pmyctx->precvbuf);
			pmyctx->precvbuf = NULL;
		}
		for(i=0; i<2; i++)
		{
			if(pmyctx->async_hEvents[i] != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(pmyctx->async_hEvents[i]);
				pmyctx->async_hEvents[i] = INVALID_HANDLE_VALUE;
			}
		}
		if(pmyctx->psockctx->m_stopworkerhandler != NULL && pmyctx->inited_userhandler)
		{
			pmyctx->psockctx->m_stopworkerhandler(pmyctx->psockctx, pmyctx->threadidx, pmyctx->pthreaduserctx);
			pmyctx->inited_userhandler = false;
		}
	}

	int JsClientSocket::workerThreadProc(JsCPPUtils::JsThread::ThreadContext *pThreadCtx, int threadindex, void *threadparam)
	{
		JsClientSocket *psockctx = (JsClientSocket*)threadparam;
		WorkerThreadInternalContext myctx(psockctx, threadindex, threadparam);

		int rc;
		int nrst;
		int msg_procstate = 0;
		
		bool started_connect = false;
		
#if defined(JSCUTILS_OS_WINDOWS)
		OVERLAPPED async_ovl;
		WSABUF recv_wsabuf;
		int async_state = 0;
#endif

		if(psockctx->m_startworkerposthandler != NULL)
		{
			if((nrst = psockctx->m_startworkerposthandler(psockctx, threadindex, &myctx.pthreaduserctx)) <= 0)
			{
				goto EXIT_STARTERR1;
			}
			myctx.inited_userhandler = true;
		}

#if defined(JSCUTILS_OS_LINUX)
		pthread_cleanup_push(workerThreadProc_CleanUp, &myctx);
#endif
		
#if defined(JSCUTILS_OS_WINDOWS)
		myctx.async_hEvents[0] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		myctx.async_hEvents[1] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#endif

		myctx.precvbuf = (char*)malloc(psockctx->m_conf_recvdatabufsize);

		while(pThreadCtx->_inthread_isRun())
		{
			JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

			if(msg_procstate == 0)
			{
				DWORD worker_dwWait;
				HANDLE worker_hEvents[3] = {psockctx->m_worker_msg.getEventHandle(), myctx.async_hEvents[0], myctx.async_hEvents[1]};
				bool _looptmp_sockreconn = false;
				int worker_msgrst = 0;
				worker_dwWait = WaitForMultipleObjects((psockctx->m_sock_state >= SOCKSTATE_CONNECTED) ? 3 : 1, worker_hEvents, FALSE, 1000);
				if((worker_dwWait == WAIT_TIMEOUT) && (psockctx->m_conf_bautoreconnect) && (psockctx->m_sock_state == SOCKSTATE_CLOSED))
				{
					worker_dwWait = (WAIT_OBJECT_0 + 0);
					spmsg = psockctx->m_autoreconn_spmsg;
					_looptmp_sockreconn = true;
				}
				switch(worker_dwWait)
				{
				case (WAIT_OBJECT_0 + 0):
					if(!_looptmp_sockreconn)
						worker_msgrst = psockctx->m_worker_msg.handlemsg_begin(&spmsg, 0, true); // 0 non-block
					
					switch(spmsg->cmd)
					{
					case WorkerThreadMessage::CMD_CONNECT:
						{
							WorkerThreadMessage_Connect *pmsg = (WorkerThreadMessage_Connect*)spmsg.getPtr();

#if defined(JSCUTILS_OS_WINDOWS)
							DWORD _tmp_dwBytes;
							BOOL _tmp_bRst;
							
							DWORD dwLocalAddrSize = pmsg->local_sockaddrlen;
							DWORD dwRemoteAddrSize = pmsg->remote_sockaddrlen;
#endif

							psockctx->_closeSocket();
							nrst = psockctx->_newSocket();
							if(nrst != 1)
							{
								spmsg->retval = nrst;
								break;
							}

							started_connect = 1;
							
							do {
#if defined(JSCUTILS_OS_LINUX)
								if(pmsg->local_bUse)
								{
									rc = ::bind(psockctx->m_sock, (struct sockaddr*)&pmsg->local_sockaddr, pmsg->local_sockaddrlen);
									if(rc < 0)
									{
										spmsg->retval = -errno;
										break;
									}
								}

								if(pmsg->remote_bUseHostname)
								{
									// TODO
								}

								rc = ::connect(psockctx->m_sock, (struct sockaddr*)&pmsg->remote_sockaddr, pmsg->remote_sockaddrlen);
								if(rc < 0)
								{
									spmsg->retval = -errno;
									break;
								}
#elif defined(JSCUTILS_OS_WINDOWS)
								if(pmsg->local_bUse)
								{
									rc = ::bind(psockctx->m_sock, (struct sockaddr*)&pmsg->local_sockaddr, pmsg->local_sockaddrlen);
									if(rc < 0)
									{
										spmsg->retval = -((int)GetLastError());
										break;
									}
								}
							
								psockctx->m_sock_state = SOCKSTATE_CONNECTING;
								if(pmsg->remote_bUseHostname)
								{
									TCHAR _tmp_tszPortName[16];
									_stprintf_s(_tmp_tszPortName, _T("%u"), pmsg->remote_port);
#ifdef _UNICODE
									_tmp_bRst = ::WSAConnectByNameW(psockctx->m_sock, (LPTSTR)pmsg->remote_strHostname.c_str(), _tmp_tszPortName, &dwLocalAddrSize, (struct sockaddr*)&pmsg->local_sockaddr, &dwRemoteAddrSize, (struct sockaddr*)&pmsg->remote_sockaddr, NULL, NULL);
#else
									_tmp_bRst = ::WSAConnectByNameA(psockctx->m_sock, (LPTSTR)pmsg->remote_strHostname.c_str(), _tmp_tszPortName, &dwLocalAddrSize, (struct sockaddr*)&pmsg->local_sockaddr, &dwRemoteAddrSize, (struct sockaddr*)&pmsg->remote_sockaddr, NULL, NULL);
#endif
									if(!_tmp_bRst)
									{
										spmsg->retval = -(::WSAGetLastError());
										break;
									}
								}else{
									rc = ::WSAConnect(psockctx->m_sock, (struct sockaddr*)&pmsg->remote_sockaddr, pmsg->remote_sockaddrlen, NULL, NULL, NULL, NULL);
									if(rc < 0)
									{
										spmsg->retval = -(::WSAGetLastError());
										break;
									}
								}

								/*
								_tmp_dwBytes = 0;
								_tmp_bRst = psockctx->m_win_fnConnectEx(psockctx->m_sock, _tmp_premoteaddr, _tmp_remoteaddrlen, NULL, 0, &_tmp_dwBytes, NULL);
								if(!_tmp_bRst)
								{
									spmsg->retval = -(::WSAGetLastError());
									break;
								}
								msg_procstate = 1;
								*/
#endif

#ifdef USE_OPENSSL
								if(psockctx->m_bUseSSL)
								{
									ERR_clear_error();
									psockctx->m_sslstate = 1;
									psockctx->m_sock_state = SOCKSTATE_CONNECTING_SSL;
									rc = SSL_set_fd(psockctx->m_sock_pSSL, (int)psockctx->m_sock);
									if(rc != 1)
									{
										spmsg->retval = 0;
										break;
									}
									rc = SSL_connect(psockctx->m_sock_pSSL);
									if(rc != 1)
									{
										ERR_print_errors_fp(stderr);
										spmsg->retval = -1;
										break;
									}
									psockctx->m_sslstate = 2;
								}
#endif
								
								::WSAResetEvent(myctx.async_hEvents[0]);
								::WSAResetEvent(myctx.async_hEvents[1]);
								rc = ::WSAEventSelect(psockctx->m_sock, myctx.async_hEvents[0], FD_READ | FD_CLOSE);
								if(rc != 0)
								{
									int tmpnerr = ::WSAGetLastError();
									spmsg->retval = -tmpnerr;
									break;
								}

								psockctx->m_sock_state = SOCKSTATE_CONNECTED;

								nrst = psockctx->m_connectedhandler(psockctx, myctx.pthreaduserctx, psockctx->m_sock);
								if(nrst != 1)
								{
									spmsg->retval = nrst;
									break;
								}

								spmsg->retval = 1;
							}while(0);

							if(spmsg->retval != 1)
							{
								psockctx->m_sock_state = SOCKSTATE_CLOSED;
								psockctx->_closeSocket();
							}
						}
						break;
					case WorkerThreadMessage::CMD_DATASEND:
						psockctx->_worker_send(spmsg);
						/*
						{
							WorkerThreadMessage_Send *pmsg = (WorkerThreadMessage_Send*)spmsg.getPtr();
							WSABUF _tmp_wsabuf;
							DWORD _tmp_dwBytes = 0;
							
							_tmp_wsabuf.buf = pmsg->data_pbuf;
							_tmp_wsabuf.len = pmsg->data_size;
							rc = ::WSASend(psockctx->m_sock, &_tmp_wsabuf, 1, &_tmp_dwBytes, 0, NULL, NULL);
							if(rc != 0)
							{
								int tmpnerr = ::WSAGetLastError();
								spmsg->retval = -tmpnerr;
								break;
							}

							spmsg->retval = 1;
						}
						*/
						break;
					}
					if((worker_msgrst == 1) && (msg_procstate != 1) && (!_looptmp_sockreconn))
						psockctx->m_worker_msg.handlemsg_end();

					break;
					
				case (WAIT_OBJECT_0 + 1):
					// Recv
#ifdef USE_OPENSSL
					if(psockctx->m_bUseSSL)
					{
						ERR_clear_error();
						errno = 0;
						rc = ::SSL_read(psockctx->m_sock_pSSL, myctx.precvbuf, psockctx->m_conf_recvdatabufsize);
						if(rc < 0)
						{
							int sslerr = SSL_get_error(psockctx->m_sock_pSSL, rc);
							ERR_print_errors_fp(stderr);
							switch(sslerr)
							{
							case SSL_ERROR_WANT_READ:
							case SSL_ERROR_WANT_WRITE:
								// errno = EAGAIN
								// Nothing work
								break;
							case SSL_ERROR_SYSCALL:
								if(errno == EINTR)
									break; // Nothing work
							case SSL_ERROR_SSL:
							default:
								psockctx->_closeSocket(-1);
								break;
							}
						}else if(rc == 0)
						{
							psockctx->_closeSocket(0);
						}else{
							if(psockctx->m_recvhandler)
							{
								nrst = psockctx->m_recvhandler(psockctx, myctx.pthreaduserctx, rc, myctx.precvbuf);
								if(nrst != 1)
								{
									psockctx->_closeSocket(nrst);
								}
							}
						}
					}else
#endif
					{
						WSABUF _tmp_wsabuf;
						DWORD _tmp_dwBytes = 0;
						DWORD _tmp_dwFlags = 0;
						
						_tmp_wsabuf.buf = myctx.precvbuf;
						_tmp_wsabuf.len = psockctx->m_conf_recvdatabufsize;
						rc = ::WSARecv(psockctx->m_sock, &_tmp_wsabuf, 1, &_tmp_dwBytes, &_tmp_dwFlags, NULL, NULL);
						if(rc < 0)
						{
							int tmpnerr = ::WSAGetLastError();
							if(tmpnerr != WSAEWOULDBLOCK)
								psockctx->_closeSocket(-tmpnerr);
						}else if(_tmp_dwBytes == 0){
							psockctx->_closeSocket(0);
						}else{
							if(psockctx->m_recvhandler)
							{
								nrst = psockctx->m_recvhandler(psockctx, myctx.pthreaduserctx, _tmp_dwBytes, myctx.precvbuf);
								if(nrst != 1)
								{
									psockctx->_closeSocket(nrst);
								}
							}
						}
					}
					break;
					
				case (WAIT_OBJECT_0 + 2):

					break;
				}

			}else{

			}
		}

		/*
		SSLERROR_SYSCALL, EINTR 처리
		*/

EXIT_STARTERR2:
#if defined(JSCUTILS_OS_LINUX)
		pthread_cleanup_pop(1);
#else
		workerThreadProc_CleanUp(&myctx);
#endif

EXIT_STARTERR1:
		return 0;
	}

	int JsClientSocket::beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const struct sockaddr *server_psockaddr, int server_sockaddrlen, bool bAutoReconnect, long timeoutms)
	{
		int retval;
		WorkerThreadMessage_Connect *pmsg;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

		if(m_sock_state <= SOCKSTATE_NOTINITED)
		{
			return 0;
		}

		pmsg = new WorkerThreadMessage_Connect(local_psockaddr, local_sockaddrlen, server_psockaddr, server_sockaddrlen, (const JSCUTILS_TYPE_DEFCHAR*)NULL, 0);
		spmsg = pmsg;
		m_conf_bautoreconnect = bAutoReconnect;
		if(bAutoReconnect)
			m_autoreconn_spmsg = spmsg;
		else
			m_autoreconn_spmsg = NULL;

		retval = m_worker_msg.post(spmsg, timeoutms);
		if((timeoutms != 0) && (retval == 1))
		{
			retval = spmsg->retval;
		}

		return retval;
	}

	int JsClientSocket::beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const std::basic_string<JSCUTILS_TYPE_DEFCHAR>& report_strHostname, uint16_t remote_port, bool bAutoReconnect, long timeoutms)
	{
		int retval;
		WorkerThreadMessage_Connect *pmsg;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

		if(m_sock_state <= SOCKSTATE_NOTINITED)
		{
			return 0;
		}

		pmsg = new WorkerThreadMessage_Connect(local_psockaddr, local_sockaddrlen, NULL, 0, report_strHostname, remote_port);
		spmsg = pmsg;
		m_conf_bautoreconnect = bAutoReconnect;
		if(bAutoReconnect)
			m_autoreconn_spmsg = spmsg;
		else
			m_autoreconn_spmsg = NULL;

		retval = m_worker_msg.post(spmsg, timeoutms);
		if((timeoutms != 0) && (retval == 1))
		{
			retval = spmsg->retval;
		}

		return retval;
	}
	int JsClientSocket::beginConnect(const struct sockaddr *local_psockaddr, int local_sockaddrlen, const JSCUTILS_TYPE_DEFCHAR* report_cszHostname, uint16_t remote_port, bool bAutoReconnect, long timeoutms)
	{
		int retval;
		WorkerThreadMessage_Connect *pmsg;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

		if(m_sock_state <= SOCKSTATE_NOTINITED)
		{
			return 0;
		}

		pmsg = new WorkerThreadMessage_Connect(local_psockaddr, local_sockaddrlen, NULL, 0, report_cszHostname, remote_port);
		spmsg = pmsg;

		retval = m_worker_msg.post(spmsg, timeoutms);
		if((timeoutms != 0) && (retval == 1))
		{
			retval = spmsg->retval;
		}

		return retval;
	}

	void JsClientSocket::setUserPtr(void *userptr)
	{
		m_userptr = userptr;
	}

	void *JsClientSocket::getUserPtr()
	{
		return m_userptr;
	}

	bool JsClientSocket::isUseSSL()
	{
		return m_bUseSSL;
	}

	JSCUTILS_SOCKET_T JsClientSocket::getSocket()
	{
		return m_sock;
	}

#ifdef USE_OPENSSL 
	SSL_CTX *JsClientSocket::getSSL_CTX()
	{
		return m_pSSLCtx;
	}
#endif
	

	int JsClientSocket::_worker_send(JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg)
	{
		WorkerThreadMessage_Send *pmsg = (WorkerThreadMessage_Send*)spmsg.getPtr();

		int rc;
		int neno;

		WSABUF _tmp_wsabuf;
		DWORD _tmp_dwBytes = 0;

		if(m_sslstate == 1)
		{
			spmsg->retval = 0;
		}else if(m_sslstate == 2)
		{
#ifdef USE_OPENSSL
			ERR_clear_error();
			errno = 0;
			rc = ::SSL_write(m_sock_pSSL, pmsg->data_pbuf, pmsg->data_size);
			if(rc < 0)
			{
				int sslerr;
				neno = errno;
				sslerr = SSL_get_error(m_sock_pSSL, rc);
				ERR_print_errors_fp(stderr);
				switch (sslerr)
				{
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					spmsg->retval = -EAGAIN;
					break;
				case SSL_ERROR_SYSCALL:
					spmsg->retval = ::GetLastError();
					if(spmsg->retval == 0)
						spmsg->retval = ::WSAGetLastError();
				default:
					spmsg->retval = -errno;
				}
			}else
				spmsg->retval = 1;
#else
			spmsg->retval = 0;
#endif
		}else{
			_tmp_wsabuf.buf = pmsg->data_pbuf;
			_tmp_wsabuf.len = pmsg->data_size;
			rc = ::WSASend(m_sock, &_tmp_wsabuf, 1, &_tmp_dwBytes, 0, NULL, NULL);
			if(rc != 0)
			{
				int tmpnerr = ::WSAGetLastError();
				spmsg->retval = -tmpnerr;
			}else{
				spmsg->retval = 1;
			}
		}


		return pmsg->retval;
	}

	int JsClientSocket::_worker_recv(JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg)
	{
		WorkerThreadMessage_Recv *pmsg = (WorkerThreadMessage_Recv*)spmsg.getPtr();
		
		int retval = 0;
		int rc;
		int neno;

		WSABUF _tmp_wsabuf;
		DWORD _tmp_dwBytes = 0;
		DWORD _tmp_dwFlags = 0;

		int processedLen = 0;

		if(m_sslstate == 1)
		{
			spmsg->retval = 0;
		}else if(m_sslstate == 2)
		{
#ifdef USE_OPENSSL
			do {
				ERR_clear_error();
				errno = 0;
				rc = ::SSL_read(m_sock_pSSL, &pmsg->data_pbuf[processedLen], pmsg->data_size - processedLen);
				if(rc < 0)
				{
					int sslerr;
					neno = errno;
					sslerr = SSL_get_error(m_sock_pSSL, rc);
					ERR_print_errors_fp(stderr);
					switch (sslerr)
					{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						retval = -EAGAIN;
						break;
					default:
						retval = -errno;
					}
				}else{
					processedLen += rc;
				}
				if(((processedLen >= pmsg->data_size) && (pmsg->readfixedsize)) || ((processedLen > 0) && (!pmsg->readfixedsize)))
				{
					spmsg->retval = 1;
					break;
				}
				if(rc < 0)
					break;
			}while(1);
#else
			spmsg->retval = 0;
#endif
		}else{
			do {
				_tmp_wsabuf.buf = &pmsg->data_pbuf[processedLen];
				_tmp_wsabuf.len = pmsg->data_size - processedLen;
				_tmp_dwBytes = 0;
				rc = ::WSARecv(m_sock, &_tmp_wsabuf, 1, &_tmp_dwBytes, &_tmp_dwFlags, NULL, NULL);
				if(rc != 0)
				{
					int tmpnerr = ::WSAGetLastError();
					spmsg->retval = -tmpnerr;
				}else{
					spmsg->retval = 1;
					processedLen += _tmp_dwBytes;
				}
				if(((processedLen >= pmsg->data_size) && (pmsg->readfixedsize)) || ((processedLen > 0) && (!pmsg->readfixedsize)))
					break;
				if(rc < 0)
					break;
			}while(1);
		}

		pmsg->recv_readsize = processedLen;

		return pmsg->retval;
	}

	int JsClientSocket::send(char *pdata, int size, int timeoutms, Client_SentHandler_t senthandler)
	{
		int retval;
		WorkerThreadMessage_Send *pmsg;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

		JSTHREAD_THREADID_TYPE curtid;
		
		if(senthandler != NULL)
			return 0;

		if(m_sock_state < SOCKSTATE_CONNECTED)
		{
			return 0;
		}

#if defined(JSCUTILS_OS_LINUX)
		curtid = pthread_self();
#elif defined(JSCUTILS_OS_WINDOWS)
		curtid = GetCurrentThreadId();
#endif

		pmsg = new WorkerThreadMessage_Send(pdata, size, timeoutms);
		spmsg = pmsg;
		
		pmsg->data_pbuf = pdata;
		pmsg->data_size = size;

		if(m_worker_tid == curtid)
		{
			retval = _worker_send(spmsg);
		}else{
			retval = m_worker_msg.post(spmsg, timeoutms);
			if((timeoutms != 0) && (retval == 1))
			{
				retval = spmsg->retval;
			}
		}

		return retval;
	}

	int JsClientSocket::recv(char *pdata, int size, int *preadbytes, bool readfixedsize, int timeoutms)
	{
		int retval;
		WorkerThreadMessage_Recv *pmsg;
		JsCPPUtils::SmartPointer<WorkerThreadMessage> spmsg;

		JSTHREAD_THREADID_TYPE curtid;
		
		if(m_sock_state < SOCKSTATE_CONNECTED)
		{
			return 0;
		}

#if defined(JSCUTILS_OS_LINUX)
		curtid = pthread_self();
#elif defined(JSCUTILS_OS_WINDOWS)
		curtid = GetCurrentThreadId();
#endif

		pmsg = new WorkerThreadMessage_Recv(pdata, size, readfixedsize, timeoutms);
		spmsg = pmsg;
		
		pmsg->data_pbuf = pdata;
		pmsg->data_size = size;

		if(m_worker_tid == curtid)
		{
			retval = _worker_recv(spmsg);
			if(preadbytes != NULL)
				*preadbytes = pmsg->recv_readsize;
		}else{
			return 0;
			/*
			retval = m_worker_msg.post(spmsg, timeoutms);
			if((timeoutms != 0) && (retval == 1))
			{
				retval = spmsg->retval;
			}
			*/
		}

		return retval;
	}
}
