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

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <thrift/protocol/TProtocol.h>
#include <thrift/asyn_asio/TAsioServer.h>
#include <thrift/async/TAsyncBufferProcessor.h>
#include <thrift/transport/TBufferTransports.h>
#include <evhttp.h>

#include <iostream>

#ifndef HTTP_INTERNAL // libevent < 2
#define HTTP_INTERNAL 500
#endif

using apache::thrift::transport::TMemoryBuffer;

namespace apache {
	namespace thrift {
		namespace async_asio {


			struct TAsioServer::RequestContext {

				boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> ibuf;
				boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> obuf;
				boost::shared_ptr< boost::asio::ip::tcp::socket > m_ptr_Socket; 
				int64_t m_uuid; 
				RequestContext(boost::shared_ptr< boost::asio::ip::tcp::socket >  curr_socket
					, char * readbuff, int readbufflen, int64_t m_uuid);
			};


			TAsioServer::TAsioServer(boost::shared_ptr< apache::thrift::async::TAsyncBufferProcessor> processor)
				: processor_(processor)
				, m_ptr_io_service()
				, m_ptr_acceptor()
				, m_ptr_asio_work() 
				, m_ptr_asio_strand() 
				
			{}


			TAsioServer::TAsioServer(boost::shared_ptr<apache::thrift::async::TAsyncBufferProcessor> processor, int port)
				: processor_(processor)
				, m_ptr_io_service()
				, m_ptr_acceptor()
				, m_ptr_asio_work()
				, m_ptr_asio_strand()

			{

				m_ptr_io_service.reset( new boost::asio::io_service() ); 
				m_ptr_asio_work.reset(new boost::asio::io_service::work(*m_ptr_io_service));
				m_ptr_asio_strand.reset(new boost::asio::io_service::strand(*m_ptr_io_service));
				m_ptr_acceptor.reset(new boost::asio::ip::tcp::acceptor(*m_ptr_io_service,
					boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)));
				

		}


			TAsioServer::~TAsioServer() {

				m_ptr_asio_strand.reset(); 
				m_ptr_acceptor.reset(); 
				//m_ptr_asio_work->reset(); 
				m_ptr_asio_work.reset(); 
				m_ptr_io_service.reset(); 

			}


			int TAsioServer::serve() {

				if (m_ptr_asio_strand.get() == nullptr ||
					m_ptr_acceptor.get() == nullptr ||
					m_ptr_asio_work.get() == nullptr ||
					m_ptr_io_service.get() == nullptr)
				{

					throw TException("Unexpected coll to TAsioServer::serve"); 
				}

				const boost::system::error_code ignoer; 
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket; 
				handle_accept(ignoer, curr_socket, true); 

				m_ptr_io_service->run(  );


				return 1 ; 

			}


			TAsioServer::RequestContext::RequestContext(boost::shared_ptr< boost::asio::ip::tcp::socket >  curr_socket 
				, char * readbuff, int readbufflen ,int64_t m_uuid)
				:m_ptr_Socket(curr_socket)
				, ibuf(new TMemoryBuffer((uint8_t *)readbuff, readbufflen, TMemoryBuffer::COPY))
				, obuf(new TMemoryBuffer())
				, m_uuid(m_uuid)

			{
				//m_pios = pios ;
			}


