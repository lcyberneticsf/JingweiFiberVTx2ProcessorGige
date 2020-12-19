#ifndef GIGEVCAMERA_H
#define GIGEVCAMERA_H

#define MAX_NETIF					8
#define MAX_CAMERAS_PER_NETIF	32
#define MAX_CAMERAS		(MAX_NETIF * MAX_CAMERAS_PER_NETIF)

// Enable/disable Bayer to RGB conversion
// (If disabled - Bayer format will be treated as Monochrome).
#define ENABLE_BAYER_CONVERSION 1

// Set an upper limit on the number of frames in a sequence 
// (so we don't fill the disk)
#define FRAME_SEQUENCE_MAX_COUNT 1024

// Set upper limit on chunk data size in case of problems with device implementation
// (Adjust this if needed).
#define MAX_CHUNK_BYTES	256
// Enable/disable buffer FULL/EMPTY handling (cycling)
#define USE_SYNCHRONOUS_BUFFER_CYCLING	0

// Enable/disable transfer tuning (buffering, timeouts, thread affinity).
#define TUNE_STREAMING_THREADS 1
#define NUM_BUF	8
// IP Config modes.
#define IPCONFIG_MODE_LLA			0x04
#define IPCONFIG_MODE_DHCP			0x02
#define IPCONFIG_MODE_PERSISTENT	0x01

#include "stdio.h"
#include "cordef.h"
#include "GenApi/GenApi.h"		//!< GenApi lib definitions.
#include "gevapi.h"				//!< GEV lib definitions.
#include "gevbuffile.h"
#include "SapX11Util.h"
//#include "X_Display_utils.h"
#include "FileUtil.h"
#include <sched.h>
//#include "VTx2Session.h"
//using namespace Vsee;
//typedef UINT32 DWORD;
int m_getlength(char* buf);
int64_t getCurrentLocalTimeStamp();
unsigned long us_timer_init(void);
unsigned long ms_timer_init(void);
int ms_timer_interval_elapsed(unsigned long origin, unsigned long timeout);
void _GetUniqueFilename(char* filename, size_t size, char* basename);
char GetKey();
void PrintMenu();
int IsTurboDriveAvailable(GEV_CAMERA_HANDLE handle);
void* ImageDisplayThread(void* context);
void* ImageCaptureThread(void* context);
int GetNumber();
static void _GetUniqueFilename_sec(char* filename, size_t size, char* basename);
int GetPixelFormatSelection(GEV_CAMERA_HANDLE handle, int size, char* pixelFormatString);
GEV_STATUS ForceCameraIPAndMode(BOOL persistentIPMode, UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnet_mask);
GEV_STATUS GetCameraIPSettings(GEV_CAMERA_HANDLE& handle, PUINT32 pIPAddress, PUINT32 pSubnet_mask, BOOL* pPersistentIpMode);
GEV_DEVICE_INTERFACE* ListAllCameras(int level);
bool IsIpv4(char* str);
std::string ip_to_hex_str(std::string& ip);
unsigned long  IntToHex(int value);
unsigned long int conv(char ipadr[]);
namespace aq {
	class gigeVCamera
	{
	public:
		typedef struct tagMY_CONTEXT
		{
			//X_VIEW_HANDLE			View;
			GEV_CAMERA_HANDLE		camHandle;
			char* base_name;
			int						depth;
			int 					format;
			int 					enable_sequence;
			int 					enable_save;
			void* convertBuffer;
			int                     camera_serial;
			BOOL					convertFormat;
			BOOL					exit;
		}MY_CONTEXT, * PMY_CONTEXT;

		gigeVCamera();
		void		camera_greetings();
		void		set_camera_options();
		void		discover_camera();
		void		choose_camera(int c_number);
		void		get_feature_node_map();
		void		creat_frame_window();
		void		create_menu();
		void		start_transfer(GEV_CAMERA_HANDLE handle, UINT32 numFrames);
		void		stop_transfer(GEV_CAMERA_HANDLE handle);
		void		about_transfer(GEV_CAMERA_HANDLE handle);
		void		convert_bayer_to_RGB(UINT32 height,UINT32 width,UINT32 convertedFmt,void* m_latestBuffer,UINT32 format,void* bufToSave);
		void		convert_bayer_to_RGB(UINT32 height, UINT32 width, void* m_latestBuffer, void** bufToSave, int format);
		GEV_STATUS	open_camera_by_adress(std::string _ip, GevAccessMode mode, GEV_CAMERA_HANDLE& handle);
		int			create_captrue_thread(std::string _ip, int argc, char* argv[]);
		int			create_captrue_thread(std::string _ip, int argc);
		int			create_captrue_thread(GEV_CAMERA_HANDLE	handle);
		int			close_camera(GEV_CAMERA_HANDLE camera_handle);
		std::vector<std::string> split(std::string str, char seg);
		bool		get_camera_roi_param(GEV_CAMERA_HANDLE handle, int& width, int& height, int& pixel_format);
		bool		set_camera_ip_adress(std::string mac_adress,std::string target_ip,std::string sub_mask,GevAccessMode mode);
		GEV_DEVICE_INTERFACE*	get_all_cameras(int level);
		bool		set_camer_trigger(GEV_CAMERA_HANDLE handle, bool b_trigger);
		GEV_CAMERA_HANDLE  create_camera_handle(std::string _ip, int argc);

		bool		set_camera_roi_param(GEV_CAMERA_HANDLE handle, int width, int height);
		bool		set_camera_width(GEV_CAMERA_HANDLE handle, int width);
		bool		get_camera_width(GEV_CAMERA_HANDLE handle, int& width);
		bool		get_camera_height(GEV_CAMERA_HANDLE handle, int& height);
		bool		set_camera_height(GEV_CAMERA_HANDLE handle, int height);
		bool		set_camera_exposuretime(GEV_CAMERA_HANDLE handle, int exposure_time);
		bool		get_camera_exposuretime(GEV_CAMERA_HANDLE handle, int& exposure_time);
		bool		get_camera_gain(GEV_CAMERA_HANDLE handle, int& gain);
		bool		set_camera_gain(GEV_CAMERA_HANDLE handle, int gain);

	private:
	public:
		int			index_num;   //To indicate the camera index number: 1---CameraA, 2----CameraB, 3-----CameraC ,4-----CameraD;
		int			i;
		int			type;
		UINT32		height;
		UINT32		width;
		UINT32		format;
		UINT32		maxHeight;
		UINT32		maxWidth;
		UINT32		maxDepth;
		UINT64		size;
		UINT64		payload_size;
		int			numBuffers = NUM_BUF;
		PUINT8		bufAddress[NUM_BUF];
		UINT32		pixFormat;
		UINT32		pixDepth;
		UINT32		convertedGevFormat;


		GEV_CAMERA_HANDLE			handle = NULL;
		std::string			_ip;
		UINT32      ExposureTime;
		UINT32      Gain;
		UINT32      AcquisitionLineRate;
		UINT32      Width;
		UINT32      Height;

		GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = { 0 };
		uint32_t			macLow = 0;
		static UINT32		image_index;
		bool			b_camera_open;
		aq::gigeVCamera::MY_CONTEXT context = { 0 };
	};
}


#endif // GIGEVCAMERA_H
