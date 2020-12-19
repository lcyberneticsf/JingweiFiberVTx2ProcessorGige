#include "gigevcamera.h"
//#include "communicator.h"
//#include <opencv2/opencv.hpp>
#include "VTx2Session.h"
#include "opencv2/opencv.hpp"
#include "string.h"
//GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = {0};

int numCamera = 0;

aq::gigeVCamera::MY_CONTEXT m_context = { 0 };
pthread_t  tid;
extern std::queue<Vsee::InferFrame> infer_queue;
 UINT32 aq::gigeVCamera::image_index=0;


int m_getlength(char* buf)
{
	int num = 0;
	for (; *buf != '\0';)
	{
		*buf++;
		num++;
	}
	return num;
}
int64_t getCurrentLocalTimeStamp()
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	return tmp.count();

	// return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
int ArrLength(char* Arr) {
	int i = 0;
	while (Arr[i])
		i++;
	return i;
}

union iptolint
{
	char ip[16];
	unsigned long int n;
};

unsigned long int conv(char ipadr[])
{
	unsigned long int num = 0, val;
	char* tok, * ptr;
	tok = strtok(ipadr, ".");
	while (tok != NULL)
	{
		val = strtoul(tok, &ptr, 0);
		num = (num << 8) + val;
		tok = strtok(NULL, ".");
	}
	return(num);
}

aq::gigeVCamera::gigeVCamera()
{
	b_camera_open = false;
}


unsigned long us_timer_init(void)
{
	struct timeval tm;
	unsigned long msec;

	// Get the time and turn it into a millisecond counter.
	gettimeofday(&tm, NULL);

	msec = (tm.tv_sec * 1000000) + (tm.tv_usec);
	return msec;
}
unsigned long ms_timer_init(void)
{
	struct timeval tm;
	unsigned long msec;

	// Get the time and turn it into a millisecond counter.
	gettimeofday(&tm, NULL);

	msec = (tm.tv_sec * 1000) + (tm.tv_usec / 1000);
	return msec;
}

int ms_timer_interval_elapsed(unsigned long origin, unsigned long timeout)
{
	struct timeval tm;
	unsigned long msec;

	// Get the time and turn it into a millisecond counter.
	gettimeofday(&tm, NULL);

	msec = (tm.tv_sec * 1000) + (tm.tv_usec / 1000);

	// Check if the timeout has expired.
	if (msec > origin)
	{
		return ((msec - origin) >= timeout) ? TRUE : FALSE;
	}
	else
	{
		return ((origin - msec) >= timeout) ? TRUE : FALSE;
	}
}
static void _GetUniqueFilename_sec(char* filename, size_t size, char* basename)
{
	// Create a filename based on the current time (to 1 seconds)
	struct timeval tm;
	uint32_t years, days, hours, seconds;

	if ((filename != NULL) && (basename != NULL))
	{
		if (size > (16 + sizeof(basename)))
		{

			// Get the time and turn it into a 10 msec resolution counter to use as an index.
			gettimeofday(&tm, NULL);
			years = ((tm.tv_sec / 86400) / 365);
			tm.tv_sec = tm.tv_sec - (years * 86400 * 365);
			days = (tm.tv_sec / 86400);
			tm.tv_sec = tm.tv_sec - (days * 86400);
			hours = (tm.tv_sec / 3600);
			seconds = tm.tv_sec - (hours * 3600);

			snprintf(filename, size, "%s_%02d%03d%02d%04d", basename, (years - 30), days, hours, (int)seconds);
		}
	}
}

void _GetUniqueFilename(char* filename, size_t size, char* basename)
{
	// Create a filename based on the current time (to 0.01 seconds)
	struct timeval tm;
	uint32_t years, days, hours, seconds;

	if ((filename != NULL) && (basename != NULL))
	{
		if (size > (16 + sizeof(basename)))
		{

			// Get the time and turn it into a 10 msec resolution counter to use as an index.
			gettimeofday(&tm, NULL);
			years = ((tm.tv_sec / 86400) / 365);
			tm.tv_sec = tm.tv_sec - (years * 86400 * 365);
			days = (tm.tv_sec / 86400);
			tm.tv_sec = tm.tv_sec - (days * 86400);
			hours = (tm.tv_sec / 3600);
			seconds = tm.tv_sec - (hours * 3600);

			snprintf(filename, size, "%s_%03d%02d%04d%02d", basename, days, hours, (int)seconds, (int)(tm.tv_usec / 10000));
		}
	}
}


char GetKey()
{
	char key = getchar();
	while ((key == '\r') || (key == '\n'))
	{
		key = getchar();
	}
	return key;
}

void PrintMenu()
{
	printf("GRAB CTL : [S]=stop, [1-9]=snap N, [G]=continuous, [A]=Abort\n");
	printf("MISC     : [Q]or[ESC]=end,         [T]=Toggle TurboMode (if available), [@]=SaveToFile\n");
}


void* ImageCaptureThread(void* context)
{
	aq::gigeVCamera::MY_CONTEXT* captureContext = (aq::gigeVCamera::MY_CONTEXT*)context;
	aq::gigeVCamera  dalsa_camera;
	void* m_latestBuffer = NULL;

	if (captureContext != NULL)
	{
		int sequence_init = 0;
		unsigned int sequence_count = 0;
		int sequence_index = 0;
		int width;
		int height;
		int channels;
		void* bufToSave;

		FILE* seqFP = NULL;
		size_t len = 0;
		char filename[FILENAME_MAX] = { 0 };
		GEV_BUFFER_OBJECT* img = NULL;
		GEV_STATUS status = 0;

		if (captureContext->base_name != NULL)
		{
			len = strlen(captureContext->base_name);
			strncpy(filename, captureContext->base_name, len);
		}

		// While we are still running.
		while (!captureContext->exit)
		{
			
			// Wait for images to be received
			//status = GevStartTransfer(captureContext->camHandle, -1);
			status = GevWaitForNextImage(captureContext->camHandle, &img, 2000);
			//status = GevGetImage(captureContext->camHandle, &img);
			std::cout << "this is wait status" << status << std::endl;

			if ((img != NULL) && (status == GEVLIB_OK))
			{
				if (img->status == 0)
				{
					m_latestBuffer = img->address;
					std::cout << "M_lastbuffer" << m_latestBuffer << std::endl;
					Vsee::InferFrame infer_frame;
					infer_frame.width = img->w;
					infer_frame.height = img->h;
					//infer_frame.channels = img->;
					infer_frame.label = 1;
					infer_frame.camera_serial = captureContext->camera_serial;
					int data_length = infer_frame.width * infer_frame.height * 3;
					//int m_length = m_getlength((char*)m_latestBuffer);
					//std::cout << "this is data_length" << data_length << std::endl;
					//std::cout << "this is m_get length" << m_length << std::endl;
					if (infer_frame.width == 4096)
					{
						infer_frame.data = new char[data_length];
						bufToSave = m_latestBuffer;
						//dalsa_camera.convert_bayer_to_RGB(infer_frame.height, infer_frame.width, m_latestBuffer, &bufToSave, captureContext->format);
						std::cout << "this is time " << getCurrentLocalTimeStamp() << std::endl;//ms 
						infer_frame.channels = 3;
						infer_frame.label = 1;
						try
						{
							memcpy(infer_frame.data, (char*)bufToSave, data_length);
						}
						catch (...)
						{
							memcpy(infer_frame.data, (char*)bufToSave, strlen((char*)bufToSave));
						}

						//infer_frame.data = (char*)m_latestBuffer;
						if (0)
						{
							cv::Mat m_img(infer_frame.height, infer_frame.width, CV_8UC3, infer_frame.data);
							std::string save_path = std::string("/home/aaeon/Pictures/camera") + std::to_string(aq::gigeVCamera::image_index) + std::string(".bmp");
							aq::gigeVCamera::image_index++;
							cv::imwrite(save_path, m_img);
						}
						
						if (1)
						{
							infer_queue.push(infer_frame);

							//infer_frame.camera_serial = (infer_frame.camera_serial) + 1;
							//infer_queue.push(infer_frame);
						}

						//pthread_exit(0);
						if ((captureContext->enable_sequence) || (sequence_init == 1))
						{
							captureContext->enable_save = FALSE; // Don't do both !!
							if (!sequence_init)
							{
								// Init the capture sequence
								snprintf(&filename[len], (FILENAME_MAX - len - 1), "_seq_%06d.gevbuf", sequence_index++);
								printf("%s\n", filename); //?????
								seqFP = GEVBUFFILE_Create(filename);
								if (seqFP != NULL)
								{
									printf("Store sequence to : %s\n", filename);
									sequence_init = 1;
								}
							}
							if (seqFP != NULL)
							{
								if (GEVBUFFILE_AddFrame(seqFP, img) > 0)
								{
									printf("Add to Sequence : Frame %llu\r", (unsigned long long)img->id); fflush(stdout);
									sequence_count++;
									if (sequence_count > FRAME_SEQUENCE_MAX_COUNT)
									{
										printf("\n Max Sequence Frame Count exceeded - closing sequence file\n");
										captureContext->enable_sequence = 0;
									}
								}
								else
								{
									printf("Add to Sequence : Data Not Saved for Frame %llu\r", (unsigned long long)img->id); fflush(stdout);
								}
							}
							// See if we  are done.
							if (!captureContext->enable_sequence)
							{
								GEVBUFFILE_Close(seqFP, sequence_count);
								printf("Complete sequence : %s has %d frames\n", filename, sequence_count);
								sequence_count = 0;
								sequence_init = 0;
							}

						}

						else if (captureContext->enable_save)
						{
							// Save image (example only).
							// Note : For better performace, some other scheme with multiple 
							//        images (sequence) in a file using file mapping is needed.

							snprintf(&filename[len], (FILENAME_MAX - len - 1), "_%09llu.gevbuf", (unsigned long long)img->id);
							GEVBUFFILE_SaveSingleBufferObject(filename, img);

							// Turn off file save for next frame
							captureContext->enable_save = 0;
							printf("Single frame saved as : %s\n", filename);
							printf("Frame %llu\r", (unsigned long long)img->id); fflush(stdout);
						}
						else
						{
							//printf("chunk_data = %p  : chunk_size = %d\n", img->chunk_data, img->chunk_size); //???????????
							printf("Frame %llu\r", (unsigned long long)img->id); fflush(stdout);
						}
					}

				}
				else
				{
					// Image had an error (incomplete (timeout/overflow/lost)).
					// Do any handling of this condition necessary.
					printf("Frame %llu : Status = %d\n", (unsigned long long)img->id, img->status);
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::microseconds(50));
				continue;
			}
#if(0)
			// See if a sequence in progress needs to be stopped here.
			if ((!captureContext->enable_sequence) && (sequence_init == 1))
			{
				GEVBUFFILE_Close(seqFP, sequence_count);
				printf("Complete sequence : %s has %d frames\n", filename, sequence_count);
				sequence_count = 0;
				sequence_init = 0;
			}
			// Synchonrous buffer cycling (saving to disk takes time).
			if (img != NULL)
			{
				// Release the buffer back to the image transfer process.
				GevReleaseImage(captureContext->camHandle, img);
			}
#endif
		}
	}
	pthread_exit(0);
}

