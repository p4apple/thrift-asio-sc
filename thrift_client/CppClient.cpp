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

#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>  
//#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
//#include <thrift/async/TAsioClientChannel.h>
#include <thrift/asyn_asio/TAsioClientChannel.h>

#include "../gen-cpp/Calculator.h"


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace tutorial;
using namespace shared;




class test_Shared : public SharedServiceCobClient
{
public:
	test_Shared(boost::shared_ptr< ::apache::thrift::async::TAsyncChannel> channel, TProtocolFactory* protocolFactory)
		: SharedServiceCobClient(channel, protocolFactory)
		, mysemaphore( 1 )
	{
		
	};



	virtual void completed__(bool success)
	{
		if (success)
		{
			printf("completed \n" );   // 输出返回结果
			std::cout << res<< std::endl; 
		}
		else
		{
			printf("failed to respone\n");
		}
		fflush(0);

		//itrans_->close();
		//otrans_->close();
		//itrans_->resetBuffer(); 
		//otrans_->resetBuffer(); 
		
	};

	int send_key; 
	SharedStruct res; 
	boost::interprocess::interprocess_semaphore  mysemaphore; 

};


// callback function

static void my_recv_sendString(SharedServiceCobClient* client)
{
	client->recv_getStruct(dynamic_cast<test_Shared *>(client)->res); 
	std::cout << " get result  " << std::endl; 
	dynamic_cast<test_Shared *>(client)->mysemaphore.post();
};

static void sendString(test_Shared& client 	)
{
	printf("sendString start\n");
	tcxx::function<void(SharedServiceCobClient* client)> cob = bind(&my_recv_sendString, _1);
	client.getStruct(cob, client.send_key); // 发送并注册回调函数
	printf("sendString end\n");
}



void DoSimpleTest( const std::string& host, int port 	)
{
	printf("running DoSimpleTest( %s, %d) ...\n",host.c_str(), port);



	boost::shared_ptr< ::apache::thrift::async::TAsyncChannel>  channel1(new ::apache::thrift::async_asio::TAsioClientChannel(host, "/", host.c_str(), port ));
	boost::shared_ptr< TBinaryProtocolFactory > ptr_btFactory(new TBinaryProtocolFactory() );

	{

		test_Shared client1(channel1, ptr_btFactory.get()  );

		client1.send_key = 1; 
		client1.mysemaphore.wait();
		//client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
		//boost::this_thread::sleep_for(boost::chrono::seconds(10));

	}

	std::cout << "start new  quest . " << std::endl;

	{

		test_Shared client1(channel1, ptr_btFactory.get() );
		client1.send_key = 2;
		client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
		//boost::this_thread::sleep_for(boost::chrono::seconds(10));

	}

	std::cout << "start new  quest . " << std::endl;
	{

		test_Shared client1(channel1, ptr_btFactory.get() );
		client1.send_key = 3;
		client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
		//boost::this_thread::sleep_for(boost::chrono::seconds(10));

	}

	std::cout << "start new  quest . " << std::endl;
	{

		test_Shared client1(channel1, ptr_btFactory.get());
		client1.send_key = 4;
		client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		//client1.mysemaphore.wait();
		//sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
		//boost::this_thread::sleep_for(boost::chrono::seconds(10));

	}


	printf("done DoSimpleTest().\n");

	return; 
}



void mytest(  int i )
{
	std::cout <<  " thread test " << i << std::endl; 
}

int main(int argc, char* argv[])
{

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

	string ip = "127.0.0.1"; 
	//boost::thread  mythread( DoSimpleTest ,  ip , 14488 ); 
	//void mytest()
	//boost::thread  mythread(mytest , 1 );
	DoSimpleTest(ip, 14488); 
	//boost::this_thread::sleep_for( boost::chrono::seconds(10) ); 
	//DoSimpleTest("127.0.0.1", 14488);
	return 0;

}
