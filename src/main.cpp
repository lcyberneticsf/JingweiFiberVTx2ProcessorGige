//extern "C"
//{
//#include "userlib.h"
//}
#include "VCameraEnum.h"
#include "VCameraFrame.h"
#include "VCameraMessage.h"
#include "VCameraProcessorTypes.h"
#include "VTx2AidiModel.h"
#include "VCameraSession.h"
#include "VCameraEnumPrivate.h"
#include "communicator.h"
#include "VTx2Session.h"
#include "opencv2/opencv.hpp"
#include <array>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <thread>
#include <iostream>
#include <functional>
#include "inifile.h"
#include <string>
#include "gigevcamera.h"
#include "CLog.h"

#define IMG_WIDTH 4096
#define IMG_HEIGHT 64
#define SAVENUM	100
#define TIMEOUT_MS	1000
//void  test_grab_(void* excu);
using namespace aq;
std::queue<Vsee::InferFrame> infer_queue;
aq::gigeVCamera camera1;   //Object of Gige camera1;
aq::gigeVCamera camera2;   //Object of Gige camera2;
aq::gigeVCamera camera3;   //Object of Gige camera3;
aq::gigeVCamera camera4;   //Object of Gige camera4;
CLog  my_log;
bool b_log = false;
bool model_dispose = false;
bool mode2_dispose = false;
bool mode3_dispose = false;
bool mode4_dispose = false;

//LF :reemend 15:55

