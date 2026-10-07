/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "tcpserver.hpp"

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(TCPServer);

TCPServer::EStatus TCPServer::initialize(uint16_t port, const char* interface_addr, HandlerFactoryFuncT handler_factory)
{
    m_handler_factory = handler_factory;

    WSADATA _wsa_data{};
    int result = WSAStartup(MAKEWORD(2,2), &_wsa_data);
    if(result != 0) {
        ERR("winsock init failed with: %d\n", result);
        terminate();
        return EStatus::WSA_INIT_ERROR;
    }
    m_wsa_initialized = true;

    const auto stop_event = WSACreateEvent();
    if(stop_event == WSA_INVALID_EVENT) {
        ERR("stop event create failed with: %d\n", WSAGetLastError());
        terminate();
        return EStatus::EVENT_CREATE_ERROR;
    }
    m_session_data.events[SessionData::NUM_MAX_ENTRY_COUNT] = stop_event;

    const auto listen_event = WSACreateEvent();
    if(listen_event == WSA_INVALID_EVENT) {
        ERR("listen event create failed with: %d\n", WSAGetLastError());
        terminate();
        return EStatus::EVENT_CREATE_ERROR;
    }
    const auto listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listen_socket == INVALID_SOCKET) {
        ERR("listen socket create failed with: %d\n", WSAGetLastError());
        WSACloseEvent(listen_event);
        terminate();
        return EStatus::SOCKET_CREATE_ERROR;
    }
    m_session_data.events[0] = listen_event;
    m_session_data.sockets[0] = listen_socket;
    m_session_data.current_index = 1;

    SOCKADDR_IN sin{};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(interface_addr);
    if(sin.sin_addr.s_addr == INADDR_NONE) {
        ERR("invalid interface address: %s\n", interface_addr);
        terminate();
        return EStatus::INTERFACE_ADDR_PARSE_ERROR;
    }

    if(bind(listen_socket, (PSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
        ERR("listen socket bind failed with: %d\n", WSAGetLastError());
        terminate();
        return EStatus::SOCKET_BIND_ERROR;
    }

    if(listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        ERR("listen socket listen failed with: %d\n", WSAGetLastError());
        terminate();
        return EStatus::SOCKET_LISTEN_ERROR;
    }

    if(WSAEventSelect(listen_socket, listen_event, FD_ACCEPT) == SOCKET_ERROR) {
        ERR("event select failed with: %d\n", WSAGetLastError());
        terminate();
        return EStatus::EVENT_SELECT_ERROR;
    }

    return EStatus::OK;
}

void TCPServer::terminate()
{
    for(DWORD i = 0; i < m_session_data.current_index; i++)
    {
        if(m_session_data.sockets[i] != INVALID_SOCKET)
            closesocket(m_session_data.sockets[i]);
        
        if(m_session_data.events[i] != WSA_INVALID_EVENT)
            WSACloseEvent(m_session_data.events[i]);

        if(m_session_data.handlers[i])
        {
            m_session_data.handlers[i]->on_disconnect();
            delete m_session_data.handlers[i];
        }
    }
    if(m_session_data.events[SessionData::NUM_MAX_ENTRY_COUNT] != WSA_INVALID_EVENT)
        WSACloseEvent(m_session_data.events[SessionData::NUM_MAX_ENTRY_COUNT]);
    m_session_data = {};

    if(m_wsa_initialized) 
    {
        WSACleanup();
        m_wsa_initialized = false;
    }
}

BOOL TCPServer::request_stop()
{
    return WSASetEvent(m_session_data.events[SessionData::NUM_MAX_ENTRY_COUNT]);
}

TCPServer::EStatus TCPServer::send_data_to_socket(SOCKET s, const void* data, DWORD& length)
{
    const char* buffer = (const char*)data;

    while(length > 0)
    {
        const auto result = send(s, buffer, length, 0);

        if(result == SOCKET_ERROR) {
            ERR("socket send failed with: %d\n", WSAGetLastError());
            return EStatus::SOCKET_WRITE_ERROR;
        }

        buffer += result;
        length -= result;
    }

    return EStatus::OK;
}

TCPServer::EStatus TCPServer::run()
{
    CHAR read_buffer[1024];

    while(true)
    {
        auto idx = WSAWaitForMultipleEvents(m_session_data.current_index, m_session_data.events, FALSE, WSA_INFINITE, FALSE);
        if(idx == WSA_WAIT_FAILED) {
            ERR("wsa multi wait failed with: %d\n", WSAGetLastError());
            return EStatus::EVENT_WAIT_ERROR;
        }
        idx -= WSA_WAIT_EVENT_0;

        // This is stop event
        if(idx == SessionData::NUM_MAX_ENTRY_COUNT) break;
        
        for(DWORD i = idx; i < m_session_data.current_index; i++)
        {
            const auto wait_result = WSAWaitForMultipleEvents(1, &m_session_data.events[i], TRUE, 0, FALSE);
            if (
                (wait_result == WSA_WAIT_FAILED) || 
                (wait_result == WSA_WAIT_TIMEOUT)
            )
                continue;

            WSANETWORKEVENTS net_events{};
            WSAEnumNetworkEvents(m_session_data.sockets[i], m_session_data.events[i], &net_events);

            if (!i) {
                if (net_events.lNetworkEvents & FD_ACCEPT)
                {
                    if (net_events.iErrorCode[FD_ACCEPT_BIT] != 0) {
                        ERR("network accept event failed with: %d\n", net_events.iErrorCode[FD_ACCEPT_BIT]);
                        continue;
                    }

                    const auto client_socket = accept(m_session_data.sockets[i], NULL, NULL);
                    if(client_socket == INVALID_SOCKET) {
                        ERR("client socket accept failed with: %d\n", WSAGetLastError());
                        continue;
                    }

                    if(m_session_data.current_index >= SessionData::NUM_MAX_ENTRY_COUNT) {
                        ERR("cannot accept anymore connections.. dropping the latest :((\n");
                        closesocket(client_socket);
                        continue;
                    }

                    const auto client_event = WSACreateEvent();
                    if(client_event == WSA_INVALID_EVENT) {
                        ERR("client event create with: %d\n", WSAGetLastError());
                        closesocket(client_socket);
                        continue;
                    }

                    if(WSAEventSelect(client_socket, client_event, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
                        ERR("client event select with: %d\n", WSAGetLastError());
                        closesocket(client_socket);
                        WSACloseEvent(client_event);
                        continue;
                    }

                    const auto handler = m_handler_factory();

                    handler->on_connect(this, client_socket);

                    m_session_data.events[m_session_data.current_index] = client_event;
                    m_session_data.sockets[m_session_data.current_index] = client_socket;
                    m_session_data.handlers[m_session_data.current_index] = handler;
                    m_session_data.current_index++;
                }

                continue;
            }

            if (net_events.lNetworkEvents & FD_READ)
            {
                if (net_events.iErrorCode[FD_READ_BIT] != 0) {
                    ERR("network read event failed with: %d\n", net_events.iErrorCode[FD_READ_BIT]);
                } else {
                    const auto read_count = recv(m_session_data.sockets[i], read_buffer, 
                            sizeof(read_buffer) - 1, 0);
                    if(read_count > 0) {
                        read_buffer[read_count] = '\0';
                        m_session_data.handlers[i]->on_data((uint8_t*)read_buffer, read_count);
                    }
                }
            }

            if (net_events.lNetworkEvents & FD_CLOSE)
            {
                if (net_events.iErrorCode[FD_CLOSE_BIT] != 0) {
                    ERR("network close event failed with: %d\n", net_events.iErrorCode[FD_CLOSE_BIT]);
                }

                while (true) {
                    const auto read_count = recv(m_session_data.sockets[i], read_buffer, 
                            sizeof(read_buffer) - 1, 0);
                    if(read_count <= 0) break;
                    read_buffer[read_count] = '\0';
                    m_session_data.handlers[i]->on_data((uint8_t*)read_buffer, read_count);
                }

                closesocket(m_session_data.sockets[i]);
                WSACloseEvent(m_session_data.events[i]);
                m_session_data.handlers[i]->on_disconnect();
                delete m_session_data.handlers[i];

                for(DWORD j = i; j < m_session_data.current_index - 1; j++) {
                    m_session_data.sockets[j] = m_session_data.sockets[j + 1];
                    m_session_data.events[j] = m_session_data.events[j + 1];
                    m_session_data.handlers[j] = m_session_data.handlers[j + 1];
                }
                m_session_data.sockets[m_session_data.current_index - 1] = INVALID_SOCKET;
                m_session_data.events[m_session_data.current_index - 1] = WSA_INVALID_EVENT;
                m_session_data.handlers[m_session_data.current_index - 1] = nullptr;

                m_session_data.current_index--;
                i--;

                continue;
            }
        }
    }

    return EStatus::OK;
}

