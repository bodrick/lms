/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2007-2019 Intel Corporation
 */
 /**
 * @file smbios_tables.cpp
 *
 * Implements functions to read settings & flags set by OEM in SMBIOS table
 *
 *
 */
#include "SMBIOS_Reader.h"
#include "UNSDebug.h"

#ifdef WIN32
#include "wbemidl.h"
#else
#include <vector>
#include <fstream>
#include <iterator>
#endif // WIN32

// Returns the length of the formatted section of a table beginning at the specified index within the smb buffer.
uint32_t SMBIOS_Reader::formatted_table_len(unsigned char *smbios_table_data, uint32_t index, uint32_t smbios_table_len)
{
	unsigned int length = (unsigned int) smbios_table_data[index+1];
	// very basic overflow protection in the add operation below
	if (length > (smbios_table_len) && (index + length) > (smbios_table_len - 1))
		return 0;
	else
		return length;
}

// Returns the length of the unformatted section of a table (string table) beginning at the specified
// index within the smb buffer.
uint32_t SMBIOS_Reader::unformatted_table_len(unsigned char *smbios_table_data, uint32_t index, uint32_t smbios_table_len)
{
	unsigned int start_point = index;
	unsigned int formatted_table_length = (unsigned int) smbios_table_data[index+1];

	// very basic overflow protection in the add operation below
	if (formatted_table_length < smbios_table_len)
	{
		index += formatted_table_length;
		while (index < smbios_table_len-1)
		{
			if ((smbios_table_data[index]==0) && (smbios_table_data[index+1]==0))
			{
				return index - formatted_table_length + 2 - start_point;
			}
			index++;
		}
	}
	return 0;
}

// Returns in the SM_BIOS_CAPABILITIES all the relevant settings or false on error related table structure
bool SMBIOS_Reader::areSmbiosFlagsSet(unsigned char *smbios_table_data, uint32_t smbios_table_len, SM_BIOS_CAPABILITIES *pCapabilities)
{
	unsigned int index = 0;
	unsigned int table_len1=0;
	unsigned int table_len2=0;
	unsigned int current_table_length = 0;
	struct SMBIOS_Type131 *SM_table;

	while(index < smbios_table_len - 1)
	{
		table_len1 = SMBIOS_Reader::formatted_table_len(smbios_table_data, index, smbios_table_len);
		table_len2 = SMBIOS_Reader::unformatted_table_len(smbios_table_data, index, smbios_table_len);
		current_table_length = table_len1 + table_len2;

		if (current_table_length == 0) break;

		SM_table = (struct SMBIOS_Type131 *) &smbios_table_data[index];

		if (SM_table->Header.Type == SMBIOS_INTEL_OEM_TYPE
			&& strncmp((char *)SM_table->vProSig, VPRO_STRING, sizeof(VPRO_STRING)) == 0)
		{
			DbgPrintW(L"areSmbiosFlagsSet:: Found vPRO table\n");
			if (current_table_length < sizeof(struct SMBIOS_Type131))
			{
				DbgPrintW(L"areSmbiosFlagsSet:: Incorrect current_table_length - returning false\n");
				return false;
			}
			else
			{
				// MEBx version defined as such:
				pCapabilities->MEBx_Major = SM_table->MebxVer[0]; // MebxVer[1] -> Mebx Major version
				pCapabilities->MEBx_Minor = SM_table->MebxVer[1]; // MebxVer[0] -> Mebx Minor version
				pCapabilities->MEBx_Hotfix = SM_table->MebxVer[2];// MebxVer[3] -> Mebx Hotfix number
				pCapabilities->MEBx_Build = SM_table->MebxVer[3]; // MebxVer[2] -> Mebx Build number
				DbgPrintW(L"areSmbiosFlagsSet:: MEBx version %d.%d.%d.%d\n",pCapabilities->MEBx_Major,pCapabilities->MEBx_Minor,pCapabilities->MEBx_Hotfix,pCapabilities->MEBx_Build );

				// ME version defined as such:
				pCapabilities->ME_Major = SM_table->MeVer[1]; // MeVer[1] -> Me Major version
				pCapabilities->ME_Minor = SM_table->MeVer[0]; // MeVer[0] -> Me Minor version
				pCapabilities->ME_Hotfix = SM_table->MeVer[3];// MeVer[3] -> Me Hotfix number
				pCapabilities->ME_Build = SM_table->MeVer[2]; // MeVer[2] -> Me Build number
				DbgPrintW(L"areSmbiosFlagsSet:: ME version %d.%d.%d.%d\n",pCapabilities->ME_Major,pCapabilities->ME_Minor,pCapabilities->ME_Hotfix,pCapabilities->ME_Build );

				// AT properties are defined as such:
				if (SM_table->MeCap.AtpSupported)
				{
					DbgPrintW(L"areSmbiosFlagsSet:: TDT supported %d !\n",SM_table->MeCap.AtpSupported);
					pCapabilities->AT_Allowed = SM_table->MeCap.AtpSupported;
					at_info1* AtInfo1 = (struct at_info1*)&SM_table->Rerserve[0];
					DbgPrintW(L"areSmbiosFlagsSet:: AT enrolled: %d\n", AtInfo1->atEnrolled);
					pCapabilities->AT_Enrolled = AtInfo1->atEnrolled;
				}
				else
				{
					pCapabilities->AT_Allowed = 0;
					pCapabilities->AT_Enrolled = 0;
					DbgPrintW(L"areSmbiosFlagsSet:: AT not supported !\n");
				}
				return true;
			}
		}
		else if (SM_table->Header.Type == SMBIOS_END_OF_TABLE_TYPE)
		{
			DbgPrintW(L"areSmbiosFlagsSet:: Type ==  SMBIOS_END_OF_TABLE_TYPE --> returning false\n");
			return false;
		}

		index += current_table_length;

	}

	DbgPrintW(L"areSmbiosFlagsSet:: returning false\n");
	return false;
}