void* ImageDisplayThread(void* context)
{
	aq::gigeVCamera::MY_CONTEXT* displayContext = (aq::gigeVCamera::MY_CONTEXT*)context;
	void* m_latestBuffer = NULL;

	if (displayContext != NULL)
	{
		unsigned long prev_time = 0;
		//unsigned long cur_time = 0;
			//unsigned long deltatime = 0;
		prev_time = us_timer_init();

		// While we are still running.
		while (!displayContext->exit)
		{
			GEV_BUFFER_OBJECT* img = NULL;
			GEV_STATUS status = 0;

			// Wait for images to be received
			status = GevStartTransfer(displayContext->camHandle, -1);
			status = GevWaitForNextImage(displayContext->camHandle, &img, 1000);
			std::cout << "this is status" << status << std::endl;
			if ((img != NULL) && (status == GEVLIB_OK))
			{
				if (img->status == 0)
				{
					m_latestBuffer = img->address;
					// Can the acquired buffer be displayed?
					if (IsGevPixelTypeX11Displayable(img->format) || displayContext->convertFormat)
					{
						// Convert the image format if required.
						if (displayContext->convertFormat)
						{
							int gev_depth = GevGetPixelDepthInBits(img->format);
							// Convert the image to a displayable format.
							//(Note : Not all formats can be displayed properly at this time (planar, YUV*, 10/12 bit packed).
							ConvertGevImageToX11Format(img->w, img->h, gev_depth, img->format, img->address, \
								displayContext->depth, displayContext->format, displayContext->convertBuffer);

							// Display the image in the (supported) converted format.
							//Display_Image( displayContext->View, displayContext->depth, img->w, img->h, displayContext->convertBuffer );
						}
						else
						{
							// Display the image in the (supported) received format.
							//Display_Image( displayContext->View, img->d,  img->w, img->h, img->address );
						}
					}
					else
					{
						//printf("Not displayable\n");
					}
				}
				else
				{
					// Image had an error (incomplete (timeout/overflow/lost)).
					// Do any handling of this condition necessary.
				}
			}
#if USE_SYNCHRONOUS_BUFFER_CYCLING
			if (img != NULL)
			{
				// Release the buffer back to the image transfer process.
				GevReleaseImage(displayContext->camHandle, img);
			}
#endif
		}
	}
	pthread_exit(0);
}

int GetNumber()
{
	char input[MAX_PATH] = { 0 };
	char* nptr = NULL;
	int num;

	scanf("%s", input);
	num = (int)strtol(input, &nptr, 10);

	if (nptr == input)
	{
		return -2;
	}
	return num;
}

int IsTurboDriveAvailable(GEV_CAMERA_HANDLE handle)
{
	int type;
	UINT32 val = 0;

	if (0 == GevGetFeatureValue(handle, "transferTurboCurrentlyAbailable", &type, sizeof(UINT32), &val))
	{
		// Current / Standard method present - this feature indicates if TurboMode is available.
		// (Yes - it is spelled that odd way on purpose).
		return (val != 0);
	}
	else
	{
		// Legacy mode check - standard feature is not there try it manually.
		char pxlfmt_str[64] = { 0 };

		// Mandatory feature (always present).
		GevGetFeatureValueAsString(handle, "PixelFormat", &type, sizeof(pxlfmt_str), pxlfmt_str);

		// Set the "turbo" capability selector for this format.
		if (0 != GevSetFeatureValueAsString(handle, "transferTurboCapabilitySelector", pxlfmt_str))
		{
			// Either the capability selector is not present or the pixel format is not part of the
			// capability set.
			// Either way - TurboMode is NOT AVAILABLE.....
			return 0;
		}
		else
		{
			// The capabilty set exists so TurboMode is AVAILABLE.
			// It is up to the camera to send TurboMode data if it can - so we let it.
			return 1;
		}
	}
	return 0;
}


void aq::gigeVCamera::camera_greetings() {

	// SCHED_RR has fewer side effects.
	// SCHED_OTHER (normal default scheduler) is not too bad afer all.
	if (0)
	{
		//int policy = SCHED_FIFO;
		int policy = SCHED_RR;
		pthread_attr_t attrib;
		int inherit_sched = 0;
		struct sched_param param = { 0 };

		// Set an average RT priority (increase/decrease to tuner performance).
		param.sched_priority = (sched_get_priority_max(policy) - sched_get_priority_min(policy)) / 2;

		// Set scheduler policy
		pthread_setschedparam(pthread_self(), policy, &param); // Don't care if it fails since we can't do anyting about it.

		// Make sure all subsequent threads use the same policy.
		pthread_attr_init(&attrib);
		pthread_attr_getinheritsched(&attrib, &inherit_sched);
		if (inherit_sched != PTHREAD_INHERIT_SCHED)
		{
			inherit_sched = PTHREAD_INHERIT_SCHED;
			pthread_attr_setinheritsched(&attrib, inherit_sched);
		}
	}


	//============================================================================
	// Greetings
	std::cout << "there is a greeeting!! from NX1" << std::endl;
	printf("\nGigE Vision Library GenICam C++ Example Program (%s)\n", __DATE__);
	printf("Copyright (c) 2015, DALSA.\nAll rights reserved.\n\n");

}


void aq::gigeVCamera::set_camera_options() {
	//===================================================================================
	// Set default options for the library.

	GEVLIB_CONFIG_OPTIONS options = { 0 };

	GevGetLibraryConfigOptions(&options);
	//options.logLevel = GEV_LOG_LEVEL_OFF;
	//options.logLevel = GEV_LOG_LEVEL_TRACE;
	options.logLevel = GEV_LOG_LEVEL_NORMAL;
	GevSetLibraryConfigOptions(&options);
	std::cout << "Great,set NX1 Camera" << "  API successfully!" << std::endl;

}


void aq::gigeVCamera::discover_camera()
{
	//====================================================================================
	// DISCOVER Cameras
	//
	// Get all the IP addresses of attached network cards.
	GEV_STATUS status;
	status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);
	printf("%d camera(s) on the network\n", numCamera);

}



void aq::gigeVCamera::choose_camera(int c_number)
{
	// Select the first camera found (unless the command line has a parameter = the camera index)
	int camIndex = 0;
	camIndex = c_number;
	char uniqueName[128];
	GEV_STATUS status;
	if (numCamera != 0)
	{
		if (camIndex >= (int)numCamera)
		{
			printf("Camera index out of range - only %d camera(s) are present\n", numCamera);
			camIndex = -1;
		}
		if (camIndex != -1)
		{
			//====================================================================
			// Connect to Camera
			//
			//
			height = 0;
			width = 0;
			format = 0;
			maxHeight = 1600;
			maxWidth = 2048;
			maxDepth = 2;
			pixFormat = 0;
			pixDepth = 0;
			convertedGevFormat = 0;

			//====================================================================
			// Open the camera.
			status = GevOpenCamera(&pCamera[camIndex], GevExclusiveMode, &handle);
			if (status == 0)
			{
				//=================================================================
				// GenICam feature access via Camera XML File enabled by "open"
				//
				// Get the name of XML file name back (example only - in case you need it somewhere).
				//
				char xmlFileName[MAX_PATH] = { 0 };
				status = GevGetGenICamXML_FileName(handle, (int)sizeof(xmlFileName), xmlFileName);
				if (status == GEVLIB_OK)
				{
					printf("XML stored as %s\n", xmlFileName);
				}
				status = GEVLIB_OK;
			}
			// Get the low part of the MAC address (use it as part of a unique file name for saving images).
			// Generate a unique base name to be used for saving image files
			// based on the last 3 octets of the MAC address.
			macLow = pCamera[camIndex].macLow;
			macLow &= 0x00FFFFFF;
			snprintf(uniqueName, sizeof(uniqueName), "img_%06x", macLow);
			// Go on to adjust some API related settings (for tuning / diagnostics / etc....).
			if (status == 0)
			{
				GEV_CAMERA_OPTIONS camOptions = { 0 };

				// Adjust the camera interface options if desired (see the manual)
				GevGetCameraInterfaceOptions(handle, &camOptions);
				//camOptions.heartbeat_timeout_ms = 60000;		// For debugging (delay camera timeout while in debugger)
				camOptions.heartbeat_timeout_ms = 5000;		// Disconnect detection (5 seconds)

#if TUNE_STREAMING_THREADS
				// Some tuning can be done here. (see the manual)
				camOptions.streamFrame_timeout_ms = 1001;				// Internal timeout for frame reception.
				camOptions.streamNumFramesBuffered = 4;				// Buffer frames internally.
				camOptions.streamMemoryLimitMax = 64 * 1024 * 1024;		// Adjust packet memory buffering limit.
				camOptions.streamPktSize = 9180;							// Adjust the GVSP packet size.
				camOptions.streamPktDelay = 10;							// Add usecs between packets to pace arrival at NIC.

				// Assign specific CPUs to threads (affinity) - if required for better performance.
				{
					int numCpus = _GetNumCpus();
					if (numCpus > 1)
					{
						camOptions.streamThreadAffinity = numCpus - 1;
						camOptions.serverThreadAffinity = numCpus - 2;
					}
				}
#endif
				// Write the adjusted interface options back.
				GevSetCameraInterfaceOptions(handle, &camOptions);

			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}

}


void aq::gigeVCamera::get_feature_node_map()
{
	//=====================================================================
	// Get the GenICam FeatureNodeMap object and access the camera features.
	GEV_STATUS status;
	GenApi::CNodeMapRef* Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));

	if (Camera)
	{
		// Access some features using the bare GenApi interface methods
		try
		{
			//Mandatory features....
			GenApi::CIntegerPtr ptrIntNode = Camera->_GetNode("Width");
			width = (UINT32)ptrIntNode->GetValue();
			ptrIntNode = Camera->_GetNode("Height");
			height = (UINT32)ptrIntNode->GetValue();
			ptrIntNode = Camera->_GetNode("PayloadSize");
			payload_size = (UINT64)ptrIntNode->GetValue();
			GenApi::CEnumerationPtr ptrEnumNode = Camera->_GetNode("PixelFormat");
			format = (UINT32)ptrEnumNode->GetIntValue();
		}
		// Catch all possible exceptions from a node access.
		CATCH_GENAPI_ERROR(status);
	}

	if (status == 0)
	{
		//=================================================================
		// Set up a grab/transfer from this camera
		//
		printf("Camera ROI set for \n\tHeight = %d\n\tWidth = %d\n\tPixelFormat (val) = 0x%08x\n", height, width, format);

		maxHeight = height;
		maxWidth = width;
		maxDepth = GetPixelSizeInBytes(format);

		// Allocate image buffers
		// (Either the image size or the payload_size, whichever is larger - allows for packed pixel formats).
		size = maxDepth * maxWidth * maxHeight;
		size = (payload_size > size) ? payload_size : size;
		for (i = 0; i < numBuffers; i++)
		{
			bufAddress[i] = (PUINT8)malloc(size);
			memset(bufAddress[i], 0, size);
		}

#if USE_SYNCHRONOUS_BUFFER_CYCLING
		// Initialize a transfer with synchronous buffer handling. (tongbu)
		status = GevInitializeTransfer(handle, SynchronousNextEmpty, size, numBuffers, bufAddress);
#else
		// Initialize a transfer with asynchronous buffer handling. (butongbu)
		status = GevInitializeTransfer(handle, Asynchronous, size, numBuffers, bufAddress);
#endif
	}
}


