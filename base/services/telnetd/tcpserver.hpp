/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

// [IANOTE]: For this telnet PR, I'll keep the tcp server implementation
// here in telnetd source tree. When we get to SSH, it will be better
// to make this a static helper lib in "sdk/lib" instead.

#pragma once

#include <windef.h>

#define _INC_WINDOWS
#include <winsock2.h>

#include <type_traits>

class TCPServer;

class TCPServerSessionHandler
{
public:
    virtual void on_connect(TCPServer* server, SOCKET s) = 0;
    virtual void on_disconnect() = 0;

    virtual void on_data(const uint8_t* data, size_t length) = 0;
};

class TCPServer
{
public:
    // [IANOTE]: For now, this should be less than or equal to WSA_MAXIMUM_WAIT_EVENTS,
    // which is typically 64. For our current ReactOS purposes, this is plenty.
    // Later we can perhaps look into thread managers or different architecture.
    static constexpr uint32_t NUM_MAX_CONCURRENT_SESSIONS = 32;

    enum class EStatus
    {
        OK,
        SOCKET_CREATE_ERROR,
        SOCKET_BIND_ERROR,
        SOCKET_LISTEN_ERROR,
        SOCKET_ACCEPT_ERROR,
        SOCKET_READ_ERROR,
        SOCKET_WRITE_ERROR,
    };

public:
    EStatus initialize();
    void terminate();

    template<typename session_handler_type>
    EStatus run();

public:
    void send_data_to_socket(SOCKET s, const uint8_t* data, size_t length);

private:
    struct _SessionData {

        WSAEVENT event{NULL};
        SOCKET socket{INVALID_SOCKET};
        TCPServerSessionHandler* handler{};
    } m_session_data[NUM_MAX_CONCURRENT_SESSIONS];
    
    DWORD m_session_count{}; 
};

template<typename session_handler_type>
TCPServer::EStatus TCPServer::run()
{
    static_assert(
        std::is_base_of<TCPServerSessionHandler, session_handler_type>::value, 
        "session_handler_type must implement TCPServerSessionHandler"
    );

    return EStatus::OK;
}
