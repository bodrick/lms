/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2009-2023 Intel Corporation
 */
/*++

@file: AMTRedirectionServiceWSManClient.cpp

--*/

#include "AMTRedirectionServiceWSManClient.h"
#include "WsmanClientLog.h"
#include "WsmanClientCatch.h"

namespace CimTyped = Intel::Manageability::Cim::Typed;

typedef enum _TEMINATE_SESSION_QUALIFIER
{
	TEMINATE_SESSION_QUALIFIER_SUCCESS = 0x0,
	TEMINATE_SESSION_QUALIFIER_INTERNAL_ERROR = 0x1,
	TEMINATE_SESSION_QUALIFIER_INVALID_PARAMS = 0x2,
	TEMINATE_SESSION_QUALIFIER_INVALID_STATE = 0x3,
	TEMINATE_SESSION_QUALIFIER_RESERVED = 0x4
} TEMINATE_SESSION_QUALIFIER;

AMTRedirectionServiceWSManClient::AMTRedirectionServiceWSManClient(unsigned int port) : BaseWSManClient(port), m_isInit(false)
{
}

AMTRedirectionServiceWSManClient::~AMTRedirectionServiceWSManClient(void)
{
}

bool AMTRedirectionServiceWSManClient::TerminateSession(unsigned int SessionType)
{
	WSMAN_DEBUG("AMTRedirectionServiceWSManClient::TerminateSession(%d)\n",SessionType);
	
	if (!Init())
		return false;
	try 
	{
		//Lock WsMan to prevent reentry
		std::lock_guard<std::mutex> lock(WsManSemaphore());
		
		CimTyped::AMT_RedirectionService::TerminateSession_INPUT request;
		request.SessionType(SessionType);
		unsigned int returnedVal = m_service.TerminateSession(request);
		WSMAN_DEBUG("AMTRedirectionServiceWSManClient::TerminateSession(%d) ReturnValue=%d\n", SessionType, returnedVal);
		if (returnedVal != TEMINATE_SESSION_QUALIFIER_SUCCESS && returnedVal != TEMINATE_SESSION_QUALIFIER_INVALID_STATE)  // may fail if there is no active session, in this case return true 
		{
			return false;
		}
	}
	CATCH_exception_return("AMTRedirectionServiceWSManClient::TerminateSession")

	return true;
}

bool AMTRedirectionServiceWSManClient::RedirectionState(unsigned short *state)
{
	if (!Init(true))
		return false;
	if (m_service.EnabledStateExists())
	{
		*state = m_service.EnabledState();
		return true;
	}
	return false;
}

bool AMTRedirectionServiceWSManClient::Init(bool forceGet)
{
	if (!forceGet && m_isInit)
	{
		return true;
	}
	m_isInit = false;
	
	try
	{
		if (!m_endpoint)
			SetEndpoint();
		//Lock WsMan to prevent reentry
		std::lock_guard<std::mutex> lock(WsManSemaphore());
		m_service.WsmanClient(m_client.get());
		m_service.Get();
		m_isInit = true;
	}
	CATCH_exception("AMTRedirectionServiceWSManClient::Init")
	
	return m_isInit;
}