#include "live_text_osd.h"
#include "sample_osd.h"


#include <stdio.h>
#include <string.h>
#include <time.h>

#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER_IP "192.168.1.192"
#define SERVER_PORT 3000
#define HTTP_BUF_SIZE 512

static time_t last_switch_time = 0;
static char live_text[20] = "Amir";


static int http_get_live_text(char *out,int out_len){
	int sockfd;
	struct sockaddr_in server_addr;
	char send_buf[256];
	char recv_buf[HTTP_BUF_SIZE];
	int recv_len;
	char *body;


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect");
		close(sockfd);
		return -1;
	}

	snprintf(send_buf, sizeof(send_buf), "GET /data HTTP/1.1\r\n" "Host: %s:%d\r\n" "Connection: close\r\n" "\r\n", SERVER_IP);

	send(sockfd, send_buf, strlen(send_buf), 0);

	recv_len = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);

	if(recv_len <= 0){
		perror("recv");
		close(sockfd);
		return -1;
	}

	recv_buf[recv_len] = '\0';
	close(sockfd);

	body = strstr(recv_buf, "\r\n\r\n");
	if(!body){
		fprintf(stderr, "Invalid HTTP response: no header-body separation\r\n");
		return -1;
	}

	body += 4; // Skip the "\r\n\r\n"

	strncpy(out, body, out_len - 1);
	out[out_len - 1] = '\0';

	out[strcspn(out, "\r\n")] = '\0';

	return 0;
}

void updateLiveOsdText(uint8_t c_idx){
	time_t now;
	unsigned int i, j;
	unsigned char input_ascii;
	void *real_addr;
	int width_acc = 0;

	now = time(NULL);

	if(difftime(now, last_switch_time) >= 5){
		
		if(http_get_live_text(live_text, sizeof(live_text)) == 0){
			printf("Updated live text: %s\n", live_text);
		}else{
			printf("Failed to get live text from server, keeping previous text: %s\n", live_text);
		}
		last_switch_time = now;
	}

	memset((void *)p_canvas_attr_live[c_idx].canvas_addr, 0, LIVE_OSD_SIZE_WIDTH * LIVE_OSD_SIZE_HEIGHT * 2);

	for(i=0;i<strlen(live_text);i++){
		input_ascii = live_text[i] -32;
		real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

		for(j=0;(UINT32)j < (osdindex[input_ascii].image_height);j++){
			memcpy((void *)((p_canvas_attr_live[c_idx].canvas_addr) +
			                (j * LIVE_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
			       real_addr, (osdindex[input_ascii].image_width * 2));
			real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
		}
		width_acc = width_acc + osdindex[input_ascii].image_width;
	}

	MPI_updateOsdCanvas(g_handle[c_idx][3]);
}
