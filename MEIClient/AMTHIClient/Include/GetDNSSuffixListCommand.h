/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetDNSSuffixListCommand.h

--*/

#ifndef __GET_DNS_SUFFIX_LIST_COMMAND_H__
#define __GET_DNS_SUFFIX_LIST_COMMAND_H__

#include "AMTHICommand.h"
#include "MEIparser.h"

namespace Intel
{
	namespace MEI_Client
	{
		namespace AMTHI_Client
		{
			struct GET_DNS_SUFFIX_LIST_RESPONSE
			{
				std::vector<uint8_t> HashHandles;

				void parse (std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator end)
				{
					Intel::MEI_Client::parseSimpleArray16(HashHandles, itr, end);
				}
			};

			class GetDNSSuffixListRequest : public AMTHICommandRequest
			{
			public:
				GetDNSSuffixListRequest() : AMTHICommandRequest(REQUEST_COMMAND_NUMBER) {}
				virtual ~GetDNSSuffixListRequest() {}

			private:
				static const uint32_t REQUEST_COMMAND_NUMBER = 0x0400003E;
			};

			class GetDNSSuffixListCommand : public AMTHICommand
			{
			public:

				GetDNSSuffixListCommand ()
				{
					m_request = std::make_shared<GetDNSSuffixListRequest>();
					Transact();
				}
				virtual ~GetDNSSuffixListCommand () {}

				GET_DNS_SUFFIX_LIST_RESPONSE getResponse() { return m_response.getResponse(); }

			private:
				virtual void parseResponse(const std::vector<uint8_t>& buffer)
				{
					m_response = AMTHICommandResponse<GET_DNS_SUFFIX_LIST_RESPONSE>(buffer, RESPONSE_COMMAND_NUMBER);
				}

				AMTHICommandResponse<GET_DNS_SUFFIX_LIST_RESPONSE> m_response;

				static const uint32_t RESPONSE_COMMAND_NUMBER = 0x0480003E;

			};
		} // namespace AMTHI_Client
	} // namespace MEI_Client
} // namespace Intel

#endif //__GET_DNS_SUFFIX_LIST_COMMAND_H__