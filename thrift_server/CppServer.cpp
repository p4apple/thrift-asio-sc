/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <thrift/concurrency/ThreadManager.h>
//#include <thrift/concurrency/PosixThreadFactory.h>
//#include <thrift/concurrency/BoostThreadFactory.h>
#include <thrift/concurrency/StdThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/async/TAsyncBufferProcessor.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
//#include <thrift/async/TEvhttpServer.h>
#include <thrift/asyn_asio/TAsioServer.h>

#include <iostream>
#include <stdexcept>
#include <sstream>


#include "../gen-cpp/Calculator.h"
#include "../gen-cpp/SharedService.h"

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace ::apache::thrift::async;

using namespace tutorial;
using namespace shared;



class SharedHandle : public SharedServiceCobSvIf
{

public:
	SharedHandle()
	{
	}

	virtual void getStruct(tcxx::function<void(SharedStruct const& _return)> cob, const int32_t key)
	{
		std::cout << "key" << key  << std::endl; 
		
		SharedStruct coutdata; 
		coutdata.key = key; 
		coutdata.value = "It is test . "; 

		return cob(coutdata );
	}

};


int main(int argc, char **argv) {



#ifdef WIN32
	WORD    version(MAKEWORD(2, 2));
	WSAData data = { 0 };

	int error(WSAStartup(version, &data));
	if (error != 0)
	{
		BOOST_ASSERT(false);
		throw std::runtime_error("Failed to initialise Winsock.");
	}

#endif 

	boost::shared_ptr<SharedServiceCobSvIf> Shardt(new SharedHandle() );
	boost::shared_ptr<TAsyncProcessor> underlying_pro(new SharedServiceAsyncProcessor( Shardt ) );
	boost::shared_ptr<TAsyncBufferProcessor> processor(new TAsyncProtocolProcessor(underlying_pro, boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory())));


	apache::thrift::async_asio::TAsioServer server(processor, 14488);
	server.serve();
	return 0;
}



