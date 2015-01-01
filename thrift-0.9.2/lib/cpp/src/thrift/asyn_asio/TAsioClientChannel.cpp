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
#include <sstream>
#include <map>


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

#include <thrift/asyn_asio/TAsioClientChannel.h>
#include <evhttp.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TProtocolException.h>


using namespace apache::thrift::protocol;
using apache::thrift::transport::TTransportException;

namespace apache {
	namespace thrift {
		namespace async_asio {


			TAsioClientChannel::TAsioClientChannel(
				const std::string& host,
				const std::string& path,
				const char* address,
				int port 
				)
				: host_(host)
				, path_(path)
				, recvBuf_(NULL)
				, m_bSockeConnectok(false)
				, m_bToStop(false)
				, uuid_atomic(1)
			{


				m_pService.reset(new boost::asio::io_service );
				m_pWork.reset(new boost::asio::io_service::work(*m_pService));
				m_pstrand.reset(new boost::asio::io_service::strand(*m_pService));

				boost::packaged_task<size_t> pt(boost::bind(&boost::asio::io_service::run , m_pService.get()));
				m_uf = pt.get_future(); 
				m_iothread.reset(new boost::thread(boost::move(pt))); 


				m_ptrSocket.reset(new boost::asio::ip::tcp::socket(*m_pService));
				boost::asio::ip::tcp::endpoint endport(
					boost::asio::ip::address::from_string(host_.c_str()), port);
				boost::system::error_code ignore;
				m_ptrSocket->connect(endport, ignore);
				if (ignore)
				{
					std::cerr << ignore.message() << std::endl;
					m_bSockeConnectok = false;
				}
				else
				{
					m_bSockeConnectok = true;

					boost::shared_array<char> ptrbuff(new char[4]);
					boost::asio::async_read(*m_ptrSocket, boost::asio::buffer(ptrbuff.get(), 4),
						boost::bind(&TAsioClientChannel::recive_data_head, this, ptrbuff, m_ptrSocket, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

				}
			}


			TAsioClientChannel::~TAsioClientChannel() {

				if (m_bToStop == false)
					m_bToStop = true; 

				if (m_ptrSocket.get() != nullptr)
				{
					if (m_ptrSocket->is_open() && m_bSockeConnectok == true)
					{
						boost::system::error_code ignore;
						m_ptrSocket->close(ignore);

					}
				}
				m_ptrSocket.reset(); 
				m_pWork.reset(); 

				m_uf.wait();
				m_iothread->join(); 

				m_pstrand.reset(); 
				m_pService.reset(); 
				m_iothread.reset(); 
				m_iothread.reset(); 

				


			}


			void TAsioClientChannel::sendAndRecvMessage(
				const VoidCallback& cob,
				apache::thrift::transport::TMemoryBuffer* sendBuf,
				apache::thrift::transport::TMemoryBuffer* recvBuf) {
				cob_ = cob;
				recvBuf_ = recvBuf;


				int64_t curr_uuid;

				curr_uuid = uuid_atomic++; 

				{
					boost::lock_guard< boost::mutex > lg(cop_map_mutex);
					cob_map.insert(std::make_pair(curr_uuid, cob_));
				}

				uint8_t* obuf;
				uint32_t sz;
				sendBuf->getBuffer(&obuf, &sz);

				boost::shared_array< char > databuff(new char[sz + 4 + 8 ]);
				char * c_databuff = databuff.get();
				*(int *)(c_databuff) = htonl(sz + 8 );
				*(int64_t *)(c_databuff + 4) = (int64_t)htonll(curr_uuid);
				::std::memcpy(c_databuff + 4 + 8 , obuf, sz);

				boost::asio::async_write(*m_ptrSocket, 
					boost::asio::buffer(c_databuff, sz + 4 + 8 ), m_pstrand->wrap(
					boost::bind(&TAsioClientChannel::send_data_asio, this, databuff, m_ptrSocket, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
					) );


			}


			void TAsioClientChannel::sendMessage(
				const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message) {
				(void)cob;
				(void)message;
				throw TProtocolException(TProtocolException::NOT_IMPLEMENTED,
					"Unexpected call to TAsioClientChannel::sendMessage");
			}


			void TAsioClientChannel::recvMessage(
				const VoidCallback& cob, apache::thrift::transport::TMemoryBuffer* message) {
				(void)cob;
				(void)message;
				throw TProtocolException(TProtocolException::NOT_IMPLEMENTED,
					"Unexpected call to TAsioClientChannel::recvMessage");
			}



			void  TAsioClientChannel::send_data_asio(boost::shared_array<char> ptr_buff,
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket,
				const boost::system::error_code& error, std::size_t bytes_transferred)
			{
				if (error || m_bSockeConnectok == false)
				{
					//cob_();
					return;
				}



			}

			void TAsioClientChannel::recive_data_head(boost::shared_array<char> ptr_buff,
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket,
				const boost::system::error_code& error, std::size_t bytes_transferred)
			{
				if (error || m_bSockeConnectok == false)
				{
					std::cout << error.message() << std::endl; 
					//cob_();
					return;
				}
				int bufflen = ntohl(*(int *)(ptr_buff.get()));
				boost::shared_array<char > bodybuff(new char[bufflen]);
				boost::asio::async_read(*curr_socket, boost::asio::buffer(bodybuff.get(), bufflen),
					boost::bind(&TAsioClientChannel::recive_data_body, this, bodybuff, curr_socket, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

			}

			void TAsioClientChannel::recive_data_body(boost::shared_array<char> ptr_buff,
				boost::shared_ptr<boost::asio::ip::tcp::socket > curr_socket,
				const boost::system::error_code& error, std::size_t bytes_transferred)
			{
				if (error || m_bSockeConnectok == false)
				{
					//cob_();
					return;
				}

				char * readbuff = ptr_buff.get(); 
				int64_t uuid = (int64_t)ntohll(*(int64_t *)readbuff); 

				recvBuf_->resetBuffer((uint8_t *)(readbuff + 8 ), bytes_transferred - 8 , apache::thrift::async::TMemoryBuffer::COPY);

				//cob_();
				{
					boost::lock_guard< boost::mutex > lg(cop_map_mutex);
					std::map< int64_t, VoidCallback >::iterator fit = cob_map.find(uuid);
					if (fit != cob_map.end())
					{
						fit->second();

						cob_map.erase(fit);

					}
				}


				boost::shared_array<char> ptrbuff(new char[4]);
				boost::asio::async_read(*m_ptrSocket, boost::asio::buffer(ptrbuff.get(), 4),
					boost::bind(&TAsioClientChannel::recive_data_head, this, ptrbuff, m_ptrSocket, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));


			}

		}
	}
} // apache::thrift::async
