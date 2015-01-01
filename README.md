thrift-asio-sc
==============

警告：
分享一个业余时间做的例子，完全是交流经验互相学习。
很不完善，除非你知道你在做什么，不要在正式环境使用。

使用boost asio 重新实现异步通信的thrift 0.9.2  ，实现了异步消息队列功能。 

注意：

这个版本的thrift编译完成后，会多出一个动态库：libthriftasio.so  

使用asio的异步通信框架需要连接下面动态库:
		libthrift.so libthriftnb.so libthriftasio.so

接口文件生产时需要选项:

thrift --gen cpp:cob_style 	tutorial.thrift


客户端例子：
/////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>  
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
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
		, mysemaphore(1)
	{ };
	virtual void completed__(bool success)
	{
		if (success)
		{
			printf("completed \n");   // 输出返回结果
			std::cout << res << std::endl;
		}
		else
			printf("failed to respone\n");
		
		fflush(0);
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

static void sendString(test_Shared& client)
{
	printf("sendString start\n");
	tcxx::function<void(SharedServiceCobClient* client)> cob = bind(&my_recv_sendString, _1);
	client.getStruct(cob, client.send_key); // 发送并注册回调函数
	printf("sendString end\n");
}



void DoSimpleTest(const std::string& host, int port)
{
	printf("running DoSimpleTest( %s, %d) ...\n", host.c_str(), port);
	boost::shared_ptr< ::apache::thrift::async::TAsyncChannel>  channel1(new ::apache::thrift::async_asio::TAsioClientChannel(host, "/", host.c_str(), port));
	boost::shared_ptr< TBinaryProtocolFactory > ptr_btFactory(new TBinaryProtocolFactory());
	{
		test_Shared client1(channel1, ptr_btFactory.get());
		client1.send_key = 1;
		client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
	}
	std::cout << "start new  quest . " << std::endl;
	{
		test_Shared client1(channel1, ptr_btFactory.get());
		client1.send_key = 2;
		client1.mysemaphore.wait();
		sendString(client1);   // 发送第一个请求
		client1.mysemaphore.wait();
	}
	printf("done DoSimpleTest().\n");
	return;
}
int main(int argc, char* argv[])
{
	string ip = "127.0.0.1";
	DoSimpleTest(ip, 14488);
	return 0;

}

服务端例子：
//////////////////////////////////////////////////////////////////////////////////

#include <thrift/concurrency/ThreadManager.h>
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
	SharedHandle()	{}
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

	boost::shared_ptr<SharedServiceCobSvIf> Shardt(new SharedHandle() );
	boost::shared_ptr<TAsyncProcessor> underlying_pro(new SharedServiceAsyncProcessor( Shardt ) );
	boost::shared_ptr<TAsyncBufferProcessor> processor(new TAsyncProtocolProcessor(underlying_pro, boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory())));
	apache::thrift::async_asio::TAsioServer server(processor, 14488);
	server.serve();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
例子的编译方式间 CMakelists.txt 文件。 



