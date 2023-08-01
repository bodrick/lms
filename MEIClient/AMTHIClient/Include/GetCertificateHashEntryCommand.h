/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
/*++

@file: GetCertificateHashEntryCommand.h

--*/

#ifndef __GET_CERTIFICATE_HASH_ENTRY_COMMAND_H__
#define __GET_CERTIFICATE_HASH_ENTRY_COMMAND_H__

#include "AMTHICommand.h"
#include "MEIparser.h"
#include "AmtAnsiString.h"

namespace Intel
{
	namespace MEI_Client
	{
		namespace AMTHI_Client
		{
			const static uint32_t CERT_HASH_MAX_LENGTH = 64;

			enum CERT_HASH_ALGORITHM8	//uint8_t
			{
				CERT_HASH_ALGORITHM8_MD5 = 0,  // 16 bytes
				CERT_HASH_ALGORITHM8_SHA1,     // 20 bytes
				CERT_HASH_ALGORITHM8_SHA256,   // 32 bytes
				CERT_HASH_ALGORITHM8_SHA512   // 64 bytes
			};

			struct GET_CERTIFICATE_HASH_ENTRY_RESPONSE
			{
				GET_CERTIFICATE_HASH_ENTRY_RESPONSE() : IsDefault(0), IsActive(0), CertificateHash { 0 },
					HashAlgorithm(CERT_HASH_ALGORITHM8_MD5) {}
				bool					IsDefault;
				bool					IsActive;
				uint8_t					CertificateHash[CERT_HASH_MAX_LENGTH];
				uint8_t					HashAlgorithm; //Represents CERT_HASH_ALGORITHM8
				std::string				Name;

				void parse (std::vector<uint8_t>::const_iterator& itr, const std::vector<uint8_t>::const_iterator &end)
				{
					parseData (IsDefault, itr, end);
					parseData (IsActive, itr, end);
					parseArray(CertificateHash, CERT_HASH_MAX_LENGTH, itr, end);
					parseData (HashAlgorithm, itr, end);
					Name = AmtAnsiString(itr, end).getString();
				}
			};

			class GetCertificateHashEntryRequest : public AMTHICommandRequest
			{
			public:
				GetCertificateHashEntryRequest(uint32_t hashHandle) : _hashHandle(hashHandle), AMTHICommandRequest(REQUEST_COMMAND_NUMBER) {}
				virtual ~GetCertificateHashEntryRequest() {}

			private:

				static const uint32_t REQUEST_COMMAND_NUMBER = 0x0400002D;

				virtual uint32_t requestDataSize()
				{
					return sizeof(uint32_t);
				}
				virtual std::vector<uint8_t> SerializeData()
				{
					std::vector<uint8_t> output((std::uint8_t*)&_hashHandle, (std::uint8_t*)&_hashHandle + sizeof(uint32_t));
					return output;
				}
				uint32_t _hashHandle;
			};

			class GetCertificateHashEntryCommand : public AMTHICommand
			{
			public:

				GetCertificateHashEntryCommand(uint32_t hashHandle)
				{
					m_request = std::make_shared<GetCertificateHashEntryRequest>(hashHandle);
					Transact();
				}
				virtual ~GetCertificateHashEntryCommand() {}

				GET_CERTIFICATE_HASH_ENTRY_RESPONSE getResponse() { return m_response.getResponse(); }

			private:
				virtual void parseResponse(const std::vector<uint8_t>& buffer)
				{
					m_response = AMTHICommandResponse<GET_CERTIFICATE_HASH_ENTRY_RESPONSE>(buffer, RESPONSE_COMMAND_NUMBER);
				}

				AMTHICommandResponse<GET_CERTIFICATE_HASH_ENTRY_RESPONSE> m_response;

				static const uint32_t RESPONSE_COMMAND_NUMBER = 0x0480002D;
			};
		} // namespace AMTHI_Client
	} // namespace MEI_Client
} // namespace Intel

#endif //__GET_CERTIFICATE_HASH_ENTRY_COMMAND_H__