void aq::gigeVCamera::creat_frame_window()
{
	// Create an image display window.
	// This works best for monochrome and RGB. The packed color formats (with Y, U, V, etc..) require
	// conversion as do, if desired, Bayer formats.
	// (Packed pixels are unpacked internally unless passthru mode is enabled).

	// Translate the raw pixel format to one suitable for the (limited) Linux display routines.
	GEV_STATUS status;

	status = GetX11DisplayablePixelFormat(ENABLE_BAYER_CONVERSION, format, &convertedGevFormat, &pixFormat);

	if (format != convertedGevFormat)
	{
		// We MAY need to convert the data on the fly to display it.
		if (GevIsPixelTypeRGB(convertedGevFormat))
		{
			// Conversion to RGB888 required.
			pixDepth = 32;	// Assume 4 8bit components for color display (RGBA)
			m_context.format = Convert_SaperaFormat_To_X11(pixFormat);
			m_context.depth = pixDepth;
			m_context.convertBuffer = malloc((maxWidth * maxHeight * ((pixDepth + 7) / 8)));
			m_context.convertFormat = TRUE;
		}
		else
		{
			// Converted format is MONO - generally this is handled
			// internally (unpacking etc...) unless in passthru mode.
			// (
			pixDepth = GevGetPixelDepthInBits(convertedGevFormat);
			m_context.format = Convert_SaperaFormat_To_X11(pixFormat);
			m_context.depth = pixDepth;
			m_context.convertBuffer = NULL;
			m_context.convertFormat = FALSE;
		}
	}
	else
	{
		pixDepth = GevGetPixelDepthInBits(convertedGevFormat);
		m_context.format = Convert_SaperaFormat_To_X11(pixFormat);
		m_context.depth = pixDepth;
		m_context.convertBuffer = NULL;
		m_context.convertFormat = FALSE;
	}

	// View = CreateDisplayWindow("GigE-V GenApi Console Demo", TRUE, height, width, pixDepth, pixFormat, FALSE );

	 // Create a thread to receive images from the API and display them.
	 //m_context.View = View;
	m_context.camHandle = handle;
	m_context.exit = FALSE;
	pthread_create(&tid, NULL, ImageDisplayThread, &m_context);
}


void aq::gigeVCamera::create_menu()
{
	// Call the main command loop or the example.
	void* m_latestBuffer = NULL;
	char c;
	int done = FALSE;
	int turboDriveAvailable = 0;
	char uniqueName[128];
	GEV_STATUS status;
	PrintMenu();
	while (!done)
	{
		c = GetKey();

		if ((c == 'T') || (c == 't'))	// Toggle turboMode
		{
			// See if TurboDrive is available.
			turboDriveAvailable = IsTurboDriveAvailable(handle);
			if (turboDriveAvailable)
			{
				UINT32 val = 1;
				GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val);
				val = (val == 0) ? 1 : 0;
				GevSetFeatureValue(handle, "transferTurboMode", sizeof(UINT32), &val);
				GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val);
				if (val == 1)
				{
					printf("TurboMode Enabled\n");
				}
				else
				{
					printf("TurboMode Disabled\n");
				}
			}
			else
			{
				printf("*** TurboDrive is NOT Available for this device/pixel format combination ***\n");
			}
		}

		// Stop
		if ((c == 'S') || (c == 's') || (c == '0'))
		{
			GevStopTransfer(handle);
		}
		//Abort
		if ((c == 'A') || (c == 'a'))
		{
			GevAbortTransfer(handle);
		}
		// Snap N (1 to 9 frames)
		if ((c >= '1') && (c <= '9'))
		{
			for (i = 0; i < numBuffers; i++)
			{
				memset(bufAddress[i], 0, size);
			}

			status = GevStartTransfer(handle, (UINT32)(c - '0'));
			if (status != 0) printf("Error starting grab - 0x%x  or %d\n", status, status);
		}
		// Continuous grab.
		if ((c == 'G') || (c == 'g'))
		{
			for (i = 0; i < numBuffers; i++)
			{
				memset(bufAddress[i], 0, size);
			}
			status = GevStartTransfer(handle, -1);
			if (status != 0) printf("Error starting grab - 0x%x  or %d\n", status, status);
		}

		// Save image
		if ((c == '@'))
		{
			char filename[128] = { 0 };
			int ret = -1;
			uint32_t saveFormat = format;
			void* bufToSave = m_latestBuffer;
			int allocate_conversion_buffer = 0;

			// Make sure we have data to save.
			if (m_latestBuffer != NULL)
			{
				uint32_t component_count = 1;
				UINT32 convertedFmt = 0;

				// Bayer conversion enabled for save image to file option.
				//
				// Get the converted pixel type received from the API that is
				//	based on the pixel type output from the camera.
				// (Packed formats are automatically unpacked - unless in "passthru" mode.)
				//
				convertedFmt = GevGetConvertedPixelType(0, format);

				if (GevIsPixelTypeBayer(convertedFmt) && ENABLE_BAYER_CONVERSION)
				{
					int img_size = 0;
					int img_depth = 0;
					uint8_t fill = 0;

					// Bayer will be converted to RGB.
					saveFormat = GevGetBayerAsRGBPixelType(convertedFmt);

					// Convert the image to RGB.
					img_depth = GevGetPixelDepthInBits(saveFormat);
					component_count = GevGetPixelComponentCount(saveFormat);
					img_size = width * height * component_count * ((img_depth + 7) / 8);
					bufToSave = malloc(img_size);
					fill = (component_count == 4) ? 0xFF : 0;  // Alpha if needed.
					memset(bufToSave, fill, img_size);
					allocate_conversion_buffer = 1;

					// Convert the Bayer to RGB
					ConvertBayerToRGB(0, height, width, convertedFmt, m_latestBuffer, saveFormat, bufToSave);

				}
				else
				{
					saveFormat = convertedFmt;
					allocate_conversion_buffer = 0;
				}

				// Generate a file name from the unique base name.
				_GetUniqueFilename(filename, (sizeof(filename) - 5), uniqueName);

#if defined(LIBTIFF_AVAILABLE)
				// Add the file extension we want.
				strncat(filename, ".tif", sizeof(filename));

				// Write the file (from the latest buffer acquired).
				ret = Write_GevImage_ToTIFF(filename, width, height, saveFormat, bufToSave);
				if (ret > 0)
				{
					printf("Image saved as : %s : %d bytes written\n", filename, ret);
				}
				else
				{
					printf("Error %d saving image\n", ret);
				}
#else
				printf("*** Library libtiff not installed ***\n");
#endif
			}
			else
			{
				printf("No image buffer has been acquired yet !\n");
			}

			if (allocate_conversion_buffer)
			{
				free(bufToSave);
			}

		}
		if (c == '?')
		{
			PrintMenu();
		}

		if ((c == 0x1b) || (c == 'q') || (c == 'Q'))
		{
			GevStopTransfer(handle);
			done = TRUE;
			m_context.exit = TRUE;
			pthread_join(tid, NULL);
		}
	}

	GevAbortTransfer(handle);
	status = GevFreeTransfer(handle);
	//DestroyDisplayWindow(View);


	for (i = 0; i < numBuffers; i++)
	{
		free(bufAddress[i]);
	}
	if (m_context.convertBuffer != NULL)
	{
		free(m_context.convertBuffer);
		m_context.convertBuffer = NULL;
	}
	GevCloseCamera(&handle);
	GevApiUninitialize();

	// Close socket API
	_CloseSocketAPI();	// must close API even on error
}

void aq::gigeVCamera::start_transfer(GEV_CAMERA_HANDLE handle, UINT32 numFrames)
{
	GEV_STATUS status;
	if ((numFrames >= 1) && (numFrames <= 9))
	{
		for (i = 0; i < numBuffers; i++)
		{
			memset(bufAddress[i], 0, size);
		}

		status = GevStartTransfer(handle, numFrames);
		if (status != 0) printf("Error starting grab - 0x%x  or %d\n", status, status);
	}
	else if (numFrames == -1)
	{
		status = GevStartTransfer(handle, numFrames);//Grab continues
	}
}


void aq::gigeVCamera::stop_transfer(GEV_CAMERA_HANDLE handle)
{
	GevStopTransfer(handle);
}


void aq::gigeVCamera::about_transfer(GEV_CAMERA_HANDLE handle)
{
	GevAbortTransfer(handle);
}


void aq::gigeVCamera::convert_bayer_to_RGB(UINT32 height, UINT32 width, void* m_latestBuffer, void** bufToSave, int format)
{
	char filename[128] = { 0 };
	int ret = -1;

	uint32_t saveFormat = format;
	std::cout << "this is format" << format << std::endl;
	*bufToSave = m_latestBuffer;
	int allocate_conversion_buffer = 0;

	// Make sure we have data to save.
	if (m_latestBuffer != NULL)
	{
		uint32_t component_count = 1;
		UINT32 convertedFmt = 0;

		// Bayer conversion enabled for save image to file option.
		// Get the converted pixel type received from the API that is based on the pixel type output from the camera.
		// (Packed formats are automatically unpacked - unless in "passthru" mode.)

		convertedFmt = GevGetConvertedPixelType(0, format);
		if (GevIsPixelTypeBayer(convertedFmt) && ENABLE_BAYER_CONVERSION)
		{
			int img_size = 0;
			int img_depth = 0;
			uint8_t fill = 0;

			saveFormat = GevGetBayerAsRGBPixelType(convertedFmt);// Bayer will be converted to RGB.		
			img_depth = GevGetPixelDepthInBits(saveFormat);// Convert the image to RGB.
			component_count = GevGetPixelComponentCount(saveFormat);
			img_size = width * height * component_count * ((img_depth + 7) / 8);
			*bufToSave = malloc(img_size);
			fill = (component_count == 4) ? 0xFF : 0;  // Alpha if needed.
			memset(*bufToSave, fill, img_size);
			allocate_conversion_buffer = 1;
			
			ConvertBayerToRGB(0, height, width, convertedFmt, m_latestBuffer, saveFormat, bufToSave);// Convert the Bayer to RGB	
		}
		else
		{
			saveFormat = convertedFmt;
			allocate_conversion_buffer = 0;
		}
		int ret = -1;
		_GetUniqueFilename(filename, sizeof(filename) - 5, "img");
		strncat(filename, ".tif", sizeof(filename));
		/*ret = Write_GevImage_ToTIFF(filename, width, height, saveFormat, m_latestBuffer);
		if (ret > 0)
		{
			printf("image saved success");
		}
		else
		{
			printf("error saved image");
		}*/
	}

}


void aq::gigeVCamera::convert_bayer_to_RGB(UINT32 height, UINT32 width, UINT32 convertedFmt, void* m_latestBuffer, UINT32 format, void* bufToSave)
{
	char filename[128] = { 0 };
	int ret = -1;
	uint32_t saveFormat = format;
	bufToSave = m_latestBuffer;
	int allocate_conversion_buffer = 0;

	// Make sure we have data to save.
	if (m_latestBuffer != NULL)
	{
		uint32_t component_count = 1;
		UINT32 convertedFmt = 0;

		// Bayer conversion enabled for save image to file option.
		//
		// Get the converted pixel type received from the API that is 
		//	based on the pixel type output from the camera.
		// (Packed formats are automatically unpacked - unless in "passthru" mode.)
		//
		convertedFmt = GevGetConvertedPixelType(0, format);

		if (GevIsPixelTypeBayer(convertedFmt) && ENABLE_BAYER_CONVERSION)
		{
			int img_size = 0;
			int img_depth = 0;
			uint8_t fill = 0;

			// Bayer will be converted to RGB.
			saveFormat = GevGetBayerAsRGBPixelType(convertedFmt);

			// Convert the image to RGB.
			img_depth = GevGetPixelDepthInBits(saveFormat);
			component_count = GevGetPixelComponentCount(saveFormat);
			img_size = width * height * component_count * ((img_depth + 7) / 8);
			bufToSave = malloc(img_size);
			fill = (component_count == 4) ? 0xFF : 0;  // Alpha if needed.
			memset(bufToSave, fill, img_size);
			allocate_conversion_buffer = 1;

			// Convert the Bayer to RGB	
			ConvertBayerToRGB(0, height, width, convertedFmt, m_latestBuffer, saveFormat, bufToSave);

		}
		else
		{
			saveFormat = convertedFmt;
			allocate_conversion_buffer = 0;
		}
	}
}


