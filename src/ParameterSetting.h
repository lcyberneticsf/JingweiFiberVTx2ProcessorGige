#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <string>
#include <string.h>
#include <cstdint>
using namespace std;
class CParameterSetting
{
public:
	CParameterSetting();
	~CParameterSetting();
	std::uint32_t signalling;		//to indicate the command instruction  to send to the NX_box;
	std::uint32_t box_num;          //to choose box
	//[System]
	//char local_ip_2[20];
	std::uint16_t model_select = 1;
	char local_ip[20];// = "192.168.1.11";
	std::uint16_t local_port = 9712;
	char box1_ip[20];// = "192.168.1.11";
	std::uint16_t box1_port = 9712;
	char box2_ip[20];// = "192.168.1.12";
	std::uint16_t box2_port = 9712;
	char electric_board_ip[20];// = "192.168.1.15";
	std::uint16_t electric_board_port = 9712;

	//[CameraSetting]
	char cameraA_ip[20];// = "192.168.1.13";
	char cameraA_mac[20];//="00:01:0D:EF::03:0F"
	std::uint16_t cameraA_port = 9712;
	std::uint16_t cameraA_scan_speed = 10000;
	std::uint16_t cameraA_tirgging_mode = 0;
	std::uint16_t cameraA_exposure_time = 15;
	std::uint16_t cameraA_gain = 2;
	std::uint16_t cameraA_width = 4096;
	std::uint16_t cameraA_height = 100;
	std::uint16_t cameraA_trigger_source = 1;
	char cameraB_ip[20];// = "192.168.1.14";
	char cameraB_mac[20];//="00:01:0D:EF::03:0F"
	std::uint16_t cameraB_port = 9712;
	std::uint16_t cameraB_scan_speed = 10000;
	std::uint16_t cameraB_tirgging_mode = 0;
	std::uint16_t cameraB_exposure_time = 15;
	std::uint16_t cameraB_gain = 2;
	std::uint16_t cameraB_width = 4096;
	std::uint16_t cameraB_height = 100;
	std::uint16_t cameraB_trigger_source = 1;
	char cameraC_ip[20];// = "192.168.1.14";
	char cameraC_mac[20];//="00:01:0D:EF::03:0F"
	std::uint16_t cameraC_port = 9712;
	std::uint16_t cameraC_scan_speed = 10000;
	std::uint16_t cameraC_tirgging_mode = 0;
	std::uint16_t cameraC_exposure_time = 15;
	std::uint16_t cameraC_gain = 2;
	std::uint16_t cameraC_width = 4096;
	std::uint16_t cameraC_height = 100;
	std::uint16_t cameraC_trigger_source = 1;
	char cameraD_ip[20];// = "192.168.1.14";
	char cameraD_mac[20];//="00:01:0D:EF::03:0F"
	std::uint16_t cameraD_port = 9712;
	std::uint16_t cameraD_scan_speed = 10000;
	std::uint16_t cameraD_tirgging_mode = 0;
	std::uint16_t cameraD_exposure_time = 15;
	std::uint16_t cameraD_gain = 2;
	std::uint16_t cameraD_width = 4096;
	std::uint16_t cameraD_height = 100;
	std::uint16_t cameraD_trigger_source = 1;

	//[BaseDetectParameter]
	std::uint16_t cameraA_begin = 100;
	std::uint16_t cameraA_end = 4000;
	std::uint16_t cameraA_align_begin = 100;
	std::uint16_t cameraA_align_end = 4000;
	std::uint16_t cameraA_detect_begin = 400;
	std::uint16_t cameraA_detect_end = 4000;
	std::uint16_t cameraA_valve_amount = 24;
	std::uint16_t cameraB_begin = 100;
	std::uint16_t cameraB_end = 4000;
	std::uint16_t cameraB_align_begin = 100;
	std::uint16_t cameraB_align_end = 4000;
	std::uint16_t cameraB_detect_begin = 400;
	std::uint16_t cameraB_detect_end = 4000;
	std::uint16_t cameraB_valve_amount = 24;
	std::uint16_t cameraC_begin = 100;
	std::uint16_t cameraC_end = 4000;
	std::uint16_t cameraC_align_begin = 100;
	std::uint16_t cameraC_align_end = 4000;
	std::uint16_t cameraC_detect_begin = 400;
	std::uint16_t cameraC_detect_end = 4000;
	std::uint16_t cameraC_valve_amount = 24;
	std::uint16_t cameraD_begin = 100;
	std::uint16_t cameraD_end = 4000;
	std::uint16_t cameraD_align_begin = 100;
	std::uint16_t cameraD_align_end = 4000;
	std::uint16_t cameraD_detect_begin = 400;
	std::uint16_t cameraD_detect_end = 4000;
	std::uint16_t cameraD_valve_amount = 24;

