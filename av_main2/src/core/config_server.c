#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

#include<pthread.h>
#include "core.h"
#include "nodes.h"


#define PORT 8080
#define BUF_SIZE 2048

extern GlobalConf g_conf;
extern Node g_nodes[NODE_NUM];
extern pthread_mutex_t g_conf_mutex;

static void change_mirror_en(int new_mirr_en){

    if(new_mirr_en != 0 && new_mirr_en != 1){
        printf("Invalid mirr_en value: %d. Must be 0 or 1.\n", new_mirr_en);
        return;
    }

    pthread_mutex_lock(&g_conf_mutex);
    
    // UPdate Global Conf
    for(unsigned int i=0;i<g_conf.strm.video_strm_cnt;i++){
        if(g_conf.strm.video_strm[i].mirr_en == new_mirr_en) continue; // No change needed for this stream
        g_conf.strm.video_strm[i].mirr_en = new_mirr_en;
    }

    // Get the chn node pointer
    Node *chn_node = &g_nodes[CHN];
    int ret = NODES_execRestart(chn_node);
    pthread_mutex_unlock(&g_conf_mutex);

    if(ret != 0){
        printf("Failed to restart CHN node\n");
    }else{
        printf("Successfully restarted CHN node with new mirr_en: %d\n", new_mirr_en);
    }
}


static void change_rotate(int new_rotate){

    if(new_rotate != 0 && new_rotate != 90 && new_rotate != 180 && new_rotate != 270){
        printf("Invalid rotate value: %d. Must be 0, 90, 180, or 270.\n", new_rotate);
        return;
    }

    pthread_mutex_lock(&g_conf_mutex);
    
    // UPdate Global Conf
    for(unsigned int i=0;i<g_conf.strm.video_strm_cnt;i++){
        if(g_conf.strm.video_strm[i].rotate == new_rotate) continue; // No change needed for this stream
        g_conf.strm.video_strm[i].rotate = new_rotate;
    }

    // Get the chn node pointer
    Node *chn_node = &g_nodes[CHN];
    int ret = NODES_execRestart(chn_node);
    pthread_mutex_unlock(&g_conf_mutex);

    if(ret != 0){
        printf("Failed to restart CHN node\n");
    }else{
        printf("Successfully restarted CHN node with new rotate: %d\n", new_rotate);
    }
}

static void change_flip_en(int new_flip_en){

    if(new_flip_en != 0 && new_flip_en != 1){
        printf("Invalid flip_en value: %d. Must be 0 or 1.\n", new_flip_en);
        return;
    }

    pthread_mutex_lock(&g_conf_mutex);
    
    // UPdate Global Conf
    for(unsigned int i=0;i<g_conf.strm.video_strm_cnt;i++){
        if(g_conf.strm.video_strm[i].flip_en == new_flip_en) continue; // No change needed for this stream
        g_conf.strm.video_strm[i].flip_en = new_flip_en;
    }

    // Get the chn node pointer
    Node *chn_node = &g_nodes[CHN];
    int ret = NODES_execRestart(chn_node);
    pthread_mutex_unlock(&g_conf_mutex);

    if(ret != 0){
        printf("Failed to restart CHN node\n");
    }else{
        printf("Successfully restarted CHN node with new flip_en: %d\n", new_flip_en);
    }
}

static int get_json_int(const char *json, const char *key, int *value){
    char *pos = strstr(json, key);
    if(!pos){
        return 0; // Key not found
    }
    pos = strchr(pos, ':');
    if(!pos){
        return 0;
    }
    pos++; // Move past ':'
    return (sscanf(pos,"%d",value) == 1);
}


static void handle_json(const char *json){

    int val;

    if(get_json_int(json, "flip_en", &val)){
        change_flip_en(val);
    }

    if(get_json_int(json, "mirr_en", &val)){
        change_mirror_en(val);
    }

    if(get_json_int(json, "rotate", &val)){
        change_rotate(val);
    }
}

void *config_server_run(){
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    printf("Config server is listening on port %d\n", PORT);

    while(1){
        client_fd = accept(server_fd, NULL, NULL);
        if(client_fd < 0){
            printf("Failed to accept connection\n");
            continue;
        }
        memset(buffer, 0, BUF_SIZE);
        read(client_fd, buffer, BUF_SIZE - 1);

        printf("Received config: \n%s\n", buffer);

        char *body = strstr(buffer, "\r\n\r\n");

        if(body){
            body += 4;
            handle_json(body);
        }else{
            printf("No Http body found\n");
        }

        const char *resp = "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: 2\r\n\r\nOK";

        write(client_fd, resp, strlen(resp));

        close(client_fd);
    }
   
    close(server_fd);
    return NULL;
}
