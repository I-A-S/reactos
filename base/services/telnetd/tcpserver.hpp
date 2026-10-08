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

#include <vector>

class TCPServer;

class TCPServerSessionHandler
{
public:
    virtual ~TCPServerSessionHandler() = default;

    virtual void on_connect() = 0;
    virtual void on_disconnect() = 0;

    virtual void on_data(const uint8_t* data, size_t length) = 0;

protected:
    TCPServer* m_server{};
    SOCKET m_socket{INVALID_SOCKET};

    bool send_data(const void* data, size_t length) { return _internal_send_data(data, length, true) == length; }

private:
    std::vector<uint8_t> m_send_queue{};
    size_t m_send_queue_cursor{};

    void _internal_on_connect(TCPServer* server, SOCKET s);

    size_t _internal_send_data(const void* data, size_t length, bool allow_queue);

    friend class TCPServer;
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
        // +2 because we're gonna store the stop event at index 0, and listen socket and event 
        // at index 1. 
        // handlers array gets 2 wasted slots and sockets array gets 1, however that is worth it 
        // as otherwise indexing is going to be unnecessarily confusing.
        static constexpr DWORD NUM_MAX_ENTRY_COUNT = NUM_MAX_CONCURRENT_SESSIONS + 2;

        // [IANOTE]: For now, NUM_MAX_ENTRY_COUNT should be less than or equal to 
        // WSA_MAXIMUM_WAIT_EVENTS, which is typically 64. For our current ReactOS purposes,
        // this is plenty. Later we can perhaps look into thread managers or different architecture.
        static_assert(NUM_MAX_ENTRY_COUNT <= WSA_MAXIMUM_WAIT_EVENTS,
            "NUM_MAX_ENTRY_COUNT must be less than or equal to WSA_MAXIMUM_WAIT_EVENTS");

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
    };

public:
    template<typename session_handler_type>
    EStatus initialize(uint16_t port, const char* interface_addr = "0.0.0.0");

    void terminate();

    EStatus run();

    BOOL request_stop();

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