/*
Author: lf
date:2020-10-22
Function: Use to connect a carame from IP adress;
GevAccessMode mode possible value may be:
GevExclusiveMode : Exclusive R/W access to the camera.
GevMonitorMode : Shared Read-only access to the camera.
GevControlMode : Shared R/W access to the camera.
*/
GEV_STATUS aq::gigeVCamera::open_camera_by_adress(std::string _ip, GevAccessMode mode, GEV_CAMERA_HANDLE& handle)
{
	std::string _error_str;
	GEV_STATUS status;
	//UINT32 u_ip_adress;
	//mode = GevMonitorMode;
	bool b_return = false;

	std::vector<std::string> ip = split(_ip, '.');
	if (ip.size() != 4)
	{
		_error_str = "Invalid Ip. There is a size error.";
		return false;
	}
	for (auto iter : ip)
	{
		int tmp = -1;
		try
		{
			tmp = std::stoi(iter);
		}
		catch (std::invalid_argument e)
		{
			std::cerr << e.what() << std::endl;
			_error_str = std::string("Invalid Ip. There is an invalid argument: ") + iter + ".";
			return false;
		}
		if ((tmp < 0) || (tmp > 255))
		{
			_error_str = std::string("Invalid Ip. There is an argument: ") + iter + " out of range.";
			return false;
		}
	}
	//u_ip_adress = atoi(ip[0].c_str())*0xff*0xff*0xff + atoi(ip[1].c_str())*0xff*0xff + atoi(ip[2].c_str())*0xff + atoi(ip[3].c_str());
	union iptolint ipl;
	strcpy(ipl.ip, _ip.c_str());
	ipl.n = conv(ipl.ip);
	//std::cout << "this is my change ip" << ipl.n << std::endl;

	status = GevOpenCameraByAddress(ipl.n, mode, &handle);
	if (status == 0)
		b_return = GEVLIB_OK;
	else
		b_return = GEVLIB_ERROR_INVALID_HANDLE;
	return b_return;
}


/*
Author: lf
date:2020-10-22
Function: Use to force set  carame IP adress which the camera MAC adress is mac_adress,the MAC adress format like : 00:01:0D:C5:20:DB ;
std::string mac_adress: camera MAC adrss, MAC format like : 00:01:0D:C5:20:DB
std::string target_ip : Camera  target ip adress that camera is  to be set.
std::string sub_mask : Camera netmask is to be set.
GevControlMode : Shared R/W access to the camera,can be nullptr.
*/
bool aq::gigeVCamera::set_camera_ip_adress(std::string mac_adress, std::string target_ip, std::string sub_mask, GevAccessMode mode)
{
	bool b_return = false;

	GEV_STATUS status = -1;
	int parse_index = 0;
	int  persistentIPMode = FALSE;
	unsigned int macLo = 0;
	unsigned int macHi = 0;
	unsigned int ipAddr = 0;
	unsigned int subnet_mask = 0;
	int len = 0;
	int i1, i2, i3, i4, i5, i6;
	persistentIPMode = TRUE;

	// Parse the command line.
	{
		len = sscanf(mac_adress.c_str(), "%x:%x:%x:%x:%x:%x", &i1, &i2, &i3, &i4, &i5, &i6);
		if (len != 6)
		{
			printf("\t*** Malformed MAC address string *** \n");
			exit(-1);
		}
		macLo = ((i3 & 0xFF) << 24) | ((i4 & 0xFF) << 16) | ((i5 & 0xFF) << 8) | (i6 & 0xFF);
		macHi = ((i1 & 0xFF) << 8) | (i2 & 0xFF);
	}

	{
		len = sscanf(target_ip.c_str(), "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
		if (len != 4)
		{
			printf("\t*** Malformed IP address string *** \n");
			exit(-1);
		}
		ipAddr = ((i1 & 0xFF) << 24) | ((i2 & 0xFF) << 16) | ((i3 & 0xFF) << 8) | (i4 & 0xFF);
	}

	{
		len = sscanf(sub_mask.c_str(), "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
		if (len != 4)
		{
			printf("\t*** Malformed IP subnet string *** \n");
			exit(-1);
		}
		subnet_mask = ((i1 & 0xFF) << 24) | ((i2 & 0xFF) << 16) | ((i3 & 0xFF) << 8) | (i4 & 0xFF);
	}

	// Initialize the API.
	GevApiInitialize();

	// Change the command acknowledge timeout 
	// Disable automatic XML feature initialization
	{
		GEVLIB_CONFIG_OPTIONS options = { 0 };

		GevGetLibraryConfigOptions(&options);
		if (options.command_timeout_ms < 500)
		{
			options.command_timeout_ms = 500;
		}
		options.manual_xml_handling = TRUE;  // No need to set up XML features by default.
		GevSetLibraryConfigOptions(&options);
	}

	// Program the IP address (and mode) 
	status = ForceCameraIPAndMode(persistentIPMode, macHi, macLo, ipAddr, subnet_mask);
	if (status == 0)
	{
		GEV_CAMERA_HANDLE handle;
		GevDeviceCount();

		// Open the camera by address (readonly mode)
		status = GevOpenCameraByAddress(ipAddr, GevMonitorMode, &handle);
		if (status == 0)
		{
			// Read the settings back and report them
			status = GetCameraIPSettings(handle, &ipAddr, &subnet_mask, &persistentIPMode);
			if (status == 0)
			{
				printf(" ForceIP : Camera at %02x:%02x:%02x:%02x:%02x:%02x set to ", ((macHi & 0x0000ff00) >> 8), (macHi & 0x000000ff), ((macLo & 0xff000000) >> 24), ((macLo & 0x00ff0000) >> 16), ((macLo & 0x0000ff00) >> 8), (macLo & 0x000000ff));
				if (persistentIPMode)
					printf("Persistent ");
				printf("IP address %d.%d.%d.%d ", ((ipAddr & 0xff000000) >> 24), ((ipAddr & 0x00ff0000) >> 16), ((ipAddr & 0x0000ff00) >> 8), (ipAddr & 0x000000ff));
				printf("(Mask= %d.%d.%d.%d)\n", ((subnet_mask & 0xff000000) >> 24), ((subnet_mask & 0x00ff0000) >> 16), ((subnet_mask & 0x0000ff00) >> 8), (subnet_mask & 0x000000ff));
			}
			else
			{
				printf(" ForceIP successful but could not read resulting configuration back from the camera!\n");
			}
			GevCloseCamera(&handle);
			b_return = true;
		}
		else
		{
			printf(" ForceIP successful but camera not accessible !\n");
		}
	}
	else
	{
		int decstatus = status;
		if ((status & 0x0000F000) == 0x0000F000) decstatus |= 0xFFFF0000;
		if ((status > 0x8000) && (status < 0x8FFF))
		{
			// Error from protocol.
			printf(" ForceIP failed - status = 0x%x (possibly no response from camera)\n", status);
		}
		else
		{
			// Error from API
			printf(" ForceIP failed - status = %d\n", decstatus);
		}
	}
	//GevApiUninitialize();

	return b_return;
}


std::vector<std::string> aq::gigeVCamera::split(std::string str, char seg)
{
	std::vector<std::string> split_strs;
	std::string temp;
	std::istringstream iss(str);
	while (getline(iss, temp, seg))
	{
		split_strs.push_back(temp);
	}
	return split_strs;
}


int aq::gigeVCamera::create_captrue_thread(std::string _ip, int argc, char* argv[])
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = { 0 };
	GEV_STATUS status;
	int numCamera = 0;
	int camIndex = 0;
	//aq::gigeVCamera::MY_CONTEXT context = { 0 };
	pthread_t  tid;
	char c;
	int done = FALSE;
	int turboDriveAvailable = 0;
	char uniqueName[FILENAME_MAX];
	char filename[FILENAME_MAX] = { 0 };
	uint32_t macLow = 0;		// Low 32-bits of the mac address (for file naming).

	// Boost application RT response (not too high since GEV library boosts data receive thread to max allowed)
	// SCHED_FIFO can cause many unintentional side effects.
	// SCHED_RR has fewer side effects.
	// SCHED_OTHER (normal default scheduler) is not too bad after all.
	if (0)
	{
		//int policy = SCHED_FIFO;
		int policy = SCHED_RR;
		pthread_attr_t attrib;
		int inherit_sched = 0;
		struct sched_param param = { 0 };

		// Set an average RT priority (increase/decrease to tuner performance).
		param.sched_priority = (sched_get_priority_max(policy) - sched_get_priority_min(policy)) / 2;

		// Set scheduler policy
		pthread_setschedparam(pthread_self(), policy, &param); // Don't care if it fails since we can't do anyting about it.	
		pthread_attr_init(&attrib);// Make sure all subsequent threads use the same policy.
		pthread_attr_getinheritsched(&attrib, &inherit_sched);
		if (inherit_sched != PTHREAD_INHERIT_SCHED)
		{
			inherit_sched = PTHREAD_INHERIT_SCHED;
			pthread_attr_setinheritsched(&attrib, inherit_sched);
		}
	}

	//============================================================================
	printf("\nGigE Vision Library GenICam C Example Program (save_data) (%s)\n", __DATE__);
	printf("Copyright (c) 2018, DALSA. (No restrictions on further use)\n\n");
	//====================================================================================
	// Set default options for the library.
	{
		GEVLIB_CONFIG_OPTIONS options = { 0 };
		GevGetLibraryConfigOptions(&options);
		//options.logLevel = GEV_LOG_LEVEL_OFF;
		//options.logLevel = GEV_LOG_LEVEL_TRACE;
		options.logLevel = GEV_LOG_LEVEL_NORMAL;
		GevSetLibraryConfigOptions(&options);
	}
	//====================================================================================
	// DISCOVER Cameras
	// Get all the IP addresses of attached network cards.
	status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);
	printf("%d camera(s) on the network\n", numCamera);

	// Select the first camera found (unless the command line has a parameter = the camera index)
	if (numCamera != 0)
	{
		if (argc > 1)
		{
			sscanf(argv[1], "%d", &camIndex);
			if (camIndex >= (int)numCamera)
			{
				printf("Camera index out of range - only %d camera(s) are present\n", numCamera);
				camIndex = -1;
			}
		}
		std::cout << "this is camera_index" << camIndex << std::endl;
		if (camIndex != -1)
		{
			//====================================================================
			// Connect to Camera,Direct instantiation of GenICam XML-based feature node map.
			int i;
			int type;
			UINT32 val = 0;
			UINT32 isLocked = 0;
			UINT32 height = 0;
			UINT32 width = 0;
			UINT32 format = 0;
			UINT32 maxHeight = 1600;
			UINT32 maxWidth = 2048;
			UINT32 maxDepth = 2;
			UINT64 size;
			UINT64 payload_size;
			int numBuffers = NUM_BUF;
			PUINT8 bufAddress[NUM_BUF];
			GEV_CAMERA_HANDLE handle = NULL;
	
			GevApiInitialize();
			status = open_camera_by_adress(_ip, GevExclusiveMode, handle);  // Open the camera.
			std::cout << "this is opencamera status" << status << std::endl;
			std::cout << "this is opencamera handle" << handle << std::endl;
			if (status == 0)
			{
				GEV_CAMERA_OPTIONS camOptions = { 0 };

				// Get the low part of the MAC address (use it as part of a unique file name for saving images).
				// Generate a unique base name to be used for saving image files based on the last 3 octets of the MAC address.
				macLow = pCamera[camIndex].macLow;
				macLow &= 0x00FFFFFF;
				snprintf(uniqueName, sizeof(uniqueName), "img_%06x", macLow);

				// If there are multiple pixel formats supported on this camera, get one.
				//{
				//	char feature_name[MAX_GEVSTRING_LENGTH] = { 0 };
				//	GetPixelFormatSelection(handle, sizeof(feature_name), feature_name);
				//	if (GevSetFeatureValueAsString(handle, "PixelFormat", feature_name) == 0)
				//	{
				//		printf("\n\tUsing selected PixelFormat = %s\n\n", feature_name);
				//	}
				//}

				// Go on to adjust some API related settings (for tuning / diagnostics / etc....).
				// Adjust the camera interface options if desired (see the manual)
				status = GevGetCameraInterfaceOptions(handle, &camOptions);
				//camOptions.heartbeat_timeout_ms = 60000;		// For debugging (delay camera timeout while in debugger)
				camOptions.heartbeat_timeout_ms = 5000;		// Disconnect detection (5 seconds)
				//camOptions.enable_passthru_mode = FALSE;
#if TUNE_STREAMING_THREADS
				// Some tuning can be done here. (see the manual)
				camOptions.streamFrame_timeout_ms = 1001;				// Internal timeout for frame reception.
				camOptions.streamNumFramesBuffered = 4;				// Buffer frames internally.
				camOptions.streamMemoryLimitMax = 64 * 1024 * 1024;		// Adjust packet memory buffering limit.	
				camOptions.streamPktSize = 9180;							// Adjust the GVSP packet size.
				camOptions.streamPktDelay = 10;							// Add usecs between packets to pace arrival at NIC.
				// Assign specific CPUs to threads (affinity) - if required for better performance.
				{
					int numCpus = _GetNumCpus();
					if (numCpus > 1)
					{
						camOptions.streamThreadAffinity = numCpus - 1;
						camOptions.serverThreadAffinity = numCpus - 2;
					}
				}
#endif
				// Write the adjusted interface options back.
				status = GevSetCameraInterfaceOptions(handle, &camOptions);

				//===========================================================
				// Set up the frame information.....

				double exposuretime = 90.0;
				status = GevSetFeatureValue(handle, "ExposureTime", sizeof(exposuretime), &exposuretime);
				double Gain = 10.0;
				status = GevSetFeatureValue(handle, "Gain", sizeof(Gain), &Gain);
				double LineRate = 1000.0;
				status = GevSetFeatureValue(handle, "AcquisitionLineRate", sizeof(LineRate), &LineRate);
				height = 100;
				status = GevSetFeatureValue(handle, "Height", sizeof(height), &height);
				val = 0;
				status = GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val);
				val = 1;
				status = GevSetFeatureValue(handle, "transferTurboMode", sizeof(val), &val);

				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "ExposureTime", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "Gain", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "AcquisitionLineRate", &type, sizeof(exposuretime), &exposuretime);
				std::cout << "this is exposuretime" << exposuretime << std::endl;

				printf("Camera ROI set for \n");
				status = GevGetFeatureValue(handle, "Width", &type, sizeof(width), &width);
				printf("\tWidth = %d\n", width);
				status = GevGetFeatureValue(handle, "Height", &type, sizeof(height), &height);
				printf("\tHeight = %d\n", height);
				status = GevGetFeatureValue(handle, "PixelFormat", &type, sizeof(format), &format);
				printf("\tPixelFormat  = 0x%x\n", format);

				if (camOptions.enable_passthru_mode)
				{
					printf("\n\tPASSTHRU Mode is ON\n");
				}

				if (IsTurboDriveAvailable(handle))
				{
					printf("\n\tTurboDrive is : \n");
					val = 1;
					if (GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val) == 0)
					{
						if (val == 1)
						{
							printf("ON\n");
						}
						else
						{
							printf("OFF\n");
						}
					}
				}
				else
				{
					printf("\t*** TurboDrive is NOT Available ***\n");
				}

				printf("\n");
				bool b_run = true;
				// End frame info
				//============================================================

				if (status == 0)
				{
					while (true)
					{
						if (b_run)
						{
							b_run = false;
							//=================================================================
							// Set up a grab/transfer from this camera based on the settings...
							GevGetPayloadParameters(handle, &payload_size, (UINT32*)&type);
							maxHeight = height;
							maxWidth = width;
							maxDepth = GetPixelSizeInBytes(format);

							// Calculate the size of the image buffers.
							// (Adjust the number of lines in the buffer to fit the maximum expected  chunk size - just in case it gets enabled !!!)
							{
								int extra_lines = (MAX_CHUNK_BYTES + width - 1) / width;
								//size = GetPixelSizeInBytes(format) * width * (height + extra_lines);
								size = GetPixelSizeInBytes(format) * width * height;
							}

							// Allocate image buffers (Either the image size or the payload_size, whichever is larger - allows for packed pixel formats and metadata).
							size = (payload_size > size) ? payload_size : size;
							for (i = 0; i < numBuffers; i++)
							{
								bufAddress[i] = (PUINT8)malloc(size);
								memset(bufAddress[i], 0, size);
							}

							// Generate a file name from the unique base name (leave at least 16 digits for index and extension)
							_GetUniqueFilename_sec(filename, (sizeof(filename) - 17), uniqueName);

							// Initialize a transfer with synchronous buffer handling. (To avoid overwriting data buffer while saving to disk).
							status = GevInitializeTransfer(handle, Asynchronous, size, numBuffers, bufAddress);
							std::cout << "this is size" << size << std::endl;
							std::cout << "this is transfer status" << status << std::endl;
							////coversion to RGB888
							//pixDepth = 32;
							//context.depth = pixDepth;
							//context.convertBuffer = malloc((maxWidth * maxHeight * ((pixDepth + 7) / 8)));
							//context.convertFormat = TRUE;
							// Create a thread to receive images from the API and save them
							context.camHandle = handle;
							context.base_name = filename;
							context.format = format;
							context.camera_serial = argc;
							context.exit = FALSE;
							std::cout << "begin save image" << std::endl;
							status = GevStartTransfer(handle, -1);
							
							std::cout << "this is starttransfer status" << status << std::endl;
							pthread_create(&tid, NULL, ImageCaptureThread, &context);

							/*if (0)
							{
								GevAbortTransfer(handle);
								status = GevFreeTransfer(handle);

								for (i = 0; i < numBuffers; i++)
								{
									free(bufAddress[i]);
								}
							}*/
							
						}
						else
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
							continue;
						}
					}

				}
				//GevCloseCamera(&handle);
			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}
	GevAbortTransfer(handle);
	status = GevFreeTransfer(handle);

	for (i = 0; i < numBuffers; i++)
	{
		free(bufAddress[i]);
	}
	GevCloseCamera(&handle);
	GevApiUninitialize();	// Close down the API.	
	_CloseSocketAPI();		// Close socket API, must close API even on error
	return 0;
}



