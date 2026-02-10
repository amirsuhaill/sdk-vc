#include "live_text_osd.h"
#include "sample_osd.h"


#include <stdio.h>
#include <string.h>
#include <time.h>
#include<errno.h>

#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>

#define SERVER_IP "192.168.1.192"
#define SERVER_PORT 3000
#define RECONNECT_INTERVAL 5



static int ws_sock = -1;
static char live_text[20] = "Renu";

static time_t last_reconnect_time = 0;


static int websocket_connect(){
    struct sockaddr_in server_addr;
    char handshake[512];
    char response[512];
    int len;

    ws_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (ws_sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if(connect(ws_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(ws_sock);
        ws_sock = -1;
        return -1;
    }

    snprintf(handshake, sizeof(handshake),
             "GET / HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
             "Sec-WebSocket-Version: 13\r\n"
             "\r\n",
             SERVER_IP, SERVER_PORT);
    send(ws_sock, handshake, strlen(handshake), 0);

    len = recv(ws_sock, response, sizeof(response) - 1, 0);
    if (len <= 0) {
        perror("handshake recv");
        close(ws_sock);
        ws_sock = -1;
        return -1;
    }
    response[len] = '\0';

    if(!strstr(response, "101")){
        fprintf(stderr, "Handshake failed: %s\n", response);
        close(ws_sock);
        ws_sock = -1;
        return -1;
    }

    printf("WebSocket connected successfully\n");
    return 0;
}

static int websocket_recv_text(char *out,int out_len){
    unsigned char hdr[2];
    int payload_len;
    int r;

    if(ws_sock < 0) return -1;

    r = recv(ws_sock, hdr, 2, MSG_DONTWAIT);

    if(r == 0){
        printf("WebSocket connection closed by server\n");
        close(ws_sock);
        ws_sock = -1;
        return -1;
    } 
    else if(r < 0){
        if(errno == EWOULDBLOCK || errno == EAGAIN){
            return -2; // No data available
        }
        close(ws_sock);
        ws_sock = -1;
        return -1;
    }

    payload_len = hdr[1] & 0x7F;

    if(payload_len <= 0 || payload_len >= out_len) return -1;

    r = recv(ws_sock, out, payload_len, 0);

    if(r <= 0){
        close(ws_sock);
        ws_sock = -1;
        return -1;
    }

    out[payload_len] = '\0';
    return 0;
}

void updateLiveOsdText(uint8_t c_idx) {
    time_t now = time(NULL);
    unsigned i,j;
    unsigned char input_ascii = 0;
    void *real_addr = 0;
    int width_acc = 0;

    if(ws_sock < 0){
        if(difftime(now, last_reconnect_time) >= RECONNECT_INTERVAL){
            printf("Attempting to reconnect WebSocket...\n");
            last_reconnect_time = now;
            websocket_connect();
        }
    }

    if(ws_sock >= 0){
        if(websocket_recv_text(live_text, sizeof(live_text)) == 0){
            printf("Received live text: %s\n", live_text);
        }
    }

    memset((void *)p_canvas_attr_live[c_idx].canvas_addr, 0, LIVE_OSD_SIZE_WIDTH * LIVE_OSD_SIZE_HEIGHT * 2);

    for(i = 0;i<strlen(live_text);i++){
        input_ascii = live_text[i] -32;
        real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

        for(j = 0;(UINT32)j < (osdindex[input_ascii].image_height);j++){
            memcpy((void *)((p_canvas_attr_live[c_idx].canvas_addr) + (j * LIVE_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
                   real_addr, (osdindex[input_ascii].image_width * 2));
            real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
        }
        width_acc += osdindex[input_ascii].image_width;
    }
    MPI_updateOsdCanvas(g_handle[c_idx][3]);
}
