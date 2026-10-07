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

#include <stdint.h>

#include <windef.h>
#include <winbase.h>

#define _INC_WINDOWS
#include <winsock2.h>

class TCPServer;

class TCPServerSessionHandler
{
public:
    virtual ~TCPServerSessionHandler() = default;

    virtual void on_connect(TCPServer* server, SOCKET s) = 0;
    virtual void on_disconnect() = 0;

    virtual void on_data(const uint8_t* data, size_t length) = 0;
};

class TCPServer
{
public:
    static constexpr DWORD NUM_MAX_CONCURRENT_SESSIONS = 32;

    enum class EStatus
    {
        OK,
        WSA_INIT_ERROR,
        EVENT_WAIT_ERROR,
        EVENT_CREATE_ERROR,
        EVENT_SELECT_ERROR,
        SOCKET_CREATE_ERROR,
        SOCKET_BIND_ERROR,
        SOCKET_LISTEN_ERROR,
        SOCKET_ACCEPT_ERROR,
        SOCKET_READ_ERROR,
        SOCKET_WRITE_ERROR,
        INTERFACE_ADDR_PARSE_ERROR,
    };

private:
    using HandlerFactoryFuncT = TCPServerSessionHandler* (*)();

    struct SessionData
    {
        // +1 because we're gonna store the listen socket and event at index 0.
        // handlers array gets a +1 too because otherwise indexing is going to
        // be unnecessarily confusing.
        static constexpr DWORD NUM_MAX_ENTRY_COUNT = NUM_MAX_CONCURRENT_SESSIONS + 1;

        // [IANOTE]: For now, NUM_MAX_ENTRY_COUNT should be less than or equal to 
        // WSA_MAXIMUM_WAIT_EVENTS, which is typically 64. For our current ReactOS purposes,
        // this is plenty. Later we can perhaps look into thread managers or different architecture.
        static_assert(NUM_MAX_ENTRY_COUNT <= WSA_MAXIMUM_WAIT_EVENTS,
            "NUM_MAX_ENTRY_COUNT must be less than or equal to WSA_MAXIMUM_WAIT_EVENTS");

        WSAEVENT events[NUM_MAX_ENTRY_COUNT + 1]; // +1 for stop event (at index NUM_MAX_ENTRY_COUNT)
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
            events[NUM_MAX_ENTRY_COUNT] = WSA_INVALID_EVENT;
        }
    };

public:
    template<typename session_handler_type>
    EStatus initialize(uint16_t port, const char* interface_addr = "0.0.0.0");

    void terminate();

    EStatus run();

    BOOL request_stop();

    EStatus send_data_to_socket(SOCKET s, const void* data, DWORD& length);

private:
    SessionData m_session_data{};
    BOOLEAN m_wsa_initialized{false};
    HandlerFactoryFuncT m_handler_factory{};

    EStatus initialize(uint16_t port, const char* interface_addr, HandlerFactoryFuncT handler_factory);
};

template<typename session_handler_type>
TCPServer::EStatus TCPServer::initialize(uint16_t port, const char* interface_addr)
{
    return initialize(port, interface_addr, []() -> TCPServerSessionHandler* { return new session_handler_type(); });
}