#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int multicastSocket(int family, int recvport, char *multiCastAddr, char *interface, int loopback, int ttl, int recvBuffSize, int sendBuffSize){
    ////////////////////////
    // Only for IPv4 or IPv6
    if (family == AF_INET || family == AF_INET6){
        
        //////////////
        // Open Socket
        int s = socket(family, SOCK_DGRAM, 0);
        if (s < 0){
            perror("Error on openning Socket descriptor");
            return errno;
        }

        ////////////////////////
        //Socket binding to port
        struct sockaddr_in6 addr = {family, htons(recvport)};
        if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
            perror("Error on port binding");
            return errno;
        }

        /////////////
        // If is IPv4
        if (family == AF_INET){

            /////////////////////
            //Add Multicast group
            struct ip_mreq grp = {0};
            inet_pton(AF_INET, multiCastAddr, &grp.imr_multiaddr);
            if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &grp, sizeof(grp)) < 0){
                perror("Error on adding group");
                return errno;
            }
            
            //////////////
            //Set loopback
            if(setsockopt(s, IPPROTO_IP , IP_MULTICAST_LOOP, &loopback, sizeof(loopback)) < 0){
                perror("Error on loopback setting");
                return errno;
            }

            ////////////////////////
            //Set time to live(hops)
            if(setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0){
                perror("Error on ttl setting");
                return errno;
            }
        }else{
            /////////////
            // If is IPv6

            
            /////////////////////
            //Add Multicast group
            struct ipv6_mreq grp = {0};
            inet_pton(AF_INET6, multiCastAddr, &grp.ipv6mr_multiaddr);
            if(setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &grp, sizeof(grp)) < 0){
                perror("Error on adding group");
                return errno;
            }

            //////////////
            //Set loopback
            if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loopback, sizeof(loopback)) < 0){    
                perror("Error on loopback setting");
                return errno;
            }

            ////////////////////////
            //Set time to live(hops)
            if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0){    
                perror("Error on ttl setting");
                return errno;
            }
        }

        //////////////////////
        //Set send buffer size
        if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sendBuffSize, 4) < 0){
            perror("Error on setting send buffer size");
            return errno;
        }

        //////////////////////
        //Set recv buffer size
        if(setsockopt(s, SOL_SOCKET, SO_RCVBUF, &recvBuffSize, 4) < 0){
            perror("Error on setting recv buffer size");
            return errno;
        }

        ///////////////////////
        //Set interface to bind
        if(setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, interface, 4) < 0){
            perror("Error on interface binding");
            return errno;
        }
        // Only Used if you have more than 1 interface up, else comment this option

        return s;
    }
    perror("Invalid family");
    return -1;
}

int main(int argc, char const *argv[]){
  
    //////////
    // Address
    char *multicastaddr = "224.0.0.189";

    ///////////////////
    // Multicast socket
    int mcSocket = multicastSocket(AF_INET, 8888, multicastaddr, "eth0", 0, 1, 1024, 1024);
    if(mcSocket < 0){
        perror("Error opening multicast socket");
        return -1;
    }
        
    ////////////
    // Dest addr 
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;     addr.sin_port = htons(9999);
    inet_pton(AF_INET, multicastaddr, &addr.sin_addr);

    char buffer[512] = {0};  
    
    printf("Client: ");
    fgets(buffer, 512, stdin);
    sendto(mcSocket, buffer, 512, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    

    recv(mcSocket, buffer, 512, 0);
    printf("Server: %s\n", buffer);
    
    close(mcSocket);

    return 0;
}