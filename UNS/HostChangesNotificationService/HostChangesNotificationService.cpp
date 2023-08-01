/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2011-2023 Intel Corporation
 */
#include "global.h"

#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "Tools.h"
#include "HostChangesNotificationService.h"
#include <SetHostFQDNCommand.h>

HostChangesNotificationService::HostChangesNotificationService() : m_hostFQDN("")
{
}

int
HostChangesNotificationService::init (int argc, ACE_TCHAR *argv[])
{
	int ret = initSubService(argc, argv);
	if (ret)
		return ret;
	static unsigned long nGetFQDN_Interval = 3600 * GMS_ACE_SECOND;
	ACE_Time_Value interval (nGetFQDN_Interval); 

	/*long timerID = */ACE_Reactor::instance()->schedule_timer (this,
		0,
		ACE_Time_Value::zero,
		interval);

	startSubService();
	return 0;
}

int
HostChangesNotificationService::fini (void)
{
	UNS_DEBUG(L"HostChangesNotificationService service stopped\n");
	ACE_Reactor::instance()->cancel_timer (this);
	return 0;
}

const ACE_TString
HostChangesNotificationService::name()
{
	return GMS_HOSTCHANGESNOTIFICATIONSERVICE;
}


LMS_SUBSERVICE_DEFINE (HOSTCHANGESNOTIFICATIONSERVICE, HostChangesNotificationService)


int HostChangesNotificationService::handle_timeout( const ACE_Time_Value &current_time,const void *arg )
{
	UNS_DEBUG(L"%s service handle timeout\n",name().c_str());

	MessageBlockPtr mbPtr(new ACE_Message_Block(), deleteMessageBlockPtr);
	mbPtr->data_block(new ACE_Data_Block());
	mbPtr->msg_type(MB_TIMER_EXPIRED);
	this->putq(mbPtr->duplicate()); 

	return 0;
}

void 
HostChangesNotificationService::HandleAceMessage(int type, MessageBlockPtr &mbPtr)
{
	UNS_DEBUG(L"HostChangesNotificationService::HandleAceMessage\n");
	switch (type) 
	{
		case MB_TIMER_EXPIRED:
			Action();
			break;
		default:
			GmsSubService::HandleAceMessage(type, mbPtr);
	}
}

void HostChangesNotificationService::Action()
{
	std::string fqdn;
	if (!GetLocalFQDN(fqdn))
	{
		UNS_ERROR(L"Error in setting host FQDN.- not sending HOSTFQDN to FW\n");
		return;
	}
	if (m_hostFQDN != fqdn)
	{
		try {
			Intel::MEI_Client::AMTHI_Client::SetHostFQDNCommand setHostFQDNCommand(fqdn);
			m_hostFQDN = fqdn;
		}
		catch(Intel::MEI_Client::AMTHI_Client::AMTHIErrorException& e)
		{
			int errNo = e.getErr();
			UNS_ERROR(L"Error in SET_HOST_FQDN. Number is: %d\n", errNo);
		}
		catch(Intel::MEI_Client::MEIClientException& e)
		{
			UNS_ERROR(L"HostChangesNotificationService::Action: SetHostFQDNCommand failed: %C\n", e.what());
		}
	}
}

int HostChangesNotificationService::resume()
{
	GmsSubService::resume();
	Action();
	return 0;
}