GEV_CAMERA_HANDLE  aq::gigeVCamera::create_camera_handle(std::string _ip, int argc)
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = { 0 };
	GEV_STATUS status;
	int numCamera = 0;
	int camIndex = 0;
	aq::gigeVCamera::MY_CONTEXT context = { 0 };
	pthread_t  tid;
	char c;
	int done = FALSE;
	int turboDriveAvailable = 0;
	char uniqueName[FILENAME_MAX];
	char filename[FILENAME_MAX] = { 0 };
	uint32_t macLow = 0;		// Low 32-bits of the mac address (for file naming).
	int i;
	int type;
	UINT32 val = 0;
	UINT32 isLocked = 0;
	UINT32 height = 0;
	UINT32 width = 0;
	UINT32 format = 0;
	UINT32 maxHeight = 1600;
	UINT32 maxWidth = 2048;
	UINT32 maxDepth = 2;
	UINT64 size;
	UINT64 payload_size;
	int numBuffers = NUM_BUF;
	PUINT8 bufAddress[NUM_BUF];
	GEV_CAMERA_HANDLE handle = NULL;

	if (0)
	{
		//int policy = SCHED_FIFO;
		int policy = SCHED_RR;
		pthread_attr_t attrib;
		int inherit_sched = 0;
		struct sched_param param = { 0 };

		// Set an average RT priority (increase/decrease to tuner performance).
		param.sched_priority = (sched_get_priority_max(policy) - sched_get_priority_min(policy)) / 2;

		// Set scheduler policy
		pthread_setschedparam(pthread_self(), policy, &param); // Don't care if it fails since we can't do anyting about it.

		// Make sure all subsequent threads use the same policy.
		pthread_attr_init(&attrib);
		pthread_attr_getinheritsched(&attrib, &inherit_sched);
		if (inherit_sched != PTHREAD_INHERIT_SCHED)
		{
			inherit_sched = PTHREAD_INHERIT_SCHED;
			pthread_attr_setinheritsched(&attrib, inherit_sched);
		}
	}
	printf("\nGigE Vision Library GenICam C Example Program (save_data) (%s)\n", __DATE__);
	printf("Copyright (c) 2018, DALSA. (No restrictions on further use)\n\n");

	GEVLIB_CONFIG_OPTIONS options = { 0 };   // Set default options for the library.
	GevGetLibraryConfigOptions(&options);
	//options.logLevel = GEV_LOG_LEVEL_OFF;
	//options.logLevel = GEV_LOG_LEVEL_TRACE;
	options.logLevel = GEV_LOG_LEVEL_NORMAL;
	GevSetLibraryConfigOptions(&options);	
	status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);   // Get all the IP addresses of attached network cards.
	printf("%d camera(s) on the network\n", numCamera);

	if (numCamera != 0)  // Select the first camera found (unless the command line has a parameter = the camera index)
	{
		if (argc > 1)
		{
			if (camIndex >= (int)numCamera)
			{
				printf("Camera index out of range - only %d camera(s) are present\n", numCamera);
				camIndex = -1;
			}
		}
		std::cout << "this is camera_index " << camIndex << std::endl;
		if (camIndex != -1)
		{
			// Connect to Camera,Direct instantiation of GenICam XML-based feature node map.
			GevApiInitialize();
			status = open_camera_by_adress(_ip, GevExclusiveMode, handle);// Open the camera.
			if(status==0)
				b_camera_open = true;
			std::cout << "this is opencamera status" << status << std::endl;
			std::cout << "this is opencamera handle" << handle << std::endl;
		}
	}
	return handle;

}


