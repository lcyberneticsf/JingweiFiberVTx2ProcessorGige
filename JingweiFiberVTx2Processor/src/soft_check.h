#pragma once
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <asm/types.h>
#include <thread>
#include <dirent.h>
#include <sys/stat.h>
//该头文件需要放在netlink.h前面防止编译出现__kernel_sa_family未定义
#include <sys/socket.h>  
#include <linux/netlink.h>





#define CHAR_MAX_LEN 256
#define BUF_MAX_LEN INT_MAX

struct usb_st {
    char sn[CHAR_MAX_LEN];
    char product[CHAR_MAX_LEN];
    uint16_t pid;
    uint16_t vid;
};
static int parse_date(char* buf, int len);

void     MonitorNetlinkUevent();

int chk_serial_number(uint16_t pid, uint16_t vid, char* sn);

void t_init();

void t_exit();