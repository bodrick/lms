/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetCodeVersionCommand.h

--*/

#ifndef __GET_CODE_VERSION_COMMAND_H__
#define __GET_CODE_VERSION_COMMAND_H__

#include "AMTHICommand.h"
#include "MEIparser.h"

namespace Intel
{
namespace MEI_Client
{
namespace AMTHI_Client
{
	struct AMT_VERSION_TYPE
	{
		std::string Description;
		std::string Version;
		void parse (std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator end)
		{
			AMT_UNICODE_STRING   description;
			AMT_UNICODE_STRING   version;

			Intel::MEI_Client::parseData(description,itr,end);
			Intel::MEI_Client::parseData(version,itr,end);
			std::string tmpDsc((char*)(description.String));
			Description = tmpDsc;
			std::string tmpVer((char*)(version.String));
			Version = tmpVer;
		}
	};

	static const uint32_t BIOS_VERSION_LEN = 65;

	struct CODE_VERSIONS
	{
		CODE_VERSIONS() : BiosVersion { 0 } {}
		uint8_t   BiosVersion[BIOS_VERSION_LEN];
		std::vector<AMT_VERSION_TYPE> Versions;
		void parse (std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator end)
		{
			Intel::MEI_Client::parseArray (BiosVersion, BIOS_VERSION_LEN, itr, end);
			Intel::MEI_Client::parseComplexArray(Versions, itr, end);
		}
	};

	class GetCodeVersionRequest : public AMTHICommandRequest
	{
	public:
		GetCodeVersionRequest() : AMTHICommandRequest(REQUEST_COMMAND_NUMBER) {}
		virtual ~GetCodeVersionRequest() {}

	private:
		static const uint32_t REQUEST_COMMAND_NUMBER = 0x0400001A;
	};

	class GetCodeVersionCommand : public AMTHICommand
	{
	public:

		GetCodeVersionCommand()
		{
			m_request = std::make_shared<GetCodeVersionRequest>();
			Transact();
		}
		virtual ~GetCodeVersionCommand() {}

		CODE_VERSIONS getResponse() { return m_response.getResponse(); }

	private:
		virtual void parseResponse(const std::vector<uint8_t>& buffer)
		{
			m_response = AMTHICommandResponse<CODE_VERSIONS>(buffer, RESPONSE_COMMAND_NUMBER);
		}

		AMTHICommandResponse<CODE_VERSIONS> m_response;

		static const uint32_t RESPONSE_COMMAND_NUMBER = 0x0480001A;
	};
} // namespace AMTHI_Client
} // namespace MEI_Client
} // namespace Intel

#endif //__GET_CODE_VERSION_COMMAND_H__