int aq::gigeVCamera::create_captrue_thread(std::string _ip, int argc)
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = { 0 };
	GEV_STATUS status;
	int numCamera = 0;
	int camIndex = 0;
	aq::gigeVCamera::MY_CONTEXT context = { 0 };
	pthread_t  tid;
	char c;
	int done = FALSE;
	int turboDriveAvailable = 0;
	char uniqueName[FILENAME_MAX];
	char filename[FILENAME_MAX] = { 0 };
	uint32_t macLow = 0;		// Low 32-bits of the mac address (for file naming).

	// Boost application RT response (not too high since GEV library boosts data receive thread to max allowed)
	// SCHED_FIFO can cause many unintentional side effects.
	// SCHED_RR has fewer side effects.
	// SCHED_OTHER (normal default scheduler) is not too bad afer all.
	if (0)
	{
		//int policy = SCHED_FIFO;
		int policy = SCHED_RR;
		pthread_attr_t attrib;
		int inherit_sched = 0;
		struct sched_param param = { 0 };

		// Set an average RT priority (increase/decrease to tuner performance).
		param.sched_priority = (sched_get_priority_max(policy) - sched_get_priority_min(policy)) / 2;

		// Set scheduler policy
		pthread_setschedparam(pthread_self(), policy, &param); // Don't care if it fails since we can't do anyting about it.

		// Make sure all subsequent threads use the same policy.
		pthread_attr_init(&attrib);
		pthread_attr_getinheritsched(&attrib, &inherit_sched);
		if (inherit_sched != PTHREAD_INHERIT_SCHED)
		{
			inherit_sched = PTHREAD_INHERIT_SCHED;
			pthread_attr_setinheritsched(&attrib, inherit_sched);
		}
	}
	//============================================================================
	// Greetings
	printf("\nGigE Vision Library GenICam C Example Program (save_data) (%s)\n", __DATE__);
	printf("Copyright (c) 2018, DALSA. (No restrictions on further use)\n\n");

	//===================================================================================
	// Set default options for the library.
	{
		GEVLIB_CONFIG_OPTIONS options = { 0 };

		GevGetLibraryConfigOptions(&options);
		//options.logLevel = GEV_LOG_LEVEL_OFF;
		//options.logLevel = GEV_LOG_LEVEL_TRACE;
		options.logLevel = GEV_LOG_LEVEL_NORMAL;
		GevSetLibraryConfigOptions(&options);
	}
	//====================================================================================
	// DISCOVER Cameras
	//
	// Get all the IP addresses of attached network cards.
	status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);
	printf("%d camera(s) on the network\n", numCamera);

	// Select the first camera found (unless the command line has a parameter = the camera index)
	if (numCamera != 0)
	{
		if (argc > 1)
		{
			if (camIndex >= (int)numCamera)
			{
				printf("Camera index out of range - only %d camera(s) are present\n", numCamera);
				camIndex = -1;
			}
		}
		std::cout << "this is camera_index" << camIndex << std::endl;
		if (camIndex != -1)
		{
			//====================================================================
			// Connect to Camera,Direct instantiation of GenICam XML-based feature node map.
			int i;
			int type;
			UINT32 val = 0;
			UINT32 isLocked = 0;
			UINT32 height = 0;
			UINT32 width = 0;
			UINT32 format = 0;
			UINT32 maxHeight = 1600;
			UINT32 maxWidth = 2048;
			UINT32 maxDepth = 2;
			UINT64 size;
			UINT64 payload_size;
			int numBuffers = NUM_BUF;
			PUINT8 bufAddress[NUM_BUF];
			GEV_CAMERA_HANDLE handle = NULL;

			// Open the camera.
			GevApiInitialize();
			status = open_camera_by_adress(_ip, GevExclusiveMode, handle);
			std::cout << "this is opencamera status" << status << std::endl;
			std::cout << "this is opencamera handle" << handle << std::endl;
			if (status == 0)
			{
				GEV_CAMERA_OPTIONS camOptions = { 0 };

				// Get the low part of the MAC address (use it as part of a unique file name for saving images).
				// Generate a unique base name to be used for saving image files based on the last 3 octets of the MAC address.
				macLow = pCamera[camIndex].macLow;
				macLow &= 0x00FFFFFF;
				snprintf(uniqueName, sizeof(uniqueName), "img_%06x", macLow);

				// If there are multiple pixel formats supported on this camera, get one.
				//{
				//	char feature_name[MAX_GEVSTRING_LENGTH] = { 0 };
				//	GetPixelFormatSelection(handle, sizeof(feature_name), feature_name);
				//	if (GevSetFeatureValueAsString(handle, "PixelFormat", feature_name) == 0)
				//	{
				//		printf("\n\tUsing selected PixelFormat = %s\n\n", feature_name);
				//	}
				//}

				// Go on to adjust some API related settings (for tuning / diagnostics / etc....).
				// Adjust the camera interface options if desired (see the manual)
				status = GevGetCameraInterfaceOptions(handle, &camOptions);
				camOptions.heartbeat_timeout_ms = 5000;		// Disconnect detection (5 seconds)
				//camOptions.enable_passthru_mode = FALSE;
#if TUNE_STREAMING_THREADS
				// Some tuning can be done here. (see the manual)
				camOptions.streamFrame_timeout_ms = 1001;				// Internal timeout for frame reception.
				camOptions.streamNumFramesBuffered = 4;				// Buffer frames internally.
				camOptions.streamMemoryLimitMax = 64 * 1024 * 1024;		// Adjust packet memory buffering limit.	
				camOptions.streamPktSize = 9180;							// Adjust the GVSP packet size.
				camOptions.streamPktDelay = 10;							// Add usecs between packets to pace arrival at NIC.
				// Assign specific CPUs to threads (affinity) - if required for better performance.
				{
					int numCpus = _GetNumCpus();
					if (numCpus > 1)
					{
						camOptions.streamThreadAffinity = numCpus - 1;
						camOptions.serverThreadAffinity = numCpus - 2;
					}
				}
#endif
				// Write the adjusted interface options back.
				status = GevSetCameraInterfaceOptions(handle, &camOptions);

				//===========================================================
				// Set up the frame information.....

				double exposuretime = 90.0;
				status = GevSetFeatureValue(handle, "ExposureTime", sizeof(exposuretime), &exposuretime);
				double Gain = 10.0;
				status = GevSetFeatureValue(handle, "Gain", sizeof(Gain), &Gain);
				double LineRate = 1000.0;
				status = GevSetFeatureValue(handle, "AcquisitionLineRate", sizeof(LineRate), &LineRate);
				height = 100;
				status = GevSetFeatureValue(handle, "Height", sizeof(height), &height);
				val = 0;
				status = GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val);
				val = 1;
				status = GevSetFeatureValue(handle, "transferTurboMode", sizeof(val), &val);

				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "ExposureTime", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "Gain", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "AcquisitionLineRate", &type, sizeof(exposuretime), &exposuretime);
				std::cout << "this is exposuretime" << exposuretime << std::endl;

				printf("Camera ROI set for \n");
				status = GevGetFeatureValue(handle, "Width", &type, sizeof(width), &width);
				printf("\tWidth = %d\n", width);
				status = GevGetFeatureValue(handle, "Height", &type, sizeof(height), &height);
				printf("\tHeight = %d\n", height);
				status = GevGetFeatureValue(handle, "PixelFormat", &type, sizeof(format), &format);
				printf("\tPixelFormat  = 0x%x\n", format);

				if (camOptions.enable_passthru_mode)
				{
					printf("\n\tPASSTHRU Mode is ON\n");
				}

				if (IsTurboDriveAvailable(handle))
				{
					printf("\n\tTurboDrive is : \n");
					val = 1;
					if (GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val) == 0)
					{
						if (val == 1)
						{
							printf("ON\n");
						}
						else
						{
							printf("OFF\n");
						}
					}
				}
				else
				{
					printf("\t*** TurboDrive is NOT Available ***\n");
				}

				printf("\n");
				bool b_run = true;
				//
				// End frame info
				//============================================================

				if (status == 0)
				{
					//while (true)
					{
						if (b_run)
						{
							b_run = false;
							//=================================================================
							// Set up a grab/transfer from this camera based on the settings...
							GevGetPayloadParameters(handle, &payload_size, (UINT32*)&type);
							maxHeight = height;
							maxWidth = width;
							maxDepth = GetPixelSizeInBytes(format);

							// Calculate the size of the image buffers. (Adjust the number of lines in the buffer to fit the maximum expected  chunk size - just in case it gets enabled !!!)
							int extra_lines = (MAX_CHUNK_BYTES + width - 1) / width;
							//size = GetPixelSizeInBytes(format) * width * (height + extra_lines);
							size = GetPixelSizeInBytes(format) * width * height;

							// Allocate image buffers (Either the image size or the payload_size, whichever is larger - allows for packed pixel formats and metadata).
							size = (payload_size > size) ? payload_size : size;
							for (i = 0; i < numBuffers; i++)
							{
								bufAddress[i] = (PUINT8)malloc(size);
								memset(bufAddress[i], 0, size);
							}

							_GetUniqueFilename_sec(filename, (sizeof(filename) - 17), uniqueName);  // Generate a file name from the unique base name (leave at least 16 digits for index and extension)

							// Initialize a transfer with synchronous buffer handling. (To avoid overwriting data buffer while saving to disk).
							status = GevInitializeTransfer(handle, Asynchronous, size, numBuffers, bufAddress);
							std::cout << "this is size" << size << std::endl;
							std::cout << "this is transfer status" << status << std::endl;
							////coversion to RGB888
							//pixDepth = 32;
							//context.depth = pixDepth;
							//context.convertBuffer = malloc((maxWidth * maxHeight * ((pixDepth + 7) / 8)));
							//context.convertFormat = TRUE;
							// Create a thread to receive images from the API and save them
							context.camHandle = handle;
							context.base_name = filename;
							context.format = format;
							context.camera_serial = argc;
							context.exit = FALSE;
							std::cout << "begin save image" << std::endl;
							status = GevStartTransfer(handle, -1);

							std::cout << "this is starttransfer status" << status << std::endl;
							pthread_create(&tid, NULL, ImageCaptureThread, &context);

							if (0)
							{
								GevAbortTransfer(handle);
								status = GevFreeTransfer(handle);

								for (i = 0; i < numBuffers; i++)
								{
									free(bufAddress[i]);
								}
							}

						}
					}
				}
				//GevCloseCamera(&handle);
			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}	
}


