/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetImageFWVersionCommand.h

--*/

#ifndef __GET_IMAGE_FW_VERSION_COMMAND_H__
#define __GET_IMAGE_FW_VERSION_COMMAND_H__

#include "MEIparser.h"
#include "MKHICommand.h"

namespace Intel
{
	namespace MEI_Client
	{
		namespace MKHI_Client
		{
			struct GET_IMAGE_FW_VERSION_REQUEST
			{
				uint32_t PartitionId;
			};

			struct FW_VERSION
			{
				uint16_t Major;
				uint16_t Minor;
				uint16_t Hotfix;
				uint16_t Build;
			};

			struct FLASH_PARTITION_DATA
			{
				uint8_t    ModuleName[12];
				FW_VERSION Version;
				uint32_t   VendorId;
				uint32_t   Reserved[16];
			};

			const int MAXIMUM_IPU_SUPPORTED = 20;

			struct GET_IMAGE_FW_VERSION_RESPONSE
			{
				GET_IMAGE_FW_VERSION_RESPONSE() : NumOfModules(0), ManifestData { 0 } {}
				uint32_t             NumOfModules;
				FLASH_PARTITION_DATA ManifestData[MAXIMUM_IPU_SUPPORTED];

				void parse(std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator &end)
				{
					Intel::MEI_Client::parseData(NumOfModules, itr, end);
					Intel::MEI_Client::parseArray(ManifestData, NumOfModules, itr, end);
				}
			};

			class GetImageFWVersionRequest : public MKHICommandRequest
			{
			public:
				GetImageFWVersionRequest(uint32_t PartitionId) :
					MKHICommandRequest(REQUEST_COMMAND_NUMBER, MKHI_GEN_GROUP_ID), _partitionId(PartitionId) {}
				virtual ~GetImageFWVersionRequest() {}

			private:
				static const uint32_t REQUEST_COMMAND_NUMBER = 0x1C;

				virtual uint32_t requestDataSize()
				{
					return sizeof(GET_IMAGE_FW_VERSION_REQUEST);
				}
				virtual std::vector<uint8_t> SerializeData()
				{
					GET_IMAGE_FW_VERSION_REQUEST req;
					req.PartitionId = _partitionId;
					return std::vector<uint8_t>((uint8_t*)&req, (uint8_t*)&req + sizeof(GET_IMAGE_FW_VERSION_REQUEST));
				}
				uint32_t _partitionId;
			};

			class GetImageFWVersionCommand : public MKHICommand
			{
			public:

				GetImageFWVersionCommand(uint32_t PartitionId)
				{
					m_request = std::make_shared<GetImageFWVersionRequest>(PartitionId);
					Transact();
				}
				virtual ~GetImageFWVersionCommand() {}

				GET_IMAGE_FW_VERSION_RESPONSE getResponse() { return m_response.getResponse(); }

			private:
				virtual void parseResponse(const std::vector<uint8_t>& buffer)
				{
					m_response = MKHICommandResponse<GET_IMAGE_FW_VERSION_RESPONSE>(buffer, RESPONSE_COMMAND_NUMBER, MKHI_GEN_GROUP_ID);
				}

				MKHICommandResponse<GET_IMAGE_FW_VERSION_RESPONSE> m_response;

				static const uint32_t RESPONSE_COMMAND_NUMBER = 0x1C;
			};
		} // namespace MKHI_Client
	} // namespace MEI_Client
} // namespace Intel

#endif //__GET_IMAGE_FW_VERSION_COMMAND_H__
