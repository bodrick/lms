/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2009-2023 Intel Corporation
 */
/*++

@file: EthernetSettingsWSManClient.cpp

--*/

#include "EthernetSettingsWSManClient.h"
#include "WsmanClientLog.h"

EthernetSettingsWSManClient::EthernetSettingsWSManClient(unsigned int port) : BaseWSManClient(port), m_isInit(false)
{
}

EthernetSettingsWSManClient::EthernetSettingsWSManClient(unsigned int port, const std::string &userName, const std::string &password) :
	BaseWSManClient(port, userName, password), m_isInit(false)
{
}

EthernetSettingsWSManClient::~EthernetSettingsWSManClient()
{
}

unsigned int EthernetSettingsWSManClient::Enumerate(std::vector<std::shared_ptr<Intel::Manageability::Cim::Typed::AMT_EthernetPortSettings>> &EthernetSettings)
{
	unsigned int retValue;

	try {
		if (!m_endpoint)
			SetEndpoint();
		//Lock WsMan to prevent reentry
		std::lock_guard<std::mutex> lock(WsManSemaphore());
		EthernetSettings = Intel::Manageability::Cim::Typed::AMT_EthernetPortSettings::Enumerate(m_client.get());
		retValue = 0;
	}
	catch (std::exception& e)
	{
		const char* reason =  e.what();
		retValue = ERROR_UNKNOWN_ERROR;
		WSMAN_DEBUG("Error: failed while calling get AMT_EthernetPortSettings: %C\n", reason);
		m_endpoint = false;
	}	
	return retValue;
}