#ifndef GNURADAR_TCP_REQUEST_CONNECTION_HPP
#define GNURADAR_TCP_REQUEST_CONNECTION_HPP

#include <stdexcept>
#include <iostream>

#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <gnuradar/CommandList.hpp>

using boost::asio::ip::tcp;

namespace gnuradar{
   namespace network{

      static const int MESSAGE_SIZE_BYTES = 512;

      class TcpConnection
         : public boost::enable_shared_from_this<TcpConnection>
      {
         public:
            typedef boost::shared_ptr<TcpConnection> pointer;
            typedef boost::array< char, MESSAGE_SIZE_BYTES > Message;

            // create and return a shared pointer to this
            static pointer create(boost::asio::io_service& io_service,
                  CommandList& commands)
            {
               return pointer( new TcpConnection(io_service, commands) );
            }

            // return the connection's socket
            tcp::socket& socket()
            {
               return socket_;
            }

            // run thread
            void start()
            {
               Message readMessage;
               std::string result;

               // read incoming message
               size_t readSize = socket_.read_some( 
                     boost::asio::buffer( readMessage ),
                     error_
                     );

               // resize to match actual received message length
               result = readMessage.data();
               result = result.substr(0,readSize);

               ExecuteRequest( result );

               boost::asio::async_write(
                     socket_, 
                     boost::asio::buffer(message_),
                     boost::bind(
                        &TcpConnection::handle_write, 
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred
                        )
                     );
            }

         private:
            CommandList& commands_;
            boost::system::error_code error_;
            tcp::socket socket_;
            std::string message_;

            void ExecuteRequest( std::string& message )
            {

               // FIXME: Remove pseudo args and replace with XML parsed args
               const std::string pseudoArgs = "fake args";

               // TODO: Add XML parser here and pass in XML structure

               // TODO: Add error handling here for unknown command
               try{
               CommandPtr commandPtr = commands_.Find( message );
               commandPtr->Execute( pseudoArgs );
               }
               catch( std::exception& e)
               {
                  std::cerr 
                     << "Invalid command " << message << " found!" 
                     << std::endl;
               }

            }


            TcpConnection(
                  boost::asio::io_service& io_service, CommandList& commands)
               : socket_(io_service), commands_(commands) { }

            void handle_write(const boost::system::error_code&, size_t size) { }
      };
   };
};

#endif