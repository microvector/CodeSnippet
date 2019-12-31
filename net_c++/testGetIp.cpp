#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <net/if.h>  
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>

using namespace std;
in_addr_t get_local_ip()
{
    size_t sfd, intr;
    struct ifreq buf[16];
    struct ifconf ifc;
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        return -1;
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (ioctl(sfd, SIOCGIFCONF, (char *)&ifc))
    {
        return -1;
    }
    intr = ifc.ifc_len / sizeof(struct ifreq);

    while (intr-- > 0 && ioctl(sfd, SIOCGIFADDR, (char *)&buf[intr]))
        ;
    close(sfd);
    in_addr_t ip = ((struct sockaddr_in *)(&buf[intr].ifr_addr))->sin_addr.s_addr;
    return ip;
}

int main()
{
    in_addr *add = new in_addr();
    in_addr_t ip = get_local_ip();
    add->s_addr = ip;
    // inet_ntoa（）函数将传入的Internet数字转换为ASCII表示。返回值是指向包含字符串的内部数组的指针。
    cout << inet_ntoa(*add) << endl;
    return 0;
}