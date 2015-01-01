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

#ifndef _THRIFT_ASIO_SERVER_H_
#define _THRIFT_ASIO_SERVER_H_ 1

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/asio.hpp>

struct event_base;
struct evhttp;
struct evhttp_request;

namespace apache {
	namespace thrift {
		namespace async {
			class TAsyncBufferProcessor;

		}
	}
}
namespace apache {
	namespace thrift {
		namespace async_asio {

			

			class TAsioServer {
			public:
				/**
				* Create a TAsioServer for use with an external evhttp instance.
				* Must be manually installed with evhttp_set_cb, using
				* TAsioServer::request as the callback and the
				* address of the server as the extra arg.
				* Do not call "serve" on this server.
				*/
				TAsioServer(boost::shared_ptr< apache::thrift::async::TAsyncBufferProcessor> processor);

				/**
				* Create a TAsioServer with an embedded event_base and evhttp,
				* listening on port and responding on the endpoint "/".
				* Call "serve" on this server to serve forever.
				*/
				TAsioServer(boost::shared_ptr<apache::thrift::async::TAsyncBufferProcessor> processor, int port);

				~TAsioServer();

				//static void request(struct evhttp_request* req, void* self);
				void handle_accept(const boost::system::error_code& error, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket,  bool first);
				void recive_data_head(boost::shared_array<char> ptr_buff , boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket , const boost::system::error_code& error, std::size_t bytes_transferred);
				void recive_data_body(boost::shared_array<char> ptr_buff , boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket , const boost::system::error_code& error, std::size_t bytes_transferred);
				int serve();

				struct event_base* getEventBase();
				

			private:
				struct RequestContext;

				//void process(struct evhttp_request* req);
				void process(boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, boost::shared_ptr<  RequestContext > ptr_Requet );
				void complete(boost::shared_ptr<  RequestContext > ctx, bool success);
				void send_data_asio(boost::shared_array<char> ptr_buff, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, const boost::system::error_code& error, std::size_t bytes_transferred); 

				boost::shared_ptr< apache::thrift::async::TAsyncBufferProcessor> processor_;
				struct event_base* eb_;
				struct evhttp* eh_;

				////////////////////////////////////////////////////////////////////
				// add asio 
				//boost::asio::io_service m_io_service; 
				//boost::asio::ip::tcp::acceptor m_acceptor_; 
				//boost::asio::io_service::work m_asio_work; 
				boost::shared_ptr<  boost::asio::io_service  > m_ptr_io_service; 
				boost::shared_ptr< boost::asio::ip::tcp::acceptor > m_ptr_acceptor; 
				boost::shared_ptr< boost::asio::io_service::work > m_ptr_asio_work; 
				boost::shared_ptr< boost::asio::io_service::strand > m_ptr_asio_strand; 

			};

		}
	}
} // apache::thrift::async

#endif // #ifndef _THRIFT_TEVHTTP_SERVER_H_