			void TAsioServer::handle_accept(const boost::system::error_code& error , 
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket  ,  bool first )
			{

				if (error)
				{
					boost::system::error_code ignoer; 
					curr_socket->close(ignoer);
					return; 
				}
				if (first == false )
				{

					boost::shared_array<char> body_buff(new char[4]);
					boost::asio::async_read(*curr_socket, boost::asio::buffer(body_buff.get(), 4),
						boost::bind(&TAsioServer::recive_data_head, this, body_buff, curr_socket,
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

					//boost::thread  curr_thread (boost::bind(&TAsioServer::process, this, curr_socket)); 
					//curr_thread.detach(); 
				}


				boost::shared_ptr<boost::asio::ip::tcp::socket > new_socket(new boost::asio::ip::tcp::socket(*m_ptr_io_service));

				m_ptr_acceptor->async_accept(*new_socket, boost::bind(&TAsioServer::handle_accept, this, boost::asio::placeholders::error, new_socket, false));


				return; 
			}


			void TAsioServer::recive_data_head(boost::shared_array<char> ptr_buff, 
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, 
				const boost::system::error_code& error, 
				std::size_t bytes_transferred)
			{

				if (error || bytes_transferred != 4 )
				{
					boost::system::error_code ignoer;
					curr_socket->close(ignoer);
					return; 
				}
				int bufflen = ntohl(*(int *)(ptr_buff.get()));

				boost::shared_array<char> body_buff(new char[bufflen]);

				boost::asio::async_read(*curr_socket, boost::asio::buffer(body_buff.get(), bufflen) , 
					boost::bind( &TAsioServer::recive_data_body , this , body_buff , curr_socket , 
					boost::asio::placeholders::error , boost::asio::placeholders::bytes_transferred ) ); 

			}

			void TAsioServer::recive_data_body(boost::shared_array<char> ptr_buff, 
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket,
				const boost::system::error_code& error, 
				std::size_t bytes_transferred)
			{
				if (error)
				{
					boost::system::error_code ignoer;
					curr_socket->close(ignoer);
					return;
				}


				int64_t uuid = 0;

				char * readbuff = ptr_buff.get();
				uuid = (int64_t)ntohll(*(int64_t *)(readbuff));

				boost::shared_ptr<  RequestContext > ptr_Requet(new RequestContext(curr_socket, readbuff + 8, bytes_transferred - 8, uuid) );



				boost::thread  curr_thread(boost::bind(&TAsioServer::process, this, curr_socket, ptr_Requet ));
				curr_thread.detach(); 





				boost::shared_array<char> body_buff(new char[4]);
				boost::asio::async_read(*curr_socket, boost::asio::buffer(body_buff.get(), 4),
					boost::bind(&TAsioServer::recive_data_head, this, body_buff, curr_socket,
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));


			}

			void TAsioServer::process(boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, boost::shared_ptr<  RequestContext > ctx)
			{
				

				return processor_->process(
					apache::thrift::stdcxx::bind(
					&TAsioServer::complete,
					this,
					ctx,
					apache::thrift::stdcxx::placeholders::_1),
					ctx->ibuf,
					ctx->obuf);

				//boost::shared_array<char> body_buff(new char[4]);
				//boost::asio::async_read(*curr_socket, boost::asio::buffer(body_buff.get(), 4 ),
				//	boost::bind(&TAsioServer::recive_data_head, this, body_buff, curr_socket,
				//	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}



			void TAsioServer::send_data_asio(boost::shared_array<char> ptr_buff, boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket, const boost::system::error_code& error, std::size_t bytes_transferred)
			{
				if (error)
				{ 
					std::cerr << "Sending data had a error for asio . " << std::endl; 
					curr_socket->close();
				}
				//curr_socket->close(); 
			}


			void TAsioServer::complete( boost::shared_ptr<  RequestContext > ctx, bool success) {
				(void)success;
				//std::auto_ptr<RequestContext> ptr(ctx);

				uint8_t* obuf;
				uint32_t sz;
				ctx->obuf->getBuffer(&obuf, &sz);

				boost::shared_array<char> outbuff(new  char[sz+ 4 + 8 ]);

				char *c_outbuff = outbuff.get(); 
				*(int *)(c_outbuff) = htonl(sz + 8 );
				*(int64_t *)(c_outbuff + 4) = (int64_t)htonll(ctx->m_uuid);
				std::memcpy(c_outbuff + 4 + 8 , obuf, sz  );
			

				boost::asio::async_write(*ctx->m_ptr_Socket, boost::asio::buffer(outbuff.get(), sz + 4 + 8 ),
					m_ptr_asio_strand->wrap( 
					boost::bind(&TAsioServer::send_data_asio, this, outbuff, ctx->m_ptr_Socket
					, boost::asio::placeholders::error , boost::asio::placeholders::bytes_transferred ) 
					));



			}


			struct event_base* TAsioServer::getEventBase() {
				return eb_;
			}


		}
	}
} // apache::thrift::async
