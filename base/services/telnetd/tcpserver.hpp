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
    virtual void on_create(TCPServer* server);
    virtual void on_destory();

    virtual void on_connect(SOCKET s) = 0;
    virtual void on_disconnect() = 0;

    virtual void on_data(const uint8_t* data, size_t length) = 0;
};

class TCPServer
{
public:
    // [IANOTE]: For now, this should be less than or equal to WSA_MAXIMUM_WAIT_EVENTS,
    // which is typically 64. For our current ReactOS purposes, this is plenty.
    // Later we can perhaps look into thread managers or different architecture.
    static constexpr DWORD NUM_MAX_CONCURRENT_SESSIONS = 32;

    enum class EStatus
    {
        OK,
        WSA_INIT_ERROR,
        EVENT_WAIT_ERROR,
        EVENT_CREATE_ERROR,
        SOCKET_CREATE_ERROR,
        SOCKET_BIND_ERROR,
        SOCKET_LISTEN_ERROR,
        SOCKET_ACCEPT_ERROR,
        SOCKET_READ_ERROR,
        SOCKET_WRITE_ERROR,
        INTERFACE_ADDR_PARSE_ERROR,
    };

public:
    EStatus initialize(uint16_t port, const char* interface_addr = "0.0.0.0");
    void terminate();

    template<typename session_handler_type>
    EStatus run();

public:
    EStatus send_data_to_socket(SOCKET s, const void* data, DWORD& length);

private:
    struct SessionData
    {
        // +1 because we're gonna store the listen socket and event at index 0.
        // handlers array gets a +1 too because otherwise indexing is going to
        // be unnecessarily confusing.
        static constexpr DWORD NUM_MAX_ENTRY_COUNT = NUM_MAX_CONCURRENT_SESSIONS + 1;

        WSAEVENT events[NUM_MAX_ENTRY_COUNT];
        SOCKET sockets[NUM_MAX_ENTRY_COUNT];
        TCPServerSessionHandler* handlers[NUM_MAX_ENTRY_COUNT];
        
        DWORD current_index{};

        SessionData() {
            for(DWORD i = 0; i < NUM_MAX_ENTRY_COUNT; i++)
            {
                events[i] = WSA_INVALID_EVENT;
                sockets[i] = INVALID_SOCKET;
                handlers[i] = nullptr;
            }

            current_index = 0;
        }
    } m_session_data{};
    BOOLEAN m_wsa_initialized{false};
};

template<typename session_handler_type>
TCPServer::EStatus TCPServer::run()
{
    static_assert(
        std::is_base_of<TCPServerSessionHandler, session_handler_type>::value, 
        "session_handler_type must implement TCPServerSessionHandler"
    );

    while(true)
    {
        const auto idx = WSAWaitForMultipleEvents(m_session_data.current_index, m_session_data.events, FALSE, WSA_INFINITE, FALSE);

    }

    return EStatus::OK;
}

