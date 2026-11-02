#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "attenuator.h"
#include "att_codec.h"
#include "hamlab_version.h"
#include "att_web.h"
static int send_all(int fd, const char *s, size_t n)
{
    while (n) {
        ssize_t sent = zsock_send(fd, s, n, 0);
        if (sent <= 0) return -EIO;
        s += sent; n -= (size_t)sent;
    }
    return 0;
}
static void reply(int fd, int code, const char *type, const char *body)
{
    char h[256];
    const char *reason = code == 200 ? "OK" : code == 404 ? "Not Found" :
        code == 503 ? "Service Unavailable" : "Bad Request";
    int n = snprintf(h, sizeof(h), "HTTP/1.1 %d %s\r\nContent-Type: %s; charset=utf-8\r\n"
        "Content-Length: %zu\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n",
        code, reason, type, strlen(body));
    if (n > 0 && (size_t)n < sizeof(h) && !send_all(fd,h,(size_t)n))
        (void)send_all(fd, body, strlen(body));
}
static void serve(int fd)
{
    struct timeval timeout = {.tv_sec = 3};
    zsock_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    zsock_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    char request[1024], method[8], path[128], version[16], extra;
    size_t used = 0;
    int64_t deadline = k_uptime_get() + 3000;
    for (;;) {
        if (used == sizeof(request)-1 || k_uptime_get() >= deadline) {
            reply(fd,400,"application/json","{\"error\":\"headers too large or slow\"}"); return;
        }
        ssize_t n = zsock_recv(fd, request+used, sizeof(request)-used-1, 0);
        if (n <= 0) return;
        used += (size_t)n; request[used] = 0;
        if (strstr(request, "\r\n\r\n")) break;
    }
    char *eol = strstr(request, "\r\n");
    if (!eol) return;
    *eol = 0;
    if (sscanf(request,"%7s %127s %15s %c",method,path,version,&extra) != 3 ||
        (strcmp(version,"HTTP/1.1") && strcmp(version,"HTTP/1.0"))) {
        reply(fd,400,"application/json","{\"error\":\"invalid request\"}"); return;
    }
    if (!strcmp(method,"GET") && !strcmp(path,"/")) {
        reply(fd,200,"text/html",page); return;
    }
    int rc = 0;
    if (!strcmp(method,"POST") && !strncmp(path,"/api/v1/att/",12)) {
        unsigned db;
        rc = att_parse_db(path+12, &db);
        if (rc) { reply(fd,400,"application/json","{\"error\":\"expected integer 0..127\"}"); return; }
        rc = att_set(db);
    } else if (strcmp(method,"GET") || (strcmp(path,"/api/v1/state") && strcmp(path,"/api/v1/att"))) {
        reply(fd,404,"application/json","{\"error\":\"unknown route\"}"); return;
    }
    char json[256];
    int n = att_json(json, sizeof(json));
    if (n < 0) { reply(fd,503,"application/json","{\"error\":\"status unavailable\"}"); return; }
    reply(fd,rc ? 503 : 200,"application/json",json);
}
static void http_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    for (;;) {
        int fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd < 0) { k_sleep(K_SECONDS(2)); continue; }
        int reuse = 1;
        zsock_setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
        struct sockaddr_in addr = {.sin_family=AF_INET,.sin_port=htons(8080),.sin_addr={.s_addr=INADDR_ANY}};
        if (zsock_bind(fd,(struct sockaddr *)&addr,sizeof(addr)) || zsock_listen(fd,2)) {
            zsock_close(fd); k_sleep(K_SECONDS(2)); continue;
        }
        for (;;) {
            int client = zsock_accept(fd,NULL,NULL);
            if (client < 0) break;
            serve(client); zsock_close(client);
        }
        zsock_close(fd);
    }
}
K_THREAD_DEFINE(att_http_id,4096,http_thread,NULL,NULL,NULL,10,0,0);
