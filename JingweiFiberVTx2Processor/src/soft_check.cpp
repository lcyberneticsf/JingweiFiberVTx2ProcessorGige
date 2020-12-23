/*----------------------------------------------------------------
* 项目名称 ：$rootnamespace$
* 项目描述 ：
* 类 名 称 ：$safeitemname$
* 类 描 述 ：
* 命名空间 ：$rootnamespace$
* 作    者 ：hantianyou
* 创建时间 ：$time$
* 更新时间 ：$time$
* 版 本 号 ：v1.0.0.0
*******************************************************************
* Copyright @ hantianyou 2020. All rights reserved.
*******************************************************************
//----------------------------------------------------------------*/
#include "soft_check.h"

static int parse_date(char* buf, int len)
{
    if (strstr(buf, "ACTION=add") != 0
        && strstr(buf, "DEVNAME=/dev/hidraw0") != 0) {
        return 2;//代表插入加密狗
    }
    if (strstr(buf, "ACTION=remove") != 0
        && ((strstr(buf,"DEVNAME=/dev/hidraw0") != 0)||(strstr(buf, "SUBSYSTEM=hidraw") != 0)))
    {
        return 1;//代表拔掉加密狗
    }
    return 0;
}

void MonitorNetlinkUevent()
{
    int sockfd;
    struct sockaddr_nl sa;
    int len;
    char buf[4096];
    struct iovec iov;
    struct msghdr msg;
    int i;

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;//getpid(); both is ok
    memset(&msg, 0, sizeof(msg));
    iov.iov_base = (void*)buf;
    iov.iov_len = sizeof(buf);
    msg.msg_name = (void*)&sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sockfd == -1)
        printf("socket creating failed:%s\n", strerror(errno));
    if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == -1)
        printf("bind error:%s\n", strerror(errno));
    while (1)
    {
        len = recvmsg(sockfd, &msg, 0);
        if (len < 0)
            printf("receive error\n");
        else if (len < 32 || len>sizeof(buf))
            printf("invalid message");
        for (i = 0; i < len; i++)
            if (*(buf + i) == '\0')
                buf[i] = '\n';
        try
        {
            int check_status = parse_date(buf, len);
            printf(buf);
            switch (check_status)
            {
            case 2:break;
            case 1:throw 1; break;
            default:break;
            }
        }
        catch (int)
        {
            perror("please check your softdog.\n");
            _exit(1);
        }
        //printf("received %d bytes\n%s\n", len, buf);
    }

}


/*
return 0 means don't equal;return 1 means equal.
*/
int chk_serial_number(uint16_t pid, uint16_t vid, char* sn) {
    //t_init();
    //usb information index
    //struct libusb_device_descriptor usb_info;
    //usb device
    //struct libusb_device* usb_d = NULL;
    //the handle of the opened usb device
    //struct libusb_device_handle* usb_p = NULL;
    //buffer
    //char buf[CHAR_MAX_LEN] = { 0 };
        try
        {
            while (true)
            {
                t_init();
                struct libusb_device_handle* usb_p = NULL;
                usb_p = libusb_open_device_with_vid_pid(NULL, pid, vid);
                if (usb_p != 0) {
                    //find information index
                    //usb_d = libusb_get_device(usb_p);
                    //if (libusb_get_device_descriptor(usb_d, &usb_info) != 0) {
                    //	perror("can't find usb device's information");
                    //	libusb_close(usb_p);
                    //	return 0;
                    //}
                    ////find SerialNumber
                    //libusb_get_string_descriptor_ascii(usb_p, usb_info.iSerialNumber, buf, CHAR_MAX_LEN);
                    //return (strcmp(buf, sn) == 0);
                    libusb_close(usb_p);
                    t_exit();
                   // return 1;
                }
                else
                {
                    libusb_close(usb_p);
                    t_exit();
                    throw 1;
                   // return 0;
                }
            }

        }
        catch (int)
        {
            perror("can't find usb device");
            _exit(1);
        }
    /*libusb_close(usb_p);
    return 1;*/
    
}

/*
initialization
*/
void t_init() {
    libusb_init(NULL);
}

/*
when app distory
*/
void t_exit() {
    libusb_exit(NULL);
}