int main()
{
	//ChangeImageSize("/home/nvidia/jingweiPicture/00000090.bmp", "/home/nvidia/jingweiPicture/0.bmp", 300, 200);
	bool log = false;
	using namespace Vsee;
	using std::cout;
	aq::Communicator communitator_;
	int branck_dev = 0;

	using MutexLock = std::lock_guard<std::mutex>;

	std::mutex cout_mutex;
	
	int nsize = sizeof(std::size_t);
	nsize = sizeof(std::uint32_t);
	nsize = sizeof(std::string);

	//const char* model_path = "/home/nvidia/model/V1";   //The model path in TX2 box;
	int model_select = communitator_.parameters_setting_trans.model_select;
	//const char* model_path = "/home/aaeon/model/V1";	  //The model path in NX box;
#ifdef Xavive_NX_MODE
	std::string model_path = "/home/aaeon/model/"+std::to_string(model_select)+"/model/V1";	  //The model path in NX box;
#endif
#ifdef TX2_MODE
	std::string model_path = "/home/nvidia/model/" + std::to_string(model_select) + "/model/V1";	  //The model path in Nvidia box;
#endif

	if (log)
	{
		MutexLock lock(cout_mutex);
		cout << "model path: " << model_path << "\n";
		cout << "model loadding...\n";
	}

	VTx2AidiModel model1("FastDetection", model_path.c_str());
	//VTx2AidiModel model2("FastDetection", model_path.c_str());
	//VTx2AidiModel model3("FastDetection", model_path.c_str());
	//VTx2AidiModel model4("FastDetection", model_path.c_str());
	//VTx2AidiModel model5("FastDetection", model_path.c_str());
	//VTx2AidiModel model6("FastDetection", model_path.c_str());
	//VTx2AidiModel model7("FastDetection", model_path.c_str());
	//VTx2AidiModel model8("FastDetection", model_path.c_str());

	std::this_thread::sleep_for(std::chrono::seconds(60));		//sleep 30 seconds to initial and load the AIDI model

	if (log)
	{
		MutexLock lock(cout_mutex);
		cout << "model loaded.\n";
	}

	//std::queue<InferFrame> infer_queue;
	std::mutex             infer_mutex;
	GevAccessMode mode = GevExclusiveMode;
	auto setting_camera_server = [&](aq::gigeVCamera camera, std::string mac_adress, std::string target_ip, std::string sub_mask, GevAccessMode mode)
	{
		camera.set_camera_ip_adress(mac_adress, target_ip, sub_mask, mode);
		return;
	};

	tcp::endpoint eps[8];		//改成从配置文件读取连接相机数量时改成vector
	eps[0] = tcp::endpoint(address::from_string("192.168.1.1"), PortNumber::PRC);
	eps[1] = tcp::endpoint(address::from_string("192.168.1.12"), PortNumber::PRC);
	eps[2] = tcp::endpoint(address::from_string("192.168.1.13"), PortNumber::PRC);
	eps[3] = tcp::endpoint(address::from_string("192.168.1.14"), PortNumber::PRC);
	eps[4] = tcp::endpoint(address::from_string("192.168.1.15"), PortNumber::PRC);
	eps[5] = tcp::endpoint(address::from_string("192.168.1.16"), PortNumber::PRC);
	eps[6] = tcp::endpoint(address::from_string("192.168.1.17"), PortNumber::PRC);
	eps[7] = tcp::endpoint(address::from_string("192.168.1.18"), PortNumber::PRC);


	io_service* ios[8] = { nullptr };
	ios[0] = new io_service(1);
	ios[1] = new io_service(1);
	ios[2] = new io_service(1);
	ios[3] = new io_service(1);
	ios[4] = new io_service(1);
	ios[5] = new io_service(1);
	ios[6] = new io_service(1);
	ios[7] = new io_service(1);


	tcp::socket* sockets[8] = { nullptr };
	sockets[0] = new tcp::socket(*ios[0]);
	sockets[1] = new tcp::socket(*ios[1]);
	sockets[2] = new tcp::socket(*ios[2]);
	sockets[3] = new tcp::socket(*ios[3]);
	sockets[4] = new tcp::socket(*ios[4]);
	sockets[5] = new tcp::socket(*ios[5]);
	sockets[6] = new tcp::socket(*ios[6]);
	sockets[7] = new tcp::socket(*ios[7]);



	VTx2Session* sessions[8] = { nullptr };
															 //(beg, end]
	std::function<void(int, int, io_service&)> connect_and_reconnect = [&](int beg, int end, io_service& io)
	{
		for (int i = beg; i < end; ++i)
		{
			auto socket = sockets[i];
			
			if (!sessions[i])
			{
				sessions[i] = new VTx2Session(*socket);

				sessions[i]->setProcessFrameFunc([&, i](VCameraFrame&& frame)
				{
					if (log)
					{
						MutexLock cmlk(cout_mutex);
						cout << "process frame: " << frame.sequence() << "\n";
					}

					MutexLock lock(infer_mutex);
					infer_queue.push(InferFrame(sessions[i], std::move(frame)));
				});

				sessions[i]->setSessionAbortedFunc([&, i, beg, end]()
				{
					if(log)
					{
						MutexLock lock(cout_mutex);
						cout << eps[i].address().to_string() << " disconnected!\n";
					}

					connect_and_reconnect(beg, end, io);
				});
			}

			if (socket->is_open())
				continue;

			asio::error_code ec;
			socket->cancel(ec);

			socket->async_connect(eps[i], [&, i, beg, end](asio::error_code ec)
			{
				if (ec)
				{
					connect_and_reconnect(beg, end, io);
					return;
				}

				if(log)
				{
					MutexLock lock(cout_mutex);
					cout << eps[i].address().to_string() << " connected.\n";
				}

				sessions[i]->startSession();
			});
		}
	};
	
	auto infer_thread_func = [&](VTx2AidiModel& model)		//Use Lambda function to define a function pointer.
	{
		while (1)
		{
			InferFrame infer_frame;
			{
				MutexLock lock(infer_mutex);
				if (infer_queue.size())
				{
					
					infer_frame = std::move(infer_queue.front());
					infer_queue.pop();
				}
			}

			if (infer_frame.label == 0)
			{
				if (!infer_frame._session)
				{
					static std::chrono::milliseconds msec(1);
					std::this_thread::sleep_for(msec);
					continue;
				}

				if (log)
				{
					MutexLock cmlk(cout_mutex);
					cout << "infer session: " << infer_frame._session << " frame: " << infer_frame._frame.sequence() << "\n";
				}
			}
			else if (infer_frame.label == 1)
			{
				if (log)
				{
					MutexLock cmlk(cout_mutex);
					cout << "infer session: " << infer_frame._session << " frame: " << infer_frame._frame.sequence() << "\n";
				}

				if (0)
				{
					cv::Mat m_img(infer_frame.height, infer_frame.width, CV_8UC3, infer_frame.data);
					std::string save_path = std::string("/home/aaeon/Pictures/camera") + std::to_string(aq::gigeVCamera::image_index) + std::string(".bmp");
					aq::gigeVCamera::image_index++;
					cv::imwrite(save_path, m_img);
				}

				
#if 1			//If the m_bRunFiberDetect=true and be in the mode of detect fiber ,the output the detect result,else will not run the AIDI detect,and the image will be discarded;
				if(aq::Communicator::m_bRunFiberDetect)
					auto result = model.getFrameResult(infer_frame);
				else if(aq::Communicator::m_bGrabTrainImage)
					auto result = model.getFrameResult(infer_frame);
#endif
			}
			else {};
		}
	};
	std::thread infer1([&] { infer_thread_func(model1); });//这里起4个infer_thread_func线程
	infer1.detach();
	//pause();
	/*
	std::thread infer2([&] { infer_thread_func(model2); });
	infer2.detach();
	//pause();

	std::thread infer3([&] { infer_thread_func(model3); });
	infer3.detach();
	//pause();
	std::thread infer4([&] { infer_thread_func(model4); });
	infer4.detach();
	//pause();

	//std::thread infer5([&] { infer_thread_func(model5); });
	//infer5.detach();
	//std::thread infer6([&] { infer_thread_func(model6); });
	//infer6.detach();
	//std::thread infer7([&] { infer_thread_func(model7); });
	//infer7.detach();
	//std::thread infer8([&] { infer_thread_func(model8); });
	//infer8.detach();
	*/

	//******************************************************S*******************************************
	//To Open a camera capture thread to get the image
	//*************************************************************************************************
	//DALSA camera mac_aress:[00:01:0D:C5:20:DB]@[192.168.3.13] on eth1=[192.168.3.100]
	auto run_camera_server = [&](aq::gigeVCamera camera, std::string _ip, int argc, char** argv)
	{
		camera.create_captrue_thread(_ip, argc, argv);
		return;
	};
	//std::thread camera_thread([&] {run_camera_server(camera2, "192.168.3.13", 1, nullptr); });   //number to choose which camera
	//camera2.create_captrue_thread("192.168.3.13", 1);
	
	//*************************************************************************************************
	//To start a pointer Lumbda function of server connect to DataSimulator to receive the image  data 
	//*************************************************************************************************
	auto run_tcp_server = [&](std::string _ip, int _port, aq::Communicator &communitator)
	{
		std::string error_str;
		std::cout << "run in run_tcp_server--------------------" << std::endl;
		if (!communitator.server(_ip, _port, error_str))	//Start a Server state machine,to server for all the net request from client,
		{													//here the the function "server()" actuall is an endless loop.
			std::cerr << error_str << std::endl;
			return;
		}
	};
	//Lumbda function to start the server to wait for the camera image simulator
	//std::thread server_thread([&] { run_tcp_server("192.168.1.100", 9712, communitator_); });
	char box_ip[20];// = "192.168.1.11";
	std::uint16_t box_port = 9712;
	if (communitator_.box_index == 1)     //The box index is the NO.1 box,ip:192.168.1.100
	{
		memcpy(box_ip, communitator_.parameters_setting_trans.box1_ip,20);
		box_port = communitator_.parameters_setting_trans.box1_port;
	}
	else if (communitator_.box_index == 2)     //The box index is the NO.2 box,ip:192.168.1.101
	{
		memcpy(box_ip, communitator_.parameters_setting_trans.box2_ip, 20);
		box_port = communitator_.parameters_setting_trans.box2_port;
	}
	std::thread server_thread([&] { run_tcp_server(box_ip,box_port, communitator_); });
	//*************************************************************************************************
	//To start a pointer Lumbda function of client connect to control program to receive the image  data 
	//*************************************************************************************************
	auto run_tcp_client = [&](std::string _ip, int _port, aq::Communicator& communitator)
	{
		std::string error_str;
		std::cout << "run in run_tcp_client--------------------" << std::endl;
		if (!communitator.run_tcp_connect(_ip, _port))		//Start a Server state machine,to server for all the net request from client,
		{													//here the the function "server()" actuall is an endless loop.
			std::cerr << error_str << std::endl;
			return;
		}
	};
	//Lumbda function to start the client to connetc the control softwear;
	std::thread client_thread([&] { run_tcp_client(communitator_.parameters_setting_trans.local_ip, communitator_.parameters_setting_trans.local_port, communitator_); });


	//*************************************************************************************************
	//To start a pointer Lumbda function of client connect to magnetic valve simulator to send magnetic valve simunlator command;
	//*************************************************************************************************
	auto run_tcp_magnetic_valve = [&](std::string _ip, int _port, aq::Communicator& communitator)
	{
		std::string error_str;
		std::cout << "run in run_magnetic_valve_connect--------------------" << std::endl;
		if (!communitator.run_magnetic_valve_connect(_ip, _port))	//Start a Server state machine,to server for all the net request from client,
		{													//here the the function "server()" actuall is an endless loop.
			std::cerr << error_str << std::endl;
			return;
		}
	};
	//Lumbda function to start the magnetic_valve to connetc the control the magnetc valve driver board;
	std::thread magnetic_valve_thread([&] { run_tcp_magnetic_valve(communitator_.parameters_setting_trans.electric_board_ip, communitator_.parameters_setting_trans.electric_board_port, communitator_); });//Lumbda function to start the server

	auto run_tcp_show_img = [&](std::string _ip, int port, aq::Communicator& communitator)
	{
		std::string error_str;
		std::cout << "run in image_show_connect--------------------" << std::endl;;
		if (!communitator.run_show_image_connect(_ip, port))
		{
			std::cerr << error_str << std::endl;
			return;
		}

	};
	std::thread show_img_thread([&] {run_tcp_show_img(communitator_.parameters_setting_trans.local_ip, 9000, communitator_); });

	auto io_thread_fun = [&](int beg, int end, io_service& io)
	{
		while (1)
		{
			connect_and_reconnect(beg, end, io);
			io.run();
		}
	};

	//std::thread io1([&] { io_thread_fun(0, 1, *ios[0]); });
	//std::thread io2([&] { io_thread_fun(1, 2, *ios[1]); });
	//std::thread io3([&] { io_thread_fun(2, 3, *ios[2]); });
	//std::thread io4([&] { io_thread_fun(3, 4, *ios[3]); });
	//std::thread io5([&] { io_thread_fun(4, 5, *ios[4]); });
	//std::thread io6([&] { io_thread_fun(5, 6, *ios[5]); });
	//std::thread io7([&] { io_thread_fun(6, 7, *ios[6]); });

	//io_thread_fun(7, 8, *ios[7]); //Here the main function will run in circle endless mode in function io_thread_fun().
	
	pause();
	return 0;
}