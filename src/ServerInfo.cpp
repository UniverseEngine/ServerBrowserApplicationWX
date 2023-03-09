
#include "pch.hpp"

#include "ServerInfo.hpp"

void ServerInfo::Query(const SOCKET& socket)
{
    m_lastPingRecv = Utils::GetTickCount();

    /* socket */
    {
        struct sockaddr_in sendaddr = { AF_INET };
        sendaddr.sin_addr.s_addr    = inet_addr(m_host.m_ip.c_str());
        sendaddr.sin_port           = htons(m_host.m_port);

        char buffer[] = { 'Q', 'U', 'E', 'R', 'Y', 's' };
        sendto(socket, buffer, sizeof(buffer), 0, (sockaddr*)&sendaddr, sizeof(sendaddr));

        char buffer2[] = { 'Q', 'U', 'E', 'R', 'Y', 'p' };
        sendto(socket, buffer2, sizeof(buffer2), 0, (sockaddr*)&sendaddr, sizeof(sendaddr));
    }
}