	//[DebugParameter]
	std::uint16_t debug_cameraA = 0;
	std::uint16_t debug_cameraB = 0;
	std::uint16_t debug_cameraC = 0;
	std::uint16_t debug_cameraD = 0;
	std::uint16_t stop_volve = 0;
	std::uint16_t save_blob = 0;

	//[ValveParameter]
	std::uint16_t cameraA_delay = 200;
	std::uint16_t cameraA_hold = 30;
	std::uint16_t cameraB_delay = 200;
	std::uint16_t cameraB_hold = 30;
	std::uint16_t cameraC_delay = 200;
	std::uint16_t cameraC_hold = 30;
	std::uint16_t cameraD_delay = 200;
	std::uint16_t cameraD_hold = 30;
	std::uint16_t valve_work_times = 30;
	std::uint16_t valve_stop_time = 30;

	//[DetectParameter]
	std::uint16_t cameraA_point_area_min = 200;
	std::uint16_t cameraA_point_area_max = 20000;
	std::uint16_t cameraA_point_length_min = 200;
	std::uint16_t cameraA_point_length_max = 20000;
	std::uint16_t cameraA_point_width_min = 200;
	std::uint16_t cameraA_point_width_max = 20000;
	std::uint16_t cameraA_thread_area_min = 200;
	std::uint16_t cameraA_thread_area_max = 20000;
	std::uint16_t cameraA_thread_length_min = 200;
	std::uint16_t cameraA_thread_length_max = 20000;
	std::uint16_t cameraA_thread_width_min = 200;
	std::uint16_t cameraA_thread_width_max = 20000;
	std::uint16_t cameraB_point_area_min = 200;
	std::uint16_t cameraB_point_area_max = 20000;
	std::uint16_t cameraB_point_length_min = 200;
	std::uint16_t cameraB_point_length_max = 20000;
	std::uint16_t cameraB_point_width_min = 200;
	std::uint16_t cameraB_point_width_max = 20000;
	std::uint16_t cameraB_thread_area_min = 200;
	std::uint16_t cameraB_thread_area_max = 20000;
	std::uint16_t cameraB_thread_length_min = 200;
	std::uint16_t cameraB_thread_length_max = 20000;
	std::uint16_t cameraB_thread_width_min = 200;
	std::uint16_t cameraB_thread_width_max = 20000;
	std::uint16_t cameraC_point_area_min = 200;
	std::uint16_t cameraC_point_area_max = 20000;
	std::uint16_t cameraC_point_length_min = 200;
	std::uint16_t cameraC_point_length_max = 20000;
	std::uint16_t cameraC_point_width_min = 200;
	std::uint16_t cameraC_point_width_max = 20000;
	std::uint16_t cameraC_thread_area_min = 200;
	std::uint16_t cameraC_thread_area_max = 20000;
	std::uint16_t cameraC_thread_length_min = 200;
	std::uint16_t cameraC_thread_length_max = 20000;
	std::uint16_t cameraC_thread_width_min = 200;
	std::uint16_t cameraC_thread_width_max = 20000;
	std::uint16_t cameraD_point_area_min = 200;
	std::uint16_t cameraD_point_area_max = 20000;
	std::uint16_t cameraD_point_length_min = 200;
	std::uint16_t cameraD_point_length_max = 20000;
	std::uint16_t cameraD_point_width_min = 200;
	std::uint16_t cameraD_point_width_max = 20000;
	std::uint16_t cameraD_thread_area_min = 200;
	std::uint16_t cameraD_thread_area_max = 20000;
	std::uint16_t cameraD_thread_length_min = 200;
	std::uint16_t cameraD_thread_length_max = 20000;
	std::uint16_t cameraD_thread_width_min = 200;
	std::uint16_t cameraD_thread_width_max = 20000;
};

