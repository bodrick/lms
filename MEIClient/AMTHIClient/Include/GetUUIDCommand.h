/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetUUIDCommand.h

--*/

#ifndef __GET_UUID_COMMAND_H__
#define __GET_UUID_COMMAND_H__

#include "AMTHICommand.h"
#include "MEIparser.h"

namespace Intel
{
	namespace MEI_Client
	{
		namespace AMTHI_Client
		{
			struct GET_UUID_RESPONSE
			{
				uint8_t UUID[16];

				void parse (std::vector<uint8_t>::const_iterator &itr, const std::vector<uint8_t>::const_iterator &end)
				{
					parseData(*this, itr, end);
				}
			} ;

			class GetUUIDCommand : public AMTHICommand
			{
			public:
				GetUUIDCommand();
				virtual ~GetUUIDCommand() {}

				GET_UUID_RESPONSE getResponse();

			private:
				virtual void parseResponse(const std::vector<uint8_t>& buffer);

				std::shared_ptr<AMTHICommandResponse<GET_UUID_RESPONSE>> m_response;

				static const uint32_t RESPONSE_COMMAND_NUMBER = 0x0480005C;
			};

			class GetUUIDRequest : public AMTHICommandRequest
			{
			public:
				GetUUIDRequest () {}
				virtual ~GetUUIDRequest() {}

			private:
				static const uint32_t REQUEST_COMMAND_NUMBER = 0x0400005C;
				virtual unsigned int requestHeaderCommandNumber()
				{
					//this is the command number (taken from the AMTHI document)
					return REQUEST_COMMAND_NUMBER;
				}

				virtual uint32_t requestDataSize()
				{
					return 0;
				}
				virtual std::vector<uint8_t> SerializeData();
			};
		} // namespace AMTHI_Client
	} // namespace MEI_Client
} // namespace Intel

#endif //__GET_UUID_COMMAND_H__