#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <nonstdlib.h>
#include "cmd_net.h"
#include "lwip/sockets.h"

int cmd_net_netinfo(int argc, char* argv[])
{


	return 0;
}

int cmd_net_dhcpsinfo(int argc, char* argv[])
{
	dhcps_display();
	return 0;
}

char message[1460]="WayneTsai TCP Test\n";
void SNXAPP_SOCKET_TCP_CLIENT_TASK(void *id){
	int sock, ret, on,  snd_len;
	struct sockaddr_in server;
	paramtpStruct *tcp_tpparam;


	dhcp_wait();
	print_msg("TCP client throughput test...\n");
	
	tcp_tpparam = (paramtpStruct *)id;
	print_msg("ip address : %s port :%d\n", tcp_tpparam->ip_addr, tcp_tpparam->port);
	
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		print_msg("Could not create socket\n");
	}
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_family = AF_INET;
	server.sin_port = htons(0);

	ret = bind(sock , (struct sockaddr*)&server, sizeof(server));
	if( ret < 0)
	{
		print_msg("bind error ret = %d\n", ret);
	}else{
		print_msg("bind success\n");
	}


	on = 1;
//	ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));

	if( ret < 0)
	{
		print_msg("setsockopt error\n");
	}else{
		print_msg("setsockopt success\n");
	}

	server.sin_addr.s_addr = inet_addr(tcp_tpparam->ip_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons( tcp_tpparam->port );

	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		print_msg("connect failed. Error");
		vTaskDelete(NULL);
	}else{
		//	fcntl(sock, F_SETFL, O_NONBLOCK);

		while(1)
		{
			//Send some data
			snd_len = send(sock , message , sizeof(message), 0);
			//if( send(sock , message , sizeof(message), 0) < 0)
			if( snd_len < 0)
			{
				print_msg("Send failed err =%d \n",snd_len);
				close(sock);
				vTaskDelete(NULL);
			}else if(snd_len!= sizeof(message)){
				print_msg("snd_len = %d \n", snd_len);
			}else if(snd_len==0){
				close(sock);
				vTaskDelete(NULL);
			}
			

		}
	}
}

void SNXAPP_SOCKET_UDP_CLIENT_TASK(void *id){
	int socket_fd;
	struct sockaddr_in sa, ra;
	paramtpStruct *udp_tpparam;

	int sent_data; 
	/* Creates an UDP socket (SOCK_DGRAM) with Internet Protocol Family (PF_INET).
	 * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	 */
	
	dhcp_wait();
	print_msg("UDP client throughput test...\n");
	udp_tpparam = (paramtpStruct *)id;
	print_msg("ip address : %s port :%d\n", udp_tpparam->ip_addr, udp_tpparam->port);
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	if ( socket_fd < 0 )
	{

		print_msg("socket call failed");
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(8383);

	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == -1)
	{
		print_msg("Bind to Port Number %d ,IP address %s failed\n",3838,IP_ADDR_ANY);
		close(socket_fd);
	}

	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr =inet_addr(udp_tpparam->ip_addr);
	ra.sin_port = htons(udp_tpparam->port);


	while(1){
		sent_data = sendto(socket_fd, message,sizeof(message),0,(struct sockaddr*)&ra,sizeof(ra));
		if(sent_data < 0)
		{
			print_msg("send failed\n");
			close(socket_fd);
		} 
	}
	close(socket_fd);
}
