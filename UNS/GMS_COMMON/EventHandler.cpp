/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2010-2023 Intel Corporation
 */
#include "EventHandler.h"

int
EventHandler::init (int argc, ACE_TCHAR *argv[])
{
	UNS_DEBUG(L"EventHandler::init %s \n", name().c_str());

	int ret = initSubService(argc, argv);
	if (ret)
		return ret;

	//register to enentManager as subscriber 
	MessageBlockPtr mbPtr(new ACE_Message_Block(), deleteMessageBlockPtr);
	mbPtr->data_block(new SubscribeEventHandler(name(), getFilter()));
	mbPtr->msg_type(MB_SUBSCRIBE_EVENT);
	m_mainService->sendMessage(EVENT_MANAGER,mbPtr);
	return 0;
}


void 
EventHandler::HandleAceMessage(int type, MessageBlockPtr &mbPtr)
{
	UNS_DEBUG(L"EventHandler::HandleAceMessage type:%d, name: %s\n", type, name().c_str());

	switch (type) {
		case MB_SUBSCRIBE_EVENT:
			{
				if (!m_serviceIsClosed)
				{
					UNS_DEBUG(L"%s MB_SUBSCRIBE_EVENT\n",name().c_str());
					startSubService();
				}
			}
			break;
		case MB_UNSUBSCRIBE_EVENT:
			{
				UNS_DEBUG(L"%s MB_UNSUBSCRIBE_EVENT\n",name().c_str());
				UnSubscribeEventHandler *event = dynamic_cast<UnSubscribeEventHandler*>(mbPtr->data_block());
				bool meiEnabled = false;
				if (event != nullptr)
					meiEnabled = event->meiEnabled_;
				PreStop(type, meiEnabled);
				closeSubService();
			}
			break;
		case MB_STOP_SERVICE:
			{
				UNS_DEBUG(L"%s MB_STOP_SERVICE\n",name().c_str());
				StopServiceDataBlock *event = dynamic_cast<StopServiceDataBlock*>(mbPtr->data_block());
				bool meiEnabled = false;
				if (event != nullptr)
					meiEnabled = event->m_meiEnabled;
				MessageBlockPtr unPtr(new ACE_Message_Block(), deleteMessageBlockPtr);
				unPtr->data_block(new UnSubscribeEventHandler(name(),meiEnabled));
				unPtr->msg_type(MB_UNSUBSCRIBE_EVENT);
				unPtr->msg_priority(5);
				m_mainService->sendMessage(EVENT_MANAGER, unPtr);
				m_serviceIsClosed=true;
			}
			break;
		case MB_SUSPEND_SERVICE:
			{
				suspendSubService();
			}
			break;
		default:
			{
				if (!m_serviceIsClosed)
				{
					UNS_DEBUG(L"Handling event\n");
					handle_event(mbPtr);
				}
			}
	}
}

