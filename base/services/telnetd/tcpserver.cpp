/*
 * PROJECT:     ReactOS Telnet Service
 * LICENSE:     GPL-2.0+ (https://spdx.org/licenses/GPL-2.0+)
 * PURPOSE:     Telnet service
 * COPYRIGHT:   Copyright 2026 Aravinda Samarathunga (I-A-S)
 */

#include "tcpserver.hpp"

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(TCPServer);

TCPServer::EStatus TCPServer::initialize()
{
    WSADATA _wsa_data{};
    int result = WSAStartup(MAKEWORD(2,2), &_wsa_data);
    if(result != 0) {
        ERR("winsock init failed with: %d\n", result);
        terminate();
        return EStatus::WSA_INIT_ERROR;
    }
    m_wsa_initialized = true;

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
}

void TCPServer::terminate()
{
    for(DWORD i = 0; i < SessionData::NUM_MAX_ENTRY_COUNT; i++)
    {
        if(m_session_data.sockets[i] != INVALID_SOCKET)
            closesocket(m_session_data.sockets[i]);
        
        if(m_session_data.events[i] = WSA_INVALID_EVENT)
            WSACloseEvent(m_session_data.events[i]);

        if(m_session_data.handlers[i])
        {
            m_session_data.handlers[i]->on_destory();
            delete m_session_data.handlers[i];
        }
    }

    if(m_wsa_initialized) 
    {
        WSACleanup();
        m_wsa_initialized = false;
    }
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