#ifdef WIN32
// Check For Smbios Flags using WMI to read them from Intel SMBIOS tables
uint32_t SMBIOS_Reader::CheckForSmbiosFlags()
{
	int ret = SMBIOS_FAILURE;
	HRESULT hr;
	BSTR bstrMsg;
	BOOL uninitCom = FALSE;

    // WMI Initialize COM.
    hr =  CoInitializeEx(0, COINIT_MULTITHREADED);

	if (hr == S_FALSE || hr == S_OK)
	{
		uninitCom = TRUE;
	}
	else if (hr != RPC_E_CHANGED_MODE)
    {
		ret = ERROR_COINITIALIZE;
        return ret;
    }

    // WMI interface locator for host

	IWbemLocator* ploc = 0;

    hr = CoCreateInstance(
		__uuidof(WbemLocator),
        0,
        CLSCTX_INPROC_SERVER,
		__uuidof(IWbemLocator), (LPVOID *) &ploc);

    if (hr<0)
    {
		DbgPrintW(L"CheckForSmbiosFlags:: Error during IWbemLocator initialization\n");
		ret = ERROR_COCREATEINSTANCE;
		goto comfailure;
    }

    IWbemServices* psvc = 0;

    // Now connect to the wmi namespace to make call for IWbemServices

    bstrMsg = SysAllocString(L"ROOT\\WMI");
    hr = ploc->ConnectServer(bstrMsg,  NULL,  NULL, 0, NULL, 0, 0, &psvc );
    SysFreeString(bstrMsg);
    if (hr<0)
    {
		DbgPrintW(L"CheckForSmbiosFlags:: Error during connection to WMI namespace\n");
        ploc->Release();
		ret = ERROR_WMI_CONNECT;
		goto comfailure;
    }

    // Using IWbemServices proxy

    hr = CoSetProxyBlanket( psvc,  RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
       RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

    if (hr<0)
    {
		DbgPrintW(L"CheckForSmbiosFlags:: Error in set up WMI proxy\n");
        psvc->Release();
        ploc->Release();
		ret = ERROR_WMI_SET_PROXY;
		goto comfailure;
    }

    IEnumWbemClassObject* penum = NULL;
	bstrMsg = SysAllocString(L"MSSMBios_RawSMBiosTables");
    hr = psvc->CreateInstanceEnum(bstrMsg, 0, NULL, &penum);
	SysFreeString(bstrMsg);
    if ((hr < 0) || (penum == NULL))
    {
		DbgPrintW(L"CheckForSmbiosFlags:: Error during SMBIOS tables enumration\n");
        psvc->Release();
        ploc->Release();
		ret = ERROR_SMBIOS_ENUMERATION;
		goto comfailure;
    }
    do
    {
        IWbemClassObject* pinst = NULL;
        ULONG dword_repeat = NULL;

        hr = penum->Next(WBEM_INFINITE, 1, &pinst, &dword_repeat);
        //adding check for (pinst != NULL) when bios doesn't respond
		if ((hr >= 0) && (pinst != NULL))
		{
			VARIANT smbios_variant;
			VariantInit(&smbios_variant);
			CIMTYPE type;

			bstrMsg = SysAllocString(L"SmbiosMajorVersion");
			hr = pinst->Get(bstrMsg,0,&smbios_variant,&type,NULL);
			SysFreeString(bstrMsg);
			if(hr < 0)
			{
				DbgPrintW(L"CheckForSmbiosFlags:: Failed to read SMBIOS major version\n");
				VariantClear(&smbios_variant);
			}
			else
			{
				VariantClear(&smbios_variant);
				VariantInit(&smbios_variant);
				bstrMsg = SysAllocString(L"SmbiosMinorVersion");
				hr = pinst->Get(bstrMsg,0,&smbios_variant,&type,NULL);
				SysFreeString(bstrMsg);
				if(hr < 0)
				{
					DbgPrintW(L"CheckForSmbiosFlags:: Error in reading SMBIOS minor version\n");
					VariantClear(&smbios_variant);
				}
				else
				{
					VariantClear(&smbios_variant);
					VariantInit(&smbios_variant);
					bstrMsg = SysAllocString(L"SMBiosData");
					hr = pinst->Get(bstrMsg,0,&smbios_variant,&type,NULL);
					SysFreeString(bstrMsg);
					if(hr >= 0)
					{
						if ( ( VT_UI1 | VT_ARRAY  ) != smbios_variant.vt )
						{
						}
						else
						{
							SAFEARRAY* plist = NULL;
							plist = V_ARRAY(&smbios_variant);
							unsigned char* smbios_entry = (unsigned char *)plist->pvData;
							unsigned int smbios_len = plist->rgsabound[0].cElements;
							// SMBIOS tables found. Now look for Table 131 & TDT flags
							if (areSmbiosFlagsSet(smbios_entry, smbios_len, &pCapabilities))
							{
								ret = SMBIOS_FOUNDED;
							}
							else
							{
								ret = SMBIOS_NOT_FOUNDED;
							}
						}
					}
					else
					{
						DbgPrintW(L"CheckForSmbiosFlags:: Error in reading SMBIOS data\n");
					}
					VariantClear(&smbios_variant);
				}
			}
			pinst->Release();
			break;
		}

    } while (hr == WBEM_S_NO_ERROR);

	penum->Release();
    psvc->Release();
    ploc->Release();

comfailure:
	if (uninitCom)
	{
		CoUninitialize();
	}

	return ret;
}
#else
const std::string smbios_entry_point("/sys/firmware/dmi/tables/smbios_entry_point");
const std::string smbios_dmi("/sys/firmware/dmi/tables/DMI");
const size_t smbios_table_size = 0x1F;
const size_t smbios_dmi_size_pos = 0x16;

static bool read_file(const std::string &name, std::vector<uint8_t> &data)
{
	bool ret;

	std::ifstream file(name,
		std::ios::in | std::ios::binary | std::ios::ate);
	if (!file)
		return false;
	std::streampos size = file.tellg();
	file.seekg(0, std::ios::beg);
	data.resize(size);
	file.read(reinterpret_cast<char*>(&data[0]), size);
	ret = !file.fail();
	file.close();
	return ret;
}

uint32_t SMBIOS_Reader::CheckForSmbiosFlags()
{
	int ret;

	std::vector<uint8_t> entry_point;
	std::string sm("_SM_");
	if (!read_file(smbios_entry_point, entry_point))
	{
		DbgPrintW(L"CheckForSmbiosFlags:: Failed to read %hs\n", smbios_entry_point.c_str());
		return SMBIOS_FAILURE;
	}

	if (entry_point.size() < smbios_table_size || !std::equal(sm.begin(), sm.end(), entry_point.begin()))
	{
		DbgPrintW(L"CheckForSmbiosFlags:: Not an _SM_\n");
		return SMBIOS_FAILURE;
	}

	uint16_t *len = (uint16_t*)&entry_point[smbios_dmi_size_pos];

	std::vector<uint8_t> dmi;
	if (!read_file(smbios_dmi, dmi))
	{
		DbgPrintW(L"CheckForSmbiosFlags:: Failed to read %hs\n", smbios_dmi.c_str());
		return SMBIOS_FAILURE;
	}

	if (dmi.size() != *len)
	{
		DbgPrintW(L"CheckForSmbiosFlags:: Wrong data size %d != %d\n", dmi.size(), *len);
		return SMBIOS_FAILURE;
	}

	// SMBIOS tables found. Now look for Table 131 & TDT flags
	if (areSmbiosFlagsSet(&dmi[0], *len, &pCapabilities))
	{
		ret = SMBIOS_FOUNDED;
	}
	else
	{
		ret = SMBIOS_NOT_FOUNDED;
	}

	return ret;
}
#endif //WIN32

void testSMBIOS()
{
	SMBIOS_Reader sm_reader;

	DbgPrintW(L"\n CheckSmbiosFlags return %d\n",sm_reader.CheckForSmbiosFlags());

	DbgPrintW(L"\n MEBx version %d.%d.%d.%d\n",sm_reader.pCapabilities.MEBx_Major,sm_reader.pCapabilities.MEBx_Minor,
		sm_reader.pCapabilities.MEBx_Hotfix,sm_reader.pCapabilities.MEBx_Build);
	DbgPrintW(L"\n ME version %d.%d.%d.%d\n",sm_reader.pCapabilities.ME_Major,sm_reader.pCapabilities.ME_Minor,
		sm_reader.pCapabilities.ME_Hotfix,sm_reader.pCapabilities.ME_Build);
	DbgPrintW(L"\n AT capable %d <> AT enrolled %d\n",sm_reader.pCapabilities.AT_Allowed,sm_reader.pCapabilities.AT_Enrolled);
}
