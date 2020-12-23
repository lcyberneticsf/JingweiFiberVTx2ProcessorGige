#ifndef TC358746_H
#define TC358746_H

int par_init(void);

int init_video_dev(void);

int stop_video_dev(void);

int get_one_frame(void* pdata, unsigned int timeou_ms);

#endif