int aq::gigeVCamera::create_captrue_thread(GEV_CAMERA_HANDLE	handle)
{
	GEV_DEVICE_INTERFACE  pCamera[MAX_CAMERAS] = { 0 };
	GEV_STATUS status;
	int numCamera = 0;
	int camIndex = 0;
//	aq::gigeVCamera::MY_CONTEXT context = { 0 };
	pthread_t  tid;
	char c;
	int done = FALSE;
	int turboDriveAvailable = 0;
	char uniqueName[FILENAME_MAX];
	char filename[FILENAME_MAX] = { 0 };
	uint32_t macLow = 0;		// Low 32-bits of the mac address (for file naming).
	UINT32  val = 0;
	UINT32 argc = index_num;
	
	status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);
	printf("%d camera(s) on the network\n", numCamera);
	
	if (numCamera != 0)	// Select the first camera found (unless the command line has a parameter = the camera index)
	{
		std::cout << "this is camera_index" << camIndex << std::endl;
		if (camIndex != -1)
		{
			if (handle != 0)
			{
				GEV_CAMERA_OPTIONS camOptions = { 0 };

				// Get the low part of the MAC address (use it as part of a unique file name for saving images).
				// Generate a unique base name to be used for saving image files based on the last 3 octets of the MAC address.
				macLow = pCamera[camIndex].macLow;
				macLow &= 0x00FFFFFF;
				snprintf(uniqueName, sizeof(uniqueName), "img_%06x", macLow);

				status = GevGetCameraInterfaceOptions(handle, &camOptions);
				//camOptions.heartbeat_timeout_ms = 60000;		// For debugging (delay camera timeout while in debugger)
				camOptions.heartbeat_timeout_ms = 5000;		// Disconnect detection (5 seconds)
				camOptions.streamFrame_timeout_ms = 1001;				// Internal timeout for frame reception.
				camOptions.streamNumFramesBuffered = 4;				// Buffer frames internally.
				camOptions.streamMemoryLimitMax = 64 * 1024 * 1024;		// Adjust packet memory buffering limit.	
				camOptions.streamPktSize = 9180;							// Adjust the GVSP packet size.
				camOptions.streamPktDelay = 10;							// Add usecs between packets to pace arrival at NIC.
				// Assign specific CPUs to threads (affinity) - if required for better performance.
				{
					int numCpus = _GetNumCpus();
					if (numCpus > 1)
					{
						camOptions.streamThreadAffinity = numCpus - 1;
						camOptions.serverThreadAffinity = numCpus - 2;
					}
				}
				// Write the adjusted interface options back.
				status = GevSetCameraInterfaceOptions(handle, &camOptions);
				double exposuretime = 90.0;
				status = GevSetFeatureValue(handle, "ExposureTime", sizeof(exposuretime), &exposuretime);
				double Gain = 10.0;
				status = GevSetFeatureValue(handle, "Gain", sizeof(Gain), &Gain);
				double LineRate = 1000.0;
				status = GevSetFeatureValue(handle, "AcquisitionLineRate", sizeof(LineRate), &LineRate);
				height = 100;
				status = GevSetFeatureValue(handle, "Height", sizeof(height), &height);
				val = 0;
				status = GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val);
				val = 1;
				status = GevSetFeatureValue(handle, "transferTurboMode", sizeof(val), &val);
				status = GevGetFeatureValue(handle, "PixelFormat", &type, sizeof(val), &val); //PixelFormat:RGB8--35127316;BiColorGRB8--34603173
				val = 35127316;		//Set to be GRB8 format
				status = GevSetFeatureValue(handle, "PixelFormat", sizeof(val), &val);
				status = GevSetFeatureValueAsString(handle, "PixelFormat", "RGB8");

				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "ExposureTime", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "Gain", &type, sizeof(exposuretime), &exposuretime);
				exposuretime = 0.0;
				status = GevGetFeatureValue(handle, "AcquisitionLineRate", &type, sizeof(exposuretime), &exposuretime);
				std::cout << "this is exposuretime" << exposuretime << std::endl;

				printf("Camera ROI set for \n");
				status = GevGetFeatureValue(handle, "Width", &type, sizeof(width), &width);
				printf("\tWidth = %d\n", width);
				status = GevGetFeatureValue(handle, "Height", &type, sizeof(height), &height);
				printf("\tHeight = %d\n", height);
				status = GevGetFeatureValue(handle, "PixelFormat", &type, sizeof(format), &format);
				printf("\tPixelFormat  = 0x%x\n", format);

				if (camOptions.enable_passthru_mode)
				{
					printf("\n\tPASSTHRU Mode is ON\n");
				}

				if (IsTurboDriveAvailable(handle))
				{
					printf("\n\tTurboDrive is : \n");
					val = 1;
					if (GevGetFeatureValue(handle, "transferTurboMode", &type, sizeof(UINT32), &val) == 0)
					{
						if (val == 1)
						{
							printf("ON\n");
						}
						else
						{
							printf("OFF\n");
						}
					}
				}
				else
				{
					printf("\t*** TurboDrive is NOT Available ***\n");
				}

				bool b_run = true;
				if (status == 0)
				{
					if (b_run)
					{
						b_run = false;
						GevGetPayloadParameters(handle, &payload_size, (UINT32*)&type);
						maxHeight = height;
						maxWidth = width;
						maxDepth = GetPixelSizeInBytes(format);

						// Calculate the size of the image buffers.// (Adjust the number of lines in the buffer to fit the maximum expected  chunk size - just in case it gets enabled !!!)
						int extra_lines = (MAX_CHUNK_BYTES + width - 1) / width;
						//size = GetPixelSizeInBytes(format) * width * (height + extra_lines);
						size = GetPixelSizeInBytes(format) * width * height;
						
						// Allocate image buffers (Either the image size or the payload_size, whichever is larger - allows for packed pixel formats and metadata).
						size = (payload_size > size) ? payload_size : size;
						for (i = 0; i < numBuffers; i++)
						{
							bufAddress[i] = (PUINT8)malloc(size);
							memset(bufAddress[i], 0, size);
						}												
						_GetUniqueFilename_sec(filename, (sizeof(filename) - 17), uniqueName);  // Generate a file name from the unique base name (leave at least 16 digits for index and extension)

						// Initialize a transfer with synchronous buffer handling. (To avoid overwriting data buffer while saving to disk).
						status = GevInitializeTransfer(handle, Asynchronous, size, numBuffers, bufAddress);
						std::cout << "this is size" << size << std::endl;
						std::cout << "this is transfer status" << status << std::endl;
						////coversion to RGB888
						//pixDepth = 32;
						//context.depth = pixDepth;
						//context.convertBuffer = malloc((maxWidth * maxHeight * ((pixDepth + 7) / 8)));
						//context.convertFormat = TRUE;
						// Create a thread to receive images from the API and save them
						context.camHandle = handle;
						context.base_name = filename;
						context.format = format;
						context.camera_serial = argc;
						context.exit = FALSE;
						std::cout << "begin save image" << std::endl;
						status = GevStartTransfer(handle, -1);

						std::cout << "this is starttransfer status" << status << std::endl;
						pthread_create(&tid, NULL, ImageCaptureThread, &context);
					}
				}
			}
			else
			{
				printf("Error : 0x%0x : opening camera\n", status);
			}
		}
	}
}


int aq::gigeVCamera::close_camera(GEV_CAMERA_HANDLE camera_handle)
{
	GEV_STATUS status;
	GevAbortTransfer(camera_handle);
	status = GevFreeTransfer(camera_handle);

	for (i = 0; i < numBuffers; i++)
	{
		free(bufAddress[i]);
	}
	GevCloseCamera(&camera_handle);
	GevApiUninitialize();	// Close down the API.	
	_CloseSocketAPI();		// Close socket API, must close API even on error
	return 0;
}



/*
author:LF
date: 2020-10-24
function: This function set camera trigger mode,
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			b_trigger: 1, extern pulse tigger mode; 0, auto continues grab.
*/
bool  aq::gigeVCamera::set_camer_trigger(GEV_CAMERA_HANDLE handle, bool b_trigger)
{

	GEV_STATUS status;
	if (b_trigger)	//Set camera to be pulse trigger mode 
	{
		status = GevSetFeatureValueAsString(handle, "TriggerMode", "On");
		status = GevSetFeatureValueAsString(handle, "TriggerSelector", "FrameStart");
		status = GevSetFeatureValueAsString(handle, "TriggerSource", "Action1");
	}
	else	//Set camera to be continues grab mode 
	{
		status = GevSetFeatureValueAsString(handle, "TriggerMode", "Off");
	}
	if (status == 0)
		return true;
	else
		return false;
}



/*
author:LF
date:2020-10-24
handle   :   Handle to the camera
featrue_name:	String containing the name of the ferure to be accessed
feature_type:
			GENAPI_UBUSED_TYPE  =   1
			GENAPI_INTEGER_TYPE  =  2
			GENAPI_BOOLEAN_TYPE  =  3
			GENAPI_COMMAND_TYPE  =  4
			GENAPI_FLOAT_TYPE  =	5
			GENAPI_STRING_TYPE  =	6
			GENAPI_REGISTER_TYPE  = 7
			GENAPI_ENUM_TYPE  =		9
			GENAPI_ENUMENTRY_TYPE  =  10
value_size : Size ,in bytes,of the storage pointed by "value" that receives the data contained at the featrue node being accesed;
value:       Pointed to storage at which to return the data read from the featrue node;
*/
bool aq::gigeVCamera::get_camera_roi_param(GEV_CAMERA_HANDLE handle, int& width, int& height, int& pixel_format)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevGetFeatureValue(handle, "Width", &type, sizeof(width), &width);
	status = GevGetFeatureValue(handle, "Height", &type, sizeof(height), &height);
	status = GevGetFeatureValue(handle, "PixelFormat", &type, sizeof(format), &format);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-24
function: This function set camera  ROI rect,to set capture image width and image height
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int width: to set capture image width.
			int height: to set capture image height.
*/
bool aq::gigeVCamera::set_camera_roi_param(GEV_CAMERA_HANDLE handle, int width, int height)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	//status = GevSetFeatureValueAsString(handle, "TriggerMode", "On");
	status = GevSetFeatureValue(handle, "Width", sizeof(width), &width);
	status = GevSetFeatureValue(handle, "Height", sizeof(height), &height);
	//status = GevSetFeatureValue(handle, "PixelFormat", sizeof(format), &format);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-24
function: This function set camera  ROI rect width;
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int width: to set capture image width.
*/
bool aq::gigeVCamera::set_camera_width(GEV_CAMERA_HANDLE handle, int width)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevSetFeatureValue(handle, "Width", sizeof(width), &width);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-24
function: This function get camera  ROI rect width;
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int& width: to get capture image width.
*/
bool aq::gigeVCamera::get_camera_width(GEV_CAMERA_HANDLE handle, int& width)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevGetFeatureValue(handle, "Width", &type, sizeof(width), &width);

	if (status == 0)
		b_return = true;
	else
		b_return = false;
	return b_return;
}


/*
author:LF
date: 2020-10-25
function: This function get camera  ROI rect height
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int& height capture image height.
*/
bool aq::gigeVCamera::get_camera_height(GEV_CAMERA_HANDLE handle, int& height)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevGetFeatureValue(handle, "Height", &type, sizeof(height), &height);

	return b_return;
}


/*
author:LF
date: 2020-10-25
function: This function set camera  ROI rect height
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int height: to set capture image height.
*/
bool aq::gigeVCamera::set_camera_height(GEV_CAMERA_HANDLE handle, int height)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevSetFeatureValue(handle, "Height", sizeof(height), &height);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}

/*
author:LF
date: 2020-10-25
function: This function set camera  exposure time
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int exposure_time: to set camera  exposure_time.
*/
bool aq::gigeVCamera::set_camera_exposuretime(GEV_CAMERA_HANDLE handle, int exposure_time)  //exposuretime:us
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	double df_value = exposure_time;
	status = GevSetFeatureValue(handle, "ExposureTime", sizeof(df_value), &df_value);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-25
function: This function get camera  exposure time
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int exposure_time: to get camera  exposure time.
*/
bool aq::gigeVCamera::get_camera_exposuretime(GEV_CAMERA_HANDLE handle, int& exposure_time)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format

	status = GevGetFeatureValue(handle, "Exposuretime", &type, sizeof(exposure_time), &exposure_time);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-25
function: This function to get camera  digital gain
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int& gain: to get camera  digital gain value.
*/
bool aq::gigeVCamera::get_camera_gain(GEV_CAMERA_HANDLE handle, int& gain)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	status = GevGetFeatureValue(handle, "Gain", &type, sizeof(gain), &gain);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}


/*
author:LF
date: 2020-10-25
function: This function to set camera  digital gain
parameters:
			GEV_CAMERA_HANDLE handle:handle point to a opened camera device;
			int& gain: to set camera  digital gain value.
*/
bool aq::gigeVCamera::set_camera_gain(GEV_CAMERA_HANDLE handle, int gain)
{
	bool b_return = false;
	GEV_STATUS status;
	int type;
	// Get the camera width, height, and pixel format
	double df_value = gain;
	status = GevSetFeatureValue(handle, "Gain", sizeof(df_value), &df_value);

	if (status == 0)
		b_return = true;
	else
		b_return = false;

	return b_return;
}



