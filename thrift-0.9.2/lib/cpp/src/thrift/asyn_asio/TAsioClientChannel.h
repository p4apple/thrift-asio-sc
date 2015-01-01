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

#ifndef _THRIFT_ASIO_CLIENT_CHANNEL_H_
#define _THRIFT_ASIO_CLIENT_CHANNEL_H_ 1

#include <string>
#include <map>


#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include <thrift/async/TAsyncChannel.h>
#include <thrift/protocol/TProtocol.h>

struct event_base;
struct evhttp_connection;
struct evhttp_request;

namespace apache {
	namespace thrift {
		namespace transport {
			class TMemoryBuffer;
		}
	}
}

namespace apache {
	namespace thrift {
		namespace async_asio {

	

			class TAsioClientChannel : public apache::thrift::async::TAsyncChannel {
			public:
				using TAsyncChannel::VoidCallback;

				TAsioClientChannel(
					const std::string& host,
					const std::string& path,
					const char* address,
					int port 
					);
				~TAsioClientChannel();

				virtual void sendAndRecvMessage(const VoidCallback& cob,
					apache::thrift::transport::TMemoryBuffer* sendBuf,
					apache::thrift::transport::TMemoryBuffer* recvBuf);

				virtual void sendMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message);
				virtual void recvMessage(const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message);

				//XXX
				virtual bool good() const { return true; }
				virtual bool error() const { return false; }
				virtual bool timedOut() const { return false; }

			private:
				static void response(struct evhttp_request* req, void* arg);


				boost::atomic_int64_t uuid_atomic; 
				boost::mutex cop_map_mutex; 

				boost::shared_ptr< boost::asio::io_service > m_pService; 
				boost::shared_ptr< boost::asio::io_service::work > m_pWork; 
				boost::shared_ptr< boost::asio::io_service::strand > m_pstrand; 
				boost::shared_ptr< boost::thread > m_iothread; 
				boost::unique_future<size_t> m_uf; 
	
				boost::shared_ptr< boost::asio::ip::tcp::socket > m_ptrSocket;
				volatile bool m_bSockeConnectok;
				volatile bool m_bToStop; 


				std::string host_;
				std::string path_;
				VoidCallback cob_;

				std::map< int64_t, VoidCallback > cob_map; 

				apache::thrift::transport::TMemoryBuffer* recvBuf_;

				void send_data_asio(boost::shared_array<char> ptr_buff, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, const boost::system::error_code& error, std::size_t bytes_transferred);
				void recive_data_head(boost::shared_array<char> ptr_buff, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, const boost::system::error_code& error, std::size_t bytes_transferred);
				void recive_data_body(boost::shared_array<char> ptr_buff, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, const boost::system::error_code& error, std::size_t bytes_transferred);
			};

		}
	}
} // apache::thrift::async

#endif // #ifndef _THRIFT_TEVHTTP_CLIENT_CHANNEL_H_
