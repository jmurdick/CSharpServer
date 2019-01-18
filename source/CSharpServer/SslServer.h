#pragma once

#include "Endpoint.h"
#include "SslContext.h"

#include <server/asio/ssl_server.h>

namespace CSharpServer {

    ref class SslSession;
    ref class SslServer;

    class SslSessionEx : public CppServer::Asio::SSLSession
    {
    public:
        using CppServer::Asio::SSLSession::SSLSession;

        gcroot<SslSession^> root;

        bool Send(const void* buffer, size_t size) override;
        bool Send(const std::string& text) override;

        void onConnected() override;
        void onHandshaked() override;
        void onDisconnected() override;
        void onReceived(const void* buffer, size_t size) override;
        void onSent(size_t sent, size_t pending) override;
        void onEmpty() override;
        void onError(int error, const std::string& category, const std::string& message) override;
    };

    class SslServerEx : public CppServer::Asio::SSLServer
    {
    public:
        using CppServer::Asio::SSLServer::SSLServer;

        gcroot<SslServer^> root;

        std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(std::shared_ptr<SSLServer> server) override;

        void onStarted() override;
        void onStopped() override;
        void onConnected(std::shared_ptr<CppServer::Asio::SSLSession>& session) override;
        void onHandshaked(std::shared_ptr<CppServer::Asio::SSLSession>& session) override;
        void onDisconnected(std::shared_ptr<CppServer::Asio::SSLSession>& session) override;
        void onError(int error, const std::string& category, const std::string& message) override;
    };

    //! SSL session
    /*!
        SSL session is used to read and write data from the connected SSL client.
    */
    public ref class SslSession
    {
    public:
        //! Initialize the session with a given server
        /*!
            \param server - Connected server
        */
        SslSession(SslServer^ server);
        ~SslSession() { this->!SslSession(); }

        //! Get the session Id
        property String^ Id { String^ get() { return marshal_as<String^>(_session->get()->id().string()); } }

        //! Get the server
        property SslServer^ Server { SslServer^ get() { return _server; } }

        //! Get the number of bytes pending sent by the session
        property long long BytesPending { long long get() { return (long long)_session->get()->bytes_pending(); } }
        //! Get the number of bytes sent by the session
        property long long BytesSent { long long get() { return (long long)_session->get()->bytes_sent(); } }
        //! Get the number of bytes received by the session
        property long long BytesReceived { long long get() { return (long long)_session->get()->bytes_received(); } }

        //! Get the option: receive buffer size
        property long OptionReceiveBufferSize { long get() { return (long)_session->get()->option_receive_buffer_size(); } }
        //! Get the option: send buffer size
        property long OptionSendBufferSize { long get() { return (long)_session->get()->option_send_buffer_size(); } }

        //! Is the session connected?
        property bool IsConnected { bool get() { return _session->get()->IsConnected(); } }
        //! Is the session handshaked?
        property bool IsHandshaked { bool get() { return _session->get()->IsHandshaked(); } }

        //! Disconnect the session
        /*!
            \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
        */
        bool Disconnect() { return _session->get()->Disconnect(); }

        //! Send data into the session
        /*!
            \param buffer - Buffer to send
            \return 'true' if the data was successfully sent, 'false' if the session is not connected
        */
        bool Send(array<Byte>^ buffer) { return Send(buffer, 0, buffer->Length); }
        //! Send data into the session
        /*!
            \param buffer - Buffer to send
            \param offset - Buffer offset
            \param size - Buffer size
            \return 'true' if the data was successfully sent, 'false' if the session is not connected
        */
        bool Send(array<Byte>^ buffer, long long offset, long long size)
        {
            pin_ptr<Byte> ptr = &buffer[buffer->GetLowerBound(0) + (int)offset];
            return _session->get()->Send(ptr, size);
        }
        //! Send text into the session
        /*!
            \param text - Text string to send
            \return 'true' if the text was successfully sent, 'false' if the session is not connected
        */
        bool Send(String^ text)
        {
            std::string temp = marshal_as<std::string>(text);
            return _session->get()->Send(temp.data(), temp.size());
        }

        //! Setup option: receive buffer size
        /*!
            This option will setup SO_RCVBUF if the OS support this feature.

            \param size - Receive buffer size
        */
        void SetupReceiveBufferSize(long size) { return _session->get()->SetupReceiveBufferSize(size); }
        //! Setup option: send buffer size
        /*!
            This option will setup SO_SNDBUF if the OS support this feature.

            \param size - Send buffer size
        */
        void SetupSendBufferSize(long size) { return _session->get()->SetupSendBufferSize(size); }

    protected:
        //! Handle client connected notification
        virtual void OnConnected() {}
        //! Handle client handshaked notification
        virtual void OnHandshaked() {}
        //! Handle client disconnected notification
        virtual void OnDisconnected() {}

        //! Handle buffer received notification
        /*!
            Notification is called when another chunk of buffer was received from the client.

            \param buffer - Received buffer
            \param size - Received buffer size
        */
        virtual void OnReceived(array<Byte>^ buffer, long long size) {}
        //! Handle buffer sending notification
        /*!
            \param size - Size of send buffer
            \return Allow send flag
        */
        virtual bool OnSending(long long size) { return true; }
        //! Handle buffer sent notification
        /*!
            Notification is called when another chunk of buffer was sent to the client.

            This handler could be used to send another buffer to the client
            for instance when the pending size is zero.

            \param sent - Size of sent buffer
            \param pending - Size of pending buffer
        */
        virtual void OnSent(long long sent, long long pending) {}

        //! Handle empty send buffer notification
        /*!
            Notification is called when the send buffer is empty and ready
            for a new data to send.

            This handler could be used to send another buffer to the client.
        */
        virtual void OnEmpty() {}

        //! Handle error notification
        /*!
            \param error - Error code
            \param category - Error category
            \param message - Error message
        */
        virtual void OnError(int error, String^ category, String^ message) {}

    internal:
        void InternalOnConnected() { OnConnected(); }
        void InternalOnHandshaked() { OnHandshaked(); }
        void InternalOnDisconnected() { OnDisconnected(); }
        void InternalOnReceived(array<Byte>^ buffer, long long size) { OnReceived(buffer, size); }
        bool InternalOnSending(long long sent) { return OnSending(sent); }
        void InternalOnSent(long long sent, long long pending) { OnSent(sent, pending); }
        void InternalOnEmpty() { OnEmpty(); }
        void InternalOnError(int error, String^ category, String^ message) { OnError(error, category, message); }

    protected:
        !SslSession() { _session.Release(); }

    internal:
        SslServer^ _server;
        SslContext^ _context;
        Embedded<std::shared_ptr<SslSessionEx>> _session;
    };