/*
author:LF
date: 2020-10-24
function: This function lists all cameras detected and prints their information to stdout.
output format: Format is " <Manuf>:<Model>:<Sn>:<Version>:<DeviceUserName> aka [MAC]@[<CameraIP>] on <netname> aka [<NIC IP>] "
*/
GEV_DEVICE_INTERFACE* aq::gigeVCamera::get_all_cameras(int level)
{
	return ListAllCameras(level);
}


int GetPixelFormatSelection(GEV_CAMERA_HANDLE handle, int size, char* pixelFormatString)
{
	int status = -1;
	int count = 0;
	int type = 0;

	// Get the node map.
	GenApi::CNodeMapRef* Camera = static_cast<GenApi::CNodeMapRef*>(GevGetFeatureNodeMap(handle));
	if (Camera)
	{
		try
		{
			// Access the PixelFormat (mandatory) enumeration
			GenApi::CEnumerationPtr ptrPixFormat = Camera->_GetNode("PixelFormat");
			GenApi::NodeList_t selection;
			GenApi::StringList_t availableFormats;
			ptrPixFormat->GetEntries(selection);

			// See if there is more than one possible format
			count = (int)selection.size();
			if (count == 1)
			{
				// Only one pixel format - this is it.
				status = GevGetFeatureValueAsString(handle, "PixelFormat", &type, size, pixelFormatString);
				return status;
			}
			else
			{
				// Multiple pixel formats available - Iterate to present them for selection.
				int numFormats = 0;
				int i = 0;
				int num = 0;
				int done = 0;
				for (i = 0; i < count; i++)
				{
					GenApi::CEnumEntryPtr entry(selection[i]);
					if ((GenApi::NI != entry->GetAccessMode()) && (GenApi::NA != entry->GetAccessMode()))
					{
						numFormats++;
						availableFormats.push_back(entry->GetSymbolic());
					}
				}

				if (numFormats > 1)
				{
					while (!done)
					{
						printf("\n Multiple PixelFormats available - select one by index/id :\n");
						for (i = 0; i < numFormats; i++)
						{
							printf("    [%02d] = %s \n", i, (const char*)(availableFormats[i]));
						}
						printf("Enter index (0 to %d) :", (numFormats - 1));
						fflush(stdout);
						num = GetNumber();
						if ((num >= 0) && (num < numFormats))
						{
							done = 1;
						}
						else
						{
							printf(" \t\t%d : <out of range> ", num);
						}
					}
				}
				else
				{
					num = 0;
					printf("\n Pixel format set to %s \n", (const char*)(availableFormats[i]));
				}
				strncpy(pixelFormatString, (const char*)(availableFormats[num]), size);
				status = 0;
			}
		}
		CATCH_GENAPI_ERROR(status);
	}
	return status;
}



GEV_STATUS ForceCameraIPAndMode(BOOL persistentIPMode, UINT32 macHi, UINT32 macLo, UINT32 IPAddress, UINT32 subnet_mask)
{
	GEV_STATUS status = -1;

	// Camera must be closed (an open camera will disappear from its current settings).
	// Force the IP for the identified device.
	status = GevForceCameraIPAddress(macHi, macLo, IPAddress, subnet_mask);

	if (status == 0)
	{
		int num = 0;
		GEV_CAMERA_HANDLE handle;

		// Forced a new IP address and subnet mask. Get a handle to the camera and save the settings.
		// First - refresh the camera list with the new camera.
		num = GevDeviceCount();

		// Open the camera by address;
		status = GevOpenCameraByAddress(IPAddress, GevExclusiveMode, &handle);
		if (status == 0)
		{
			// Have access to the camera. Save the settings so they are not lost at powerup.
			UINT32 config = 0;

			// Get the current IP config setting (status/mode).
			status = GevReadRegisterByName(handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);
			if (status == 0)
			{
				// Save the new settings.
				if (persistentIPMode)
				{
					// Now, write the persistent IP address and persistent subnet mask.
					status = GevWriteRegisterByName(handle, "GevPersistentIPAddress", 0, sizeof(UINT32), &IPAddress);
					status = GevWriteRegisterByName(handle, "GevPersistentSubnetMask", 0, sizeof(UINT32), &subnet_mask);

					// Mask the bits properly.
					config &= ~IPCONFIG_MODE_DHCP;
					config |= IPCONFIG_MODE_PERSISTENT;

					// Write the IP config register back.
					status = GevWriteRegisterByName(handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);

				}
				else
				{
					// Automatic mode (LLA / DHCP) - disable the persistent IP setting.
					// Mask the bits properly.
					config &= ~IPCONFIG_MODE_PERSISTENT;
					config |= IPCONFIG_MODE_DHCP;

					// Write the IP config register back.
					status = GevWriteRegisterByName(handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &config);
				}
			}
			GevCloseCamera(&handle);
		}
	}
	return status;
}



GEV_STATUS GetCameraIPSettings(GEV_CAMERA_HANDLE& handle, PUINT32 pIPAddress, PUINT32 pSubnet_mask, BOOL* pPersistentIpMode)
{
	GEV_STATUS status = -1;
	UINT32 value = 0;

	// Get the current status
	status = GevReadRegisterByName(handle, "GevCurrentIPConfiguration", 0, sizeof(UINT32), &value);
	if (status == 0)
	{
		if ((value & IPCONFIG_MODE_PERSISTENT) != 0)
		{
			// Persistent mode.
			if (pPersistentIpMode != 0)
			{
				*pPersistentIpMode = TRUE;
			}
		}
		else
		{
			// Auto mode (DHCP/LLA).
			// Persistent mode.
			if (pPersistentIpMode != 0)
			{
				*pPersistentIpMode = FALSE;
			}

		}
		// Read the current IP address and subnet mask.				
		if (pIPAddress != NULL)
		{
			status = GevReadRegisterByName(handle, "GevCurrentIPAddress", 0, sizeof(UINT32), pIPAddress);
		}
		if (pSubnet_mask != NULL)
		{
			status = GevReadRegisterByName(handle, "GevCurrentSubnetMask", 0, sizeof(UINT32), pSubnet_mask);
		}
	}
	return status;
}



/*
author:LF
date: 2020-10-24
function: This function lists all cameras detected and prints their information to stdout.
output format: Format is " <Manuf>:<Model>:<Sn>:<Version>:<DeviceUserName> aka [MAC]@[<CameraIP>] on <netname> aka [<NIC IP>] "
*/
GEV_DEVICE_INTERFACE* ListAllCameras(int level)
{
	GEV_STATUS status = -1;
	GEV_DEVICE_INTERFACE* pCamera = NULL;
	int numCamera = 0;
	int i;

	// Initialize the API.
	GevApiInitialize();

	// Allocate storage for the camera data returned from the API.
	pCamera = (GEV_DEVICE_INTERFACE*)malloc(MAX_CAMERAS * sizeof(GEV_DEVICE_INTERFACE));
	if (pCamera != NULL)
	{
		struct ifreq item = { 0 };
		int s;
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0)
		{
			perror("socket");
			return NULL;
		}

		memset(pCamera, 0, (MAX_CAMERAS * sizeof(GEV_DEVICE_INTERFACE)));
		status = GevGetCameraList(pCamera, MAX_CAMERAS, &numCamera);

		if (numCamera != 0)
		{
			if (numCamera > MAX_CAMERAS)
			{
				numCamera = MAX_CAMERAS;
			}

			// Cameras found - update list display for "numCamera" entries.		
			for (i = 0; i < numCamera; i++)
			{
				// Get the network name for the NIC
				char netname[IFNAMSIZ + 1] = { 0 };

				{
					item.ifr_ifindex = pCamera[i].host.ifIndex;
					if (ioctl(s, SIOCGIFNAME, &item) < 0)
					{
						strncpy(netname, "???", IFNAMSIZ);
					}
					else
					{
						strncpy(netname, item.ifr_name, IFNAMSIZ);
					}

				}


				// Level 1 output.
				printf("[%02X:%02X:%02X:%02X:%02X:%02X]@[%d.%d.%d.%d] on %s=[%d.%d.%d.%d] ", \
					(pCamera[i].macHigh & 0x00ff00) >> 8, (pCamera[i].macHigh & 0x00ff), \
					(pCamera[i].macLow & 0xff000000) >> 24, (pCamera[i].macLow & 0x00ff0000) >> 16, \
					(pCamera[i].macLow & 0x0000ff00) >> 8, (pCamera[i].macLow & 0x000000ff), \
					(pCamera[i].ipAddr & 0xff000000) >> 24, (pCamera[i].ipAddr & 0x00ff0000) >> 16, \
					(pCamera[i].ipAddr & 0x0000ff00) >> 8, (pCamera[i].ipAddr & 0x000000ff), \
					netname, \
					(pCamera[i].host.ipAddr & 0xff000000) >> 24, (pCamera[i].host.ipAddr & 0x00ff0000) >> 16, \
					(pCamera[i].host.ipAddr & 0x0000ff00) >> 8, (pCamera[i].host.ipAddr & 0x000000ff));

				if (level == 1)
				{
					printf("is [%s:%s]", pCamera[i].manufacturer, pCamera[i].serial);

				}
				if (level > 1)
				{
					printf("is [%s:%s:%s:ver%s]", pCamera[i].manufacturer, pCamera[i].model, pCamera[i].serial, pCamera[i].version);

				}
				if (level > 2)
				{
					printf(" aka %s", pCamera[i].username);
				}
				printf("\n");

			}
		}
		else
		{
			printf("0 cameras detected\n");
		}
		close(s);
	}
	return pCamera;
}

bool IsIpv4(char* str)
{
	char* ptr;
	int count = 0;
	const char* p = str;

	//1、判断是不是三个 ‘.’
	//2、判断是不是先导0
	//3、判断是不是四部分数
	//4、第一个数不能为0

	while (*p != '\0')
	{
		if (*p == '.')
			count++;
		p++;
	}

	if (count != 3)  return false;
	count = 0;
	ptr = strtok(str, ".");
	while (ptr != NULL)
	{
		count++;
		if (ptr[0] == '0' && isdigit(ptr[1])) return false;
		int a = atoi(ptr);
		if (count == 1 && a == 0) return false;
		if (a < 0 || a>255) return false;
		ptr = strtok(NULL, ".");

	}
	if (count == 4)  return true;
	else  return false;
}

string ip_to_hex_str(string& ip)
{

	char* ch_ip = const_cast<char*>(ip.c_str());
	char hex_ip[8] = { 0 };
	if (IsIpv4(ch_ip)) {
		struct in_addr ia_ip;
		unsigned long long_ip;
		long_ip = inet_addr(ch_ip);
		for (int i = 0; i < 4; i++)
		{
			sprintf(hex_ip + (i * 2), "%02x", *((unsigned char*)(&long_ip) + i));
		}

	}

	ostringstream ost_des;
	ost_des << "0x" << hex_ip[0] << hex_ip[1] << hex_ip[2] << hex_ip[3]
		<< hex_ip[4] << hex_ip[5] << hex_ip[6] << hex_ip[7];
	return ost_des.str();
}

unsigned long  IntToHex(int value) {
	stringstream ioss;
	ioss << std::hex << value;
	string temp;
	ioss >> temp;

	return atoi(temp.c_str());
}
