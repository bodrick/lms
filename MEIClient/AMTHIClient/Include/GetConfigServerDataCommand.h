/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetConfigServerDataCommand.h

--*/

#ifndef __GET_CONFIG_SERVER_DATA_COMMAND_H__
#define __GET_CONFIG_SERVER_DATA_COMMAND_H__

#include "AMTHICommand.h"
#include "MEIparser.h"
#include "AmtAnsiString.h"
#include <string>
#include <vector>
#include <iostream>

namespace Intel
{
	namespace MEI_Client
	{
		namespace AMTHI_Client
		{
			struct CFG_GET_CONFIG_SERVER_DATA_RESPONSE
			{
				CFG_GET_CONFIG_SERVER_DATA_RESPONSE() : ServerPort(0) {}
				uint16_t    ServerPort;
				std::string ServerAddr;
				std::string FQDN;

				void parse (std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator end)
				{
					Intel::MEI_Client::parseData(ServerPort,itr,end);
					CFG_IP_ADDR ServerAddrTmp;
					Intel::MEI_Client::parseData(ServerAddrTmp,itr,end);
					ServerAddr = ServerAddrTmp.toString();
					FQDN = AmtAnsiString(itr, end).getString();
				}
			};

			class GetConfigServerDataRequest : public AMTHICommandRequest
			{
			public:
				GetConfigServerDataRequest() : AMTHICommandRequest(REQUEST_COMMAND_NUMBER) {}
				virtual ~GetConfigServerDataRequest() {}

			private:
				static const uint32_t REQUEST_COMMAND_NUMBER = 0x0400004E;
			};

			class GetConfigServerDataCommand : public AMTHICommand
			{
			public:

				GetConfigServerDataCommand()
				{
					m_request = std::make_shared<GetConfigServerDataRequest>();
					Transact();
				}
				virtual ~GetConfigServerDataCommand() {}

				CFG_GET_CONFIG_SERVER_DATA_RESPONSE getResponse() { return m_response.getResponse(); }

			private:
				virtual void parseResponse(const std::vector<uint8_t>& buffer)
				{
					m_response = AMTHICommandResponse<CFG_GET_CONFIG_SERVER_DATA_RESPONSE>(buffer, RESPONSE_COMMAND_NUMBER);
				}

				AMTHICommandResponse<CFG_GET_CONFIG_SERVER_DATA_RESPONSE> m_response;

				static const uint32_t RESPONSE_COMMAND_NUMBER = 0x0480004E;
			};
		} // namespace AMTHI_Client
	} // namespace MEI_Client
} // namespace Intel

#endif //__GET_CONFIG_SERVER_DATA_COMMAND_H__