    //! SSL server
    /*!
        SSL server is used to connect, disconnect and manage SSL sessions.
    */
    public ref class SslServer
    {
    public:
        //! Initialize SSL server with a given service, protocol and port number
        /*!
            \param service - Service
            \param context - SSL context
            \param protocol - Protocol type
            \param port - Port number
        */
        SslServer(Service^ service, SslContext^ context, CSharpServer::InternetProtocol protocol, int port);
        //! Initialize SSL server with a given service, IP address and port number
        /*!
            \param service - Service
            \param context - SSL context
            \param address - IP address
            \param port - Port number
        */
        SslServer(Service^ service, SslContext^ context, String^ address, int port);
        //! Initialize SSL server with a given Asio service and UDP endpoint
        /*!
            \param service - Asio service
            \param context - SSL context
            \param endpoint - Server SSL endpoint
        */
        SslServer(Service^ service, SslContext^ context, TcpEndpoint^ endpoint);
        ~SslServer() { this->!SslServer(); }

        //! Get the service
        property Service^ Service { CSharpServer::Service^ get() { return _service; } }

        //! Get the SSL context
        property SslContext^ Context { CSharpServer::SslContext^ get() { return _context; } }

        //! Get the number of sessions connected to the server
        property long long ConnectedSessions { long long get() { return (long long)_server->get()->connected_sessions(); } }
        //! Get the number of bytes pending sent by the server
        property long long BytesPending { long long get() { return (long long)_server->get()->bytes_pending(); } }
        //! Get the number of bytes sent by the server
        property long long BytesSent { long long get() { return (long long)_server->get()->bytes_sent(); } }
        //! Get the number of bytes received by the server
        property long long BytesReceived { long long get() { return (long long)_server->get()->bytes_received(); } }

        //! Get the option: keep alive
        property bool OptionKeepAlive { bool get() { return _server->get()->option_keep_alive(); } }
        //! Get the option: no delay
        property bool OptionNoDelay { bool get() { return _server->get()->option_no_delay(); } }
        //! Get the option: reuse address
        property bool OptionReuseAddress { bool get() { return _server->get()->option_reuse_address(); } }
        //! Get the option: reuse port
        property bool OptionReusePort { bool get() { return _server->get()->option_reuse_port(); } }

        //! Is the server started?
        property bool IsStarted { bool get() { return _server->get()->IsStarted(); } }

        //! Start the server
        /*!
            \return 'true' if the server was successfully started, 'false' if the server failed to start
        */
        bool Start() { return _server->get()->Start(); }
        //! Stop the server
        /*!
            \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
        */
        bool Stop() { return _server->get()->Stop(); }
        //! Restart the server
        /*!
            \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
        */
        bool Restart() { return _server->get()->Restart(); }

        //! Multicast data to all connected sessions
        /*!
            \param buffer - Buffer to multicast
            \return 'true' if the data was successfully multicasted, 'false' if the data was not multicasted
        */
        bool Multicast(array<Byte>^ buffer) { return Multicast(buffer, 0, buffer->Length); }
        //! Multicast data to all connected sessions
        /*!
            \param buffer - Buffer to multicast
            \param offset - Buffer offset
            \param size - Buffer size
            \return 'true' if the data was successfully multicasted, 'false' if the data was not multicasted
        */
        bool Multicast(array<Byte>^ buffer, long long offset, long long size)
        {
            pin_ptr<Byte> ptr = &buffer[buffer->GetLowerBound(0) + (int)offset];
            return (long long)_server->get()->Multicast(ptr, size);
        }
        //! Multicast text to all connected sessions
        /*!
            \param text - Text string to multicast
            \return 'true' if the text was successfully multicasted, 'false' if the text was not multicasted
        */
        bool Multicast(String^ text)
        {
            std::string temp = marshal_as<std::string>(text);
            return (long long)_server->get()->Multicast(temp.data(), temp.size());
        }

        //! Disconnect all connected sessions
        /*!
            \return 'true' if all sessions were successfully disconnected, 'false' if the server is not started
        */
        bool DisconnectAll() { return _server->get()->DisconnectAll(); }

        //! Find a session with a given Id
        /*!
            \param id - Session Id
            \return Session with a given Id or null if the session it not connected
        */
        SslSession^ FindSession(String^ id)
        {
            std::string temp = marshal_as<std::string>(id);
            auto session = _server->get()->FindSession(CppCommon::UUID(temp));
            return session ? std::dynamic_pointer_cast<SslSessionEx>(session)->root : nullptr;
        }

        //! Setup option: keep alive
        /*!
            This option will setup SO_KEEPALIVE if the OS support this feature.

            \param enable - Enable/disable option
        */
        void SetupKeepAlive(bool enable) { return _server->get()->SetupKeepAlive(enable); }
        //! Setup option: no delay
        /*!
            This option will enable/disable Nagle's algorithm for TCP protocol.

            https://en.wikipedia.org/wiki/Nagle%27s_algorithm

            \param enable - Enable/disable option
        */
        void SetupNoDelay(bool enable) { return _server->get()->SetupNoDelay(enable); }
        //! Setup option: reuse address
        /*!
            This option will enable/disable SO_REUSEADDR if the OS support this feature.

            \param enable - Enable/disable option
        */
        void SetupReuseAddress(bool enable) { return _server->get()->SetupReuseAddress(enable); }
        //! Setup option: reuse port
        /*!
            This option will enable/disable SO_REUSEPORT if the OS support this feature.

            \param enable - Enable/disable option
        */
        void SetupReusePort(bool enable) { return _server->get()->SetupReusePort(enable); }

    protected:
        //! Create SSL session factory method
        /*!
            \return SSL session
        */
        virtual SslSession^ CreateSession() { return gcnew SslSession(this); }

    protected:
        //! Handle server started notification
        virtual void OnStarted() {}
        //! Handle server stopped notification
        virtual void OnStopped() {}

        //! Handle session connected notification
        /*!
            \param session - Connected session
        */
        virtual void OnConnected(SslSession^ session) {}
        //! Handle session handshaked notification
        /*!
            \param session - Handshaked session
        */
        virtual void OnHandshaked(SslSession^ session) {}
        //! Handle session disconnected notification
        /*!
            \param session - Disconnected session
        */
        virtual void OnDisconnected(SslSession^ session) {}

        //! Handle error notification
        /*!
            \param error - Error code
            \param category - Error category
            \param message - Error message
        */
        virtual void OnError(int error, String^ category, String^ message) {}

    internal:
        SslSession^ InternalCreateSession() { return CreateSession(); }
        void InternalOnStarted() { OnStarted(); }
        void InternalOnStopped() { OnStopped(); }
        void InternalOnConnected(SslSession^ session) { OnConnected(session); }
        void InternalOnHandshaked(SslSession^ session) { OnHandshaked(session); }
        void InternalOnDisconnected(SslSession^ session) { OnDisconnected(session); }
        void InternalOnError(int error, String^ category, String^ message) { OnError(error, category, message); }

    protected:
        !SslServer() { _server.Release(); }

    internal:
        CSharpServer::Service^ _service;
        CSharpServer::SslContext^ _context;
        Embedded<std::shared_ptr<SslServerEx>> _server;
    };

}
