/*
 * =====================================================================================
 *
 *       Filename:  fev_listen.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/23/2011 16:47:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <unistd.h>
#include "fev_listen.h"
#include "net_core.h"

#define FEV_LISTEN_QUEUE_NUM    100

struct fev_listen_info {
    int         fd;
    pfev_accept accept_cb;
};

static void on_listen_port(fev_state* fev, int fd, int mask, void* arg)
{
    fev_listen_info* listen_info = (fev_listen_info*)arg;

    while(1) {
        int new_fd = net_accept(listen_info->fd);
        if( new_fd < 0 ) return;

        if ( listen_info->accept_cb )
            listen_info->accept_cb(fev, new_fd);
    }
}

fev_listen_info* fev_add_listener(fev_state* fev, 
        int port, pfev_accept accept_cb)
{
    if( !fev ) return NULL;

    fev_listen_info* listen_info = (fev_listen_info*)malloc(sizeof(fev_listen_info)); 
    if( !listen_info ) return NULL;

    int listen_fd = net_create_listen(NULL, port, FEV_LISTEN_QUEUE_NUM, 0);
    if( listen_fd < 0 ) {
        free(listen_info);
        return NULL;
    }

    listen_info->fd = listen_fd;
    listen_info->accept_cb = accept_cb;

    int ret = fev_reg_event(fev, listen_info->fd, FEV_READ, on_listen_port, NULL, listen_info);
    if( ret < 0 ) {
        free(listen_info);
        return NULL;
    }

    return listen_info;
}

void fev_del_listener(fev_state* fev, fev_listen_info* listen_info)
{
    if( !fev || !listen_info ) return;   

    fev_del_event(fev, listen_info->fd, FEV_READ | FEV_WRITE );
    close(listen_info->fd);
    free(listen_info);
}