//#include "stdafx.h"
#define LINUX
#include "communicator.h"
#include "VCameraEnum.h"
#include "VCameraMessage.h"
#include "VCameraMessageTrans.h"
#include "VCameraFrame.h"
#ifdef WINDOWS
#include<io.h>  //Windows  system  to use _findfirst()
#endif
#ifdef LINUX
#include <dirent.h>   //Linux  System to use findnext();
#endif
#include "VAsyncIO.h"
#include <thread>
#include <sstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "VCameraSession.h"
#include "inifile.h"
#include "CLog.h"


using namespace aq;
using namespace Vsee;
using namespace std;
extern aq::gigeVCamera camera1;   //Object of Gige camera1;
extern aq::gigeVCamera camera2;   //Object of Gige camera2;
extern aq::gigeVCamera camera3;   //Object of Gige camera3;
extern aq::gigeVCamera camera4;   //Object of Gige camera4;
extern std::queue<InferFrame> infer_queue;

bool	aq::Communicator::batch_image_load = true;
bool	aq::Communicator::single_image_load = false;
bool	aq::Communicator::m_bServerSendMode = false;
bool	aq::Communicator::m_bClientSendMode = false;
bool	aq::Communicator::m_bImageClientRun = false;
bool	aq::Communicator::m_bMageneticValveClientSendMode = true;
bool	aq::Communicator::m_bMagneticValveClientRun = false;
bool	aq::Communicator::m_bServerRun = true;
bool	aq::Communicator::m_bClientRun = true;
bool	aq::Communicator::m_bRunFiberDetect = true;
bool     aq::Communicator::m_bGrabTrainImage = false;
bool     aq::Communicator::m_bShowImageConnetc = false;
bool     aq::Communicator::m_bWriteLog=false;
Vsee::VCameraMessageTrans  aq::Communicator::box_ctrl_msg;          //use to transmit control parameters from server to box;
Vsee::VCameraMessageTrans_new  aq::Communicator::box_ctrl_msg_2;    //use to transmit control parameters from server to box;
Vsee::VCameraMessageTrans*  aq::Communicator::valve_ctrl_msg;        //use to transmit magenetic valve control parameters from server to box;
Vsee::VCameraMessageTrans* aq::Communicator::show_img_msg;         //Used to transmit control defect image display 
asio::basic_stream_socket<asio::ip::tcp>*  aq::Communicator::magnetic_valve_socket_;
asio::basic_stream_socket<asio::ip::tcp>* aq::Communicator::show_image_socket_;
std::mutex    aq::Communicator::mutex_magnetic_valve;
CParameterSetting	aq::Communicator::parameters_setting_trans;
bool     aq::Communicator::m_bTcpMode=false;
asio::ip::udp::socket* aq::Communicator::magnetic_valve_udp_socket_;
asio::ip::udp::endpoint* aq::Communicator::end_point;

static const size_t BUF_SIZE = 1024;


std::vector<std::string> aq::Communicator::split(std::string str, char seg)
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

void aq::Communicator::print_hex(unsigned char *_buf, int _len)
{
    for (int i = 0 ; i < _len ; i++)
    {
        printf("%02x ", _buf[i]);
    }
    std::cout << std::endl;
}

aq::Communicator::Communicator()
{
	//infer_frame = new Vsee::InferFrame(1);
	/*
	ios[0] = new asio::io_service(1);
	ios[1] = new asio::io_service(1);
	ios[2] = new asio::io_service(1);
	ios[3] = new asio::io_service(1);
	ios[4] = new asio::io_service(1);
	ios[5] = new asio::io_service(1);
	ios[6] = new asio::io_service(1);
	ios[7] = new asio::io_service(1);

	sockets[0] = new tcp::socket(*ios[0]);//将socket->io_service变量设置为对应的ios[i]，在VCameraSession<Derived>::sendMessage(VCameraMessage && msg)中要用到io_service来发送信息
	sockets[1] = new tcp::socket(*ios[1]);
	sockets[2] = new tcp::socket(*ios[2]);
	sockets[3] = new tcp::socket(*ios[3]);
	sockets[4] = new tcp::socket(*ios[4]);
	sockets[5] = new tcp::socket(*ios[5]);
	sockets[6] = new tcp::socket(*ios[6]);
	sockets[7] = new tcp::socket(*ios[7]);

	sessions[0] = new Vsee::VTx2Session(*sockets[0]);
	sessions[1] = new Vsee::VTx2Session(*sockets[1]);
	using MutexLock = std::lock_guard<std::mutex>;

	eps[0] = tcp::endpoint(address::from_string("127.0.0.1"), 60000);
	eps[1] = tcp::endpoint(address::from_string("127.0.0.1"), 60000);
	eps[2] = tcp::endpoint(address::from_string("192.168.1.13"), 60000);
	eps[3] = tcp::endpoint(address::from_string("192.168.1.14"), 60000);
	eps[4] = tcp::endpoint(address::from_string("192.168.1.15"), 60000);
	eps[5] = tcp::endpoint(address::from_string("192.168.1.16"), 60000);
	eps[6] = tcp::endpoint(address::from_string("192.168.1.17"), 60000);
	eps[7] = tcp::endpoint(address::from_string("192.168.1.18"), 60000);
	*/
	//frame_queue_ = new std::queue<Vsee::VCameraFrame>;
	aq::Communicator::valve_ctrl_msg = new Vsee::VCameraMessageTrans;
	aq::Communicator::m_bMagneticValveClientRun = false;
	aq::Communicator::show_img_msg = new Vsee::VCameraMessageTrans;
	aq::Communicator::m_bImageClientRun = false;
	ReadIni();
}


bool aq::Communicator::tcp_connect(std::string _ip, int _port, std::string &_error_str)
{
    _error_str.clear();
    // check input
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
    if ((_port < 0) || (_port > 65535))
    {
        _error_str = std::string("Invalid Port. ") + std::to_string(_port) + " is out of range.";
        return false;
    }

	io_service_ = new asio::io_service;
	ep_ = new asio::ip::tcp::endpoint(asio::ip::address::from_string(_ip), _port);
	socket_ = new asio::ip::tcp::socket(*io_service_);
	asio::error_code ec;
	
	socket_->connect(*ep_, ec);
	if (ec)
	{
		_error_str = "The connection could have timed out.";
		return false;
	}
	data_buffer_.clear();
	data_buffer_.resize(DATA_BUFFER_LENGTH);
    _error_str = "Ok.";
    return true;
}


bool aq::Communicator::magnetic_valve_connect(std::string _ip, int _port, std::string& _error_str)
{
	_error_str.clear();
	// check input
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
	if ((_port < 0) || (_port > 65535))
	{
		_error_str = std::string("Invalid Port. ") + std::to_string(_port) + " is out of range.";
		return false;
	}
	if (m_bTcpMode)  //If the network mode is TCP  mode
	{
		asio::io_service* io_service_;
		io_service_ = new asio::io_service;
		ep_ = new asio::ip::tcp::endpoint(asio::ip::address::from_string(_ip), _port);
		magnetic_valve_socket_ = new asio::ip::tcp::socket(*io_service_);
		asio::error_code ec;

		magnetic_valve_socket_->connect(*ep_, ec);
		if (ec)
		{
			_error_str = "The connection could have timed out.";
			return false;
		}
	}
	else      //If the network mode is UDP  mode
	{
		asio::io_service ios;
		//asio::ip::udp::socket sock(ios);
		magnetic_valve_udp_socket_ = new asio::ip::udp::socket(ios);
		//asio::ip::udp::endpoint end_point(asio::ip::address::from_string(_ip), 9001);
		end_point = new asio::ip::udp::endpoint(ip::address::from_string(_ip), 9001);
		//sock.open(end_point.protocol());
		magnetic_valve_udp_socket_->open(end_point->protocol());
		
	}
	data_buffer_.clear();
	data_buffer_.resize(DATA_BUFFER_LENGTH);
	_error_str = "ok.";
	return true;
}

bool aq::Communicator::receive_img(std::string &_error_str)
{
	
	_error_str.clear();
	std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
	VCameraMessageTrans* msg_in = (VCameraMessageTrans*)data_buffer_.data();
	int width = msg_in->width;
	int height = msg_in->height;
	int channels = msg_in->channels;
	int data_size = width * height * channels;
	int signalling = msg_in->signalling;
	int magnetic_valve_num = msg_in->magnetic_valve_num;
	VCameraMessageTrans* MsgPost = new VCameraMessageTrans;
	memcpy(MsgPost, msg_in, sizeof(VCameraMessageTrans));
	show_ctrl_message(socket_,signalling, (void*)MsgPost);

	std::string strMsg = std::string("receive_img: signalling=") + std::to_string(signalling) + std::string(" ,magnetic_valve_num = ") + std::to_string(magnetic_valve_num);
	std::cout << strMsg << std::endl;

	std::int64_t next = Vsee::VCameraMessageTrans::nextLoad(data_buffer_.data(), data_buffer_.size());
	next = data_size;

	if (signalling == 20)
	{
		asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), next));
		//std::cout << "next : " << next << std::endl;
		//print_hex((unsigned char *)data_buffer_.c_str() + size, next);
		cv::Mat img(height, width, CV_8UC3, (void*)((char*)data_buffer_.data()));
		//cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
		std::string file_path = "/home/nvidia/save_image/" + std::to_string(counter) + ".bmp";
		cv::imwrite(file_path, img);
		std::cout << "image save:" << file_path << std::endl;
		counter++;
		InferFrame infer_frame;
		infer_frame.width = width;
		infer_frame.height = height;
		infer_frame.channels = channels;
		infer_frame.label = 1;
		int data_length = width * height * channels;
		infer_frame.data = new char[width * height * channels];
		memcpy(infer_frame.data, (char*)data_buffer_.data(), data_length);
		infer_queue.push(infer_frame);
	}
	else
	{
	}
	if (MsgPost != nullptr)
	{
		delete MsgPost;
		MsgPost = nullptr;
	}
	
    return true;
}


#define RECEIVE_BUF_SIZE 100
#define RECEIVE_BYTE_NUM 30
int aq::Communicator::readMaxBytesInTime(asio::ip::tcp::socket& socket, char* strBuf, int nMaxBytes, int nMilSec)
{
	//boost::timer t;
	int nTotalRec = 0;
	int nLeftBytes = nMaxBytes - nTotalRec;
	while (1)
	{
		asio::error_code ec;
		//std::string ec="";
		char buf[RECEIVE_BUF_SIZE];

		int nWantBytes = 0;
		if (nLeftBytes < RECEIVE_BUF_SIZE)
		{
			nWantBytes = nLeftBytes;
		}
		else
		{
			nWantBytes = RECEIVE_BUF_SIZE;
		}

		size_t len = socket.read_some(asio::buffer(buf, nWantBytes), ec);
		if (len > 0)
		{
			memcpy(strBuf + nTotalRec, buf, len);
			nTotalRec += len;
			nLeftBytes -= len;

			if (nLeftBytes <= 0)
				break;
			else
				continue;
		}
		else
		{
			//if (t.elapsed() * 1000 < nMilSec)
			{
				Sleep(0);
				continue;
			}
			//else
			//	break;
		}
	}
	return nTotalRec;
}


int aq::Communicator::streamReadMaxBytesInTime(asio::basic_stream_socket<asio::ip::tcp>& socket, char* strBuf, int nMaxBytes, int nMilSec)
{
	//boost::timer t;
	int nTotalRec = 0;
	int nLeftBytes = nMaxBytes - nTotalRec;
	while (1)
	{
		asio::error_code ec;
		//std::string ec="";
		char buf[RECEIVE_BUF_SIZE];

		int nWantBytes = 0;
		if (nLeftBytes < RECEIVE_BUF_SIZE)
		{
			nWantBytes = nLeftBytes;
		}
		else
		{
			nWantBytes = RECEIVE_BUF_SIZE;
		}

		size_t len = socket.read_some(asio::buffer(buf, nWantBytes), ec);
		if (len > 0)
		{
			memcpy(strBuf + nTotalRec, buf, len);
			nTotalRec += len;
			nLeftBytes -= len;

			if (nLeftBytes <= 0)
				break;
			else
				continue;
		}
		else
		{
			//if (t.elapsed() * 1000 < nMilSec)
			{
				//Sleep(0);
				//continue;
				nTotalRec = 0;
				return nTotalRec;
			}
			//else
			//	break;
		}
	}
	return nTotalRec;
}

bool aq::Communicator::receive_img(std::string &_error_str, asio::basic_stream_socket<asio::ip::tcp>* socket_)
{

	_error_str.clear();	
	//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
	std::int64_t size =streamReadMaxBytesInTime(*socket_, (char*)data_buffer_.data(), sizeof(VCameraMessageTrans), 100);
	//size_t len = socket_.read_some(buffer(buf, nWantBytes), ec);
	if (size == 0)
		return false;
	VCameraMessageTrans* msg_in = (VCameraMessageTrans*)data_buffer_.data();
	std::int64_t next = 0;
	int width = msg_in->width;
	int height = msg_in->height;
	int channels = msg_in->channels;
	int data_size = width * height * channels;
	int signalling = msg_in->signalling;
	int magnetic_valve_num = msg_in->magnetic_valve_num;
	if (signalling != 20)
	{
		memcpy(&box_ctrl_msg, msg_in, sizeof(VCameraMessageTrans));
		show_ctrl_message(socket_, signalling, (void*)&box_ctrl_msg);
	}
	else if (signalling == 20)
	{	
		next = data_size;
		//memcpy(valve_ctrl_msg, msg_in, sizeof(VCameraMessageTrans));
		memcpy(&box_ctrl_msg, msg_in, sizeof(VCameraMessageTrans));
		asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), data_size));

		if (0)
		{
			cv::Mat img(height, width, CV_8UC3, (void*)((char*)data_buffer_.data()));
			//cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
			std::string file_path = "/home/nvidia/save_image/" + std::to_string(counter) + ".bmp";
			cv::imwrite(file_path, img);
			std::cout << "image save:" << file_path << std::endl;
			counter++;
		}

		InferFrame infer_frame;
		infer_frame.width = width;
		infer_frame.height = height;
		infer_frame.channels = channels;
		infer_frame.label = 1;
		infer_frame.camera_serial = magnetic_valve_num; //To indicate camera serial number by variable "magnetic_valve_num".

		int data_length = width * height * channels;
		infer_frame.data = new char[width * height * channels];
		memcpy(infer_frame.data, (char*)data_buffer_.data(), data_length);
		MutexLock lock(infer_mutex);
		infer_queue.push(infer_frame);
	}
	else
	{
	}
	return true;
}

bool aq::Communicator::send_cmd(aq::CMD _cmd, std::string &_error_str, std::vector<int> _state, int _sequence)
{
	
    _error_str.clear();
    if (_cmd == CMD::SEND_RESULTS)
    {
        std::uint32_t result[2];
        result[0] = _sequence;
		std::uint32_t state = 0;
        for (int i = 0 ; i < 8 ; i++)
        {
            if (i < _state.size())
            {
				state += _state[i] * std::pow(16, 7 - i);
            }
            else
            {
            }
        }
		result[1] = state;

		Vsee::VCameraMessageTrans response((int)_cmd, (const char*)result, sizeof(result));
		std::cout << "send_cmd================================>> : " << _sequence << std::endl;
		print_hex((unsigned char*)response.data(), response.dataSize());
		std::cout << "<<================================send_cmd" << std::endl;
		//const std::string &tmp = response.;
		auto buf = asio::buffer(response.bytes(), response.byteSize());
		//std::this_thread::sleep_for(std::chrono::milliseconds(20));
		socket_->write_some(buf);
    }
    else
    {
		std::uint32_t result[1];
		result[0] = _sequence;
		//for (int i = 0; i < 8; i++)
		//{
		//	if (i < _state.size())
		//	{
		//		result[i + 1] = _state[i];
		//	}
		//	else
		//	{
		//		result[i + 1] = 0;
		//	}
		//}

		Vsee::VCameraMessageTrans response((int)_cmd, nullptr, 0);
		socket_->write_some(asio::buffer(response.data(), response.dataSize()));
    }
	
    return true;
}

//************************************************************************************************
//author :	LF
//time:		2020-9-16
//parameters:
//			std::string _ip : the local server ip adress
//			int _port : the local server  net port for machine server 
//function: To start a Server state machine,to wait for all the net request from client,and when require income,
//			then start a server thread for the net connect;
//************************************************************************************************
bool aq::Communicator::server(std::string _ip, int _port, std::string &_error_str)
{
	
	_error_str.clear();
	// check input
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
	if ((_port < 0) || (_port > 65535))
	{
		_error_str = std::string("Invalid Port. ") + std::to_string(_port) + " is out of range.";
		return false;
	}


	asio::error_code ec;
	//asio::io_service ios;
	
	while (true)
	{
		asio::io_service* io_service_server;
		io_service_server = new asio::io_service;
		ep_ = new asio::ip::tcp::endpoint(asio::ip::address::from_string(_ip), _port);
		asio::basic_stream_socket<asio::ip::tcp>* socket_server;
		socket_server = new asio::ip::tcp::socket(*io_service_server);
		asio::ip::tcp::acceptor acceptor(*io_service_server, *ep_);
		
		acceptor.accept(*socket_server);            //When a net connect is requseted,then start a server thrread for the connection;
		{
			std::thread io1([&] { server_run(socket_server,_ip, _port, _error_str); });
			io1.detach();
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		//服务器端程序里要注意的是自由函数buffer(),它可用包装很多种类的容器成为asio组件可用的缓冲区类型。通常我们不能直接把数组，vector等容器用作asio的读写参数，必须使用buffer()函数包装。
	}
	if (ec)
	{
		_error_str = "The connection could have timed out.";
		return false;
	}
	
	return true;
}


bool aq::Communicator::server_run(asio::basic_stream_socket<asio::ip::tcp>* socket_, std::string _ip, int _port, std::string &_error_str)
{
	size_t size = DATA_BUFFER_LENGTH;
	data_buffer_.clear();
	data_buffer_.resize(DATA_BUFFER_LENGTH);
	
	if (m_bServerSendMode)  //if the mode is server sending,and client receive image 
	{
		Vsee::VCameraFrame VFrame;
		cv::Mat image;
		image = cv::imread("/home/nvidia/" + std::to_string(1) + ".bmp");
		int nWidth = image.cols;
		int nHeight = image.rows;
		int nChannnels = image.channels();
		//size_t size = 10000000;
		char* buffer = new char[size];
		//	VCameraMessagePrivate* pre = new VCameraMessagePrivate();
		VCameraMessageTrans msg(buffer, size);
		int n_size = sizeof(VCameraMessageTrans);
		int n_size2 = sizeof(msg);
		msg.width = image.cols;
		msg.height = image.rows;
		msg.channels = image.channels();
		int length = nWidth*nHeight*nChannnels;
		//data_buffer_.clear();
		//data_buffer_.resize(DATA_BUFFER_LENGTH);
		//get_file_list(file_load_path);
		int file_size = file_list.size();
		std::string file_path;


		msg.setData((char*)image.data, nWidth*nHeight*nChannnels, true);
		//sessions[0]->sendMessage(msg);

		//Vsee::VCameraFrame frame;// (std::move(msg));
		//nWidth = frame.width();
		//nHeight = frame.height();
		int file_total = file_list.size();
		int file_countr = 0;
		while (file_countr < file_total - 1)
		{
			if (aq::Communicator::batch_image_load)
			{
				file_path = file_list[file_countr];
				file_countr++;
				image = cv::imread(file_path);
				nWidth = image.cols;
				nHeight = image.rows;
				nChannnels = image.channels();
				msg.width = image.cols;
				msg.height = image.rows;
				msg.channels = image.channels();
				length = nWidth*nHeight*nChannnels;

				//data_buffer_.clear();
				//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
				size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
				VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
				nWidth = msg2->width;
				nHeight = msg2->height;
				int nSignalling = msg2->signalling;

				if (size != 0)
				{
					//sessions[0]->sendMessage(msg);
					{
						auto& io = socket_->get_io_service();
						if (!socket_->is_open())
							return false;
						asio::error_code er;
						auto buf = asio::buffer(&msg, sizeof(msg));
						int nLenght = msg.byteSize();
						socket_->write_some(buf);

						//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
						buf = asio::buffer(image.data, length);

						//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
						//asio::write(*_socket, buf, NULL, &er);
						socket_->write_some(buf);
					}
					std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;
					
				}
				else
				{
					//sleep(1000);
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}
				int sleep_times = 100 / camera_scan_speed;
				//sleep(sleep_times);
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			else if (aq::Communicator::single_image_load)
			{
				aq::Communicator::single_image_load = false;
				file_path = file_list[file_countr];
				file_countr++;
				image = cv::imread(file_path);
				nWidth = image.cols;
				nHeight = image.rows;
				nChannnels = image.channels();
				msg.width = image.cols;
				msg.height = image.rows;
				msg.channels = image.channels();
				length = nWidth*nHeight*nChannnels;

				//data_buffer_.clear();
				//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
				size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
				VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
				nWidth = msg2->width;
				nHeight = msg2->height;
				int nSignalling = msg2->signalling;

				if (size != 0)
				{
					//sessions[0]->sendMessage(msg);
					{
						auto& io = socket_->get_io_service();
						if (!socket_->is_open())
							return false;
						asio::error_code er;
						auto buf = asio::buffer(&msg, sizeof(msg));
						int nLenght = msg.byteSize();
						socket_->write_some(buf);

						//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
						buf = asio::buffer(image.data, length);

						//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
						//asio::write(*_socket, buf, NULL, &er);
						socket_->write_some(buf);
					}
					std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;
				}
				else
				{
					//sleep(1000);
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}
			}
			else
			{
				//sleep(300);
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}

		}
	}
	else  //the mode is server receive,and clietn send image;
	{
		VCameraMessageTrans msg;
		std::string error_str;
		msg.width = 115;
		msg.height = 115;
		msg.signalling = 20;
		int nsize = sizeof(msg);	//sizeof(VCameraMessageTrans)=512,here must keep all the windows and Linux programe this data structrue to be same length;
		counter = 0;
//		send_cmd(aq::CMD::START_RECV_IMG, error_str);
		try
		{
			while (1)
			{
				//std::cout << "execute to Communicator:552" << std::endl;
				auto buf = asio::buffer(&msg, sizeof(msg));
				socket_->write_some(buf);
				std::string s;
				if (!receive_img(s, socket_))
				{
					std::cerr << s << std::endl;
				}
			}
		}
		catch (std::invalid_argument e)
		{
			return false;
			
		}
		catch (...)
		{
			return false;
		}	

	}
	
	return true;
}
bool aq::Communicator::get_file_list(std::string file_load_path)
{
	
	//CString m_strFolderPath;
	//CString m_strFileExt;
	char strFilePath[256];
	memset(strFilePath, 0, 256);
	strcpy(strFilePath, file_load_path.data());
	int nNameLength = strlen(strFilePath);
	char strFileExt[8];
	memset(strFileExt, 0, 8);
	int i = 0;
	int nDotIndex = 0;
	file_amount = 0;
	for (i = nNameLength - 1; i > 0; i--)
	{
		char ch = strFilePath[i];
		if (ch == '.')
			break;
	}
	strncpy(strFileExt, strFilePath + i + 1, nNameLength - i - 1);
	//m_strFileExt.Format("%s", strFileExt);
	for (i = nNameLength - 1; i > 0; i--)
	{
		char ch = strFilePath[i];
		if (ch == '\\')
			break;
	}
	strFilePath[i] = '\0';
	//m_strFolderPath.Format("%s", strFilePath);
	//std::string strFolderPath = m_strFolderPath;
	std::string strFolderPath;
#ifdef WINDOWS
	get_files(strFolderPath, strFileExt, file_amount);
#endif
#ifdef LINUX
	find_dir_file(strFilePath, file_list);
#endif
	
	return true;
}


//**************************************************************************************************
//author :	LF
//time:		2020-9-24
//FUNC:		get_files(std::string fileFolderPath, std::string fileExtension, int& nFileNum)
//parameters:
//			std::string _ip : the local server ip adress
//			int _port : the local server  net port for machine server 
//function: In windows ststem,to fand all the files int the fold of fileFolderPath,and push the file name into file_list;
//**************************************************************************************************
int aq::Communicator::get_files(std::string fileFolderPath, std::string fileExtension, int& nFileNum)
{
#ifdef WINDOWS

	std::string fileFolder = fileFolderPath + "\\*" + fileExtension;
	std::string fileName;
	struct _finddata_t fileInfo;
	nFileNum = 0;
	long long findResult = _findfirst(fileFolder.c_str(), &fileInfo);
	if (findResult == -1)
	{
		_findclose(findResult);
		
		return 0;
	}
	bool flag = 0;

	do
	{
		fileName = fileFolderPath + "\\" + fileInfo.name;
		if (fileInfo.attrib == _A_ARCH)
		{
			file_list.push_back(fileName);
			nFileNum++;
		}
	}// while (_findnext(findResult, &fileInfo) == 0);
	while (_findnext(findResult, &fineInfo) == 0);
	
	_findclose(findResult);
#endif // WINDOWS
	return 0;
}

//*****************************************************************************************************
//author :	LF
//time:		2020-9-24
//FUNC:		find_dir_file(const char* dir_name, std::vector<std::string>& v)
//parameters:
//			const char* dir_name : Folder to find the files
//			std::vector<std::string>& v : the file list that write all the file name into 
//function: In Linux ststem,to fand all the files int the fold of fileFolderPath,and push the file name into file_list;
//*******************************************************************************************************
int aq::Communicator::find_dir_file(const char* dir_name, std::vector<std::string>& v)     //文件夹地址，文件列表
{
#ifdef LINUX
	DIR* dirp;
	struct dirent* dp;
	dirp = opendir(dir_name);
	while ((dp = readdir(dirp)) != NULL) {
		v.push_back(std::string(dp->d_name));
	}
	(void)closedir(dirp);
#endif  //LINUX
	return 0;

}

bool  aq::Communicator::run_tcp_connect(std::string _ip, int _port)
{
	std::thread t(&aq::Communicator::tcp_connect_thread, this ,_ip, _port);
	//std::thread t(&aq::Communicator::server_run, this, socket_, _ip, _port, _error_str);
	t.detach();
	return true;
}


bool  aq::Communicator::run_magnetic_valve_connect(std::string _ip, int _port)
{
	std::thread t(&aq::Communicator::magnetic_valve_connect_thread, this, _ip, _port);
	
	t.detach();
	return true;
}

bool  aq::Communicator::run_show_image_connect(std::string _ip, int _port)
{
	std::thread t(&aq::Communicator::image_show_connect_thread, this, _ip, _port);

	t.detach();
	return true;
}


bool aq::Communicator::image_show_connect_thread(std::string _ip, int _port)
{
	while (true)
	{
		if (!m_bImageClientRun)
		{
			try
			{
				std::string error_str;
				if (!show_image_connect(_ip, _port, error_str))
				{
					std::cerr << error_str << std::endl;
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					continue;
				}
				m_bImageClientRun = true;
				std::cout << "connect to image_show_connect server, connected " << _ip << ":" << _port << std::endl;

			}
			catch (...)
			{
			}
			//m_bClientRun = false;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		
	}
	return true;
}


bool aq::Communicator::tcp_connect_thread(std::string _ip, int _port)
{
	bool tcp_connetc = false;

	if (!m_bClientSendMode)		// if mode is client receive,and server send image mode
	{
		while (true)
		{
			try
			{

				if (!tcp_connetc)
				{
					try
					{
						std::string error_str;
						if (!tcp_connect(_ip, _port, error_str))
						{
							std::cerr << error_str << std::endl;
							//return false;
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
							continue;

						}
						tcp_connetc = true;
						m_bClientRun = true;
						std::cout << "connect to control server, connected " << _ip << ":" << _port << std::endl;
						VCameraMessageTrans msg;
						msg.width = 115;
						msg.height = 115;
						msg.signalling = 11;
						counter = 0;
						//send_cmd(aq::CMD::START_RECV_IMG, error_str);
						while (m_bClientRun)
						{
							auto buf = asio::buffer(&msg, sizeof(msg));
							socket_->write_some(buf);
							std::string s;
							//if (!receive_img(s))
							if (!receive_img(s, socket_))
							{
								std::cerr << s << std::endl;
								tcp_connetc = false;
								m_bClientRun = false;
								break;
							}
						}
					}
					catch (...)
					{
						tcp_connetc = false;
						m_bClientRun = false;
					}
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				}
			}
			catch(...)
			{
				;
			}	
		}

	}
	else if (m_bClientSendMode)  //if the mode is client sending,and server receive image 
	{
		try
		{
			std::string error_str;
			if (!tcp_connect(_ip, _port, error_str))
			{
				std::cerr << error_str << std::endl;
				return false;
			}
			Vsee::VCameraFrame VFrame;
			cv::Mat image;
			image = cv::imread("f:/" + std::to_string(1) + ".bmp");
			int nWidth = image.cols;
			int nHeight = image.rows;
			int nChannnels = image.channels();
			size_t size = 10000000;
			char* buffer = new char[size];
			//	VCameraMessagePrivate* pre = new VCameraMessagePrivate();
			VCameraMessageTrans msg(buffer, size);
			int n_size = sizeof(VCameraMessageTrans);
			int n_size2 = sizeof(msg);
			msg.width = image.cols;
			msg.height = image.rows;
			msg.channels = image.channels();
			int length = nWidth * nHeight * nChannnels;
			//data_buffer_.clear();
			//data_buffer_.resize(1000000);
			//get_file_list(file_load_path);
			int file_size = file_list.size();
			std::string file_path;

			msg.setData((char*)image.data, nWidth * nHeight * nChannnels, true);
			//sessions[0]->sendMessage(msg);

			//Vsee::VCameraFrame frame;// (std::move(msg));
			//nWidth = frame.width();
			//nHeight = frame.height();
			int file_total = file_list.size();
			int file_countr = 0;
			//VCameraMessageTrans* msg = new VCameraMessageTrans;
			while (m_bClientRun)
			{
				if (box_ctrl_msg.signalling == 0)		 //Idle time,keep loop;
				{
					//sleep(200);
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					continue;
				}
				else if (box_ctrl_msg.signalling == 1)       //Start detect fiber
				{

					auto& io = socket_->get_io_service();
					if (!socket_->is_open())
						return false;
					asio::error_code er;
					auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
					int nLenght = msg.byteSize();
					socket_->write_some(buf);
					box_ctrl_msg.signalling = 0;
				}
				else if (box_ctrl_msg.signalling == 2)       //Stop detect fiber
				{
					auto& io = socket_->get_io_service();
					if (!socket_->is_open())
						return false;
					asio::error_code er;
					auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
					int nLenght = msg.byteSize();
					socket_->write_some(buf);
					box_ctrl_msg.signalling = 0;
				}
				else if (box_ctrl_msg.signalling == 3)       //Start test  electro magnetic valve
				{                                            //box_ctrl_msg.magnetic_valve_num: the serial number of electro magnetic valve;
					auto& io = socket_->get_io_service();
					if (!socket_->is_open())
						return false;
					asio::error_code er;
					auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
					int nLenght = msg.byteSize();
					socket_->write_some(buf);
					box_ctrl_msg.signalling = 0;
				}
				else if (box_ctrl_msg.signalling == 4)       //To set camera detect parameters
				{
					auto& io = socket_->get_io_service();
					if (!socket_->is_open())
						return false;
					asio::error_code er;
					auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
					int nLenght = msg.byteSize();
					socket_->write_some(buf);
					box_ctrl_msg.signalling = 0;
				}
				else if (box_ctrl_msg.signalling == 20)
				{
					while (file_countr < file_total - 1)
					{
						if (aq::Communicator::batch_image_load)
						{
							file_path = file_list[file_countr];
							file_countr++;
							image = cv::imread(file_path);
							nWidth = image.cols;
							nHeight = image.rows;
							nChannnels = image.channels();
							msg.width = image.cols;
							msg.height = image.rows;
							msg.channels = image.channels();
							length = nWidth * nHeight * nChannnels;

							//data_buffer_.clear();
							//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
							size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
							VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
							nWidth = msg2->width;
							nHeight = msg2->height;
							int nSignalling = msg2->signalling;

							if (size != 0)
							{
								//sessions[0]->sendMessage(msg);
								{
									auto& io = socket_->get_io_service();
									if (!socket_->is_open())
										return false;
									asio::error_code er;
									auto buf = asio::buffer(&msg, sizeof(msg));
									int nLenght = msg.byteSize();
									socket_->write_some(buf);

									//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
									buf = asio::buffer(image.data, length);

									//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
									//asio::write(*_socket, buf, NULL, &er);
									socket_->write_some(buf);
								}
								std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;

							}
							else
							{
								//sleep(1000);
								std::this_thread::sleep_for(std::chrono::milliseconds(200));
							}
							int sleep_times = 100 / camera_scan_speed;
							//sleep(sleep_times);
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
						}
						else if (aq::Communicator::single_image_load)
						{
							aq::Communicator::single_image_load = false;
							file_path = file_list[file_countr];
							file_countr++;
							image = cv::imread(file_path);
							nWidth = image.cols;
							nHeight = image.rows;
							nChannnels = image.channels();
							msg.width = image.cols;
							msg.height = image.rows;
							msg.channels = image.channels();
							length = nWidth * nHeight * nChannnels;

							//data_buffer_.clear();
							//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
							size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
							VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
							nWidth = msg2->width;
							nHeight = msg2->height;
							int nSignalling = msg2->signalling;

							if (size != 0)
							{
								//sessions[0]->sendMessage(msg);
								{
									auto& io = socket_->get_io_service();
									if (!socket_->is_open())
										return false;
									asio::error_code er;
									auto buf = asio::buffer(&msg, sizeof(msg));
									int nLenght = msg.byteSize();
									socket_->write_some(buf);

									//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
									buf = asio::buffer(image.data, length);

									//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
									//asio::write(*_socket, buf, NULL, &er);
									socket_->write_some(buf);
								}
								std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;
							}
							else
							{
								//sleep(1000);
								std::this_thread::sleep_for(std::chrono::milliseconds(200));
							}
						}
						else
						{
							//sleep(300);
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
						}

					}

				}
			}
		}
		catch (...)
		{
		}
		m_bClientRun = false;

	}
	else
	{
		;
	}

	return true;
}


//***************************************************************************************************************
//Author :LF
//time:   2020-10-10
//function: To process with the command from the control computer
//signalling command table and communication protocol
//1:   Start  fiber detect
//2:   Stop	fiber detect
//3:   To test the magnetic valve
//4:	To receive and write the camera detect parameters into ini-setting file and reset the detect parameters
//5:   To receive and write the boxes network parameters and cameras network and device parameters into ini-setting 
//	 file and reset the device network parameters and camera device parameters;
//20:  To receive image from camera simulator software and push the image into vector queue;
//*****************************************************************************************************************
int aq::Communicator::show_ctrl_message(asio::basic_stream_socket<asio::ip::tcp>* socket,int  wParam, void* lParam)
{
	std::string strMsg;
	int nCtrlSignalling = (int)wParam;
	Vsee::VCameraMessageTrans* MagneticVolveMsg = (Vsee::VCameraMessageTrans*)lParam;
	int recv_len = 0;
	int nCameraSerialNum = MagneticVolveMsg->magnetic_valve_num;	//Use value "magnetic_valve_num" to indicate camera serial number
	if (MagneticVolveMsg->parameters_setting_trans.box_num == parameters_setting_trans.box_num)
	{
		try
		{

			if (nCtrlSignalling == 1)   //To start the fiber detect
			{
				aq::Communicator::m_bRunFiberDetect = true;    //Begin to run in fiber detecting mode;
				std::cout << "Begin detect fiber." << std::endl;
			}
			else if (nCtrlSignalling == 2)
			{
				aq::Communicator::m_bRunFiberDetect = false;    //Begin to stop the fiber detecting mode;
				std::cout << "Stop detect fiber." << std::endl;
			}
			else if (nCtrlSignalling == 3)      //To test the magnetic valve hardware;
			{
				MutexLock lock(mutex_magnetic_valve);
				memcpy(aq::Communicator::valve_ctrl_msg, MagneticVolveMsg, sizeof(Vsee::VCameraMessageTrans));
				_time_calib_req_tag_* alib_req_tag;
				_ack_tag_* ack_tag;
				_io_output_transac_tag_* valve_driver_frame;
				//auto& io = socket->get_io_service();
				if (m_bTcpMode)   //The communication  is base TCP/IP mode
				{
					if (!magnetic_valve_socket_->is_open())
						return false;
					asio::error_code er;
					auto buf = asio::buffer(aq::Communicator::valve_ctrl_msg, sizeof(Vsee::VCameraMessageTrans));
					magnetic_valve_socket_->write_some(buf);
				}
				else       //The communication  is base UDP mode
				{
					if (!magnetic_valve_udp_socket_->is_open())
						return false;
					asio::error_code er;
					if (0)  //In self define protocol mode
					{
						auto buf = asio::buffer(aq::Communicator::valve_ctrl_msg, sizeof(Vsee::VCameraMessageTrans));
						magnetic_valve_udp_socket_->send_to(buf, *end_point);
					}
					else  //In Leihang protocal mode
					{
						_io_output_transac_tag_  valve_driver_frame;
						_time_calib_req_tag_	valve_calib_frame;
						//std::uint16_t cameraA_delay = 200;
						//std::uint16_t cameraA_hold = 30;
						valve_calib_frame.type = 0x80000010;
						valve_calib_frame.src = 0xCC880000;
						auto buf = asio::buffer(&valve_calib_frame, sizeof(valve_calib_frame));
						magnetic_valve_udp_socket_->send_to(buf, *end_point);
						recv_len = magnetic_valve_udp_socket_->receive_from(asio::buffer((char*)data_buffer_.data(), sizeof(_io_output_transac_tag_)), *end_point);
						if (recv_len == sizeof(_time_calib_req_tag_))
						{
							;
						}
						else if (recv_len == sizeof(_ack_tag_))
						{
							ack_tag = (_ack_tag_*)data_buffer_.data();
							valve_driver_frame.ref = ack_tag->ack_val;
						}

						switch (aq::Communicator::valve_ctrl_msg->magnetic_valve_num)   //Here the value 'valve_ctrl_msg->magnetic_valve_num' in fact is the camera serial number
						{
						case 1:
							valve_driver_frame.delay = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraA_delay * 10000;
							valve_driver_frame.dura = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraA_hold * 10000;
							break;
						case 2:
							valve_driver_frame.delay = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraB_delay * 10000;
							valve_driver_frame.dura = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraB_hold * 10000;
							break;
						case 3:
							valve_driver_frame.delay = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraC_delay * 10000;
							valve_driver_frame.dura = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraC_hold * 10000;
							break;
						case 4:
							valve_driver_frame.delay = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraD_delay * 10000;
							valve_driver_frame.dura = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraD_hold * 10000;
							break;
						default:
							valve_driver_frame.delay = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraA_delay * 10000;
							valve_driver_frame.dura = aq::Communicator::valve_ctrl_msg->parameters_setting_trans.cameraA_hold * 10000;
							break;
						}
						for (int i = 0; i < MAGNETIC_VALVE_SUM; i++)
						{
							if (aq::Communicator::valve_ctrl_msg->magnetic_valve[i] == 1)
							{

								valve_driver_frame.type = 0x80000010;    // 0x80000010 （请求）
								valve_driver_frame.src = 0xCC880004;   // 0xCC880004（请求类型：为I/O驱动请求）
								//valve_driver_frame.ref = 0x10000;						
								valve_driver_frame.port = i;			// Electric Valve serial number
								//valve_driver_frame.delay;				// delay time
								//valve_driver_frame.dura;				// Hold time
								auto buf = asio::buffer(&valve_driver_frame, sizeof(valve_driver_frame));
								magnetic_valve_udp_socket_->send_to(buf, *end_point);
							}
						}

					}
				}
				valve_ctrl_msg->signalling = 0;
				std::cout << "To test Magnetic Valve hardwear******************************;" << std::endl;
			}
			else if (nCtrlSignalling == 4)     //To set the camera detect parameters;
			{
				WriteIni(MagneticVolveMsg->parameters_setting_trans);
				memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				switch (MagneticVolveMsg->parameters_setting_trans.signalling)
				{
				case 0:
					std::cout << "0: To update  all detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 1:
					std::cout << "1: To set  base detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 2:
					std::cout << "2: To set  debug  parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 3:
					std::cout << "3: To set  magnetic_valve  parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 4:
					std::cout << "4: To set  cameraA detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 5:
					std::cout << "5: To set  cameraB detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 6:
					std::cout << "6: To set  cameraC detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 7:
					std::cout << "7: To set  cameraD detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				case 8:
					std::cout << "8: To set  camera&box  device parameters settting>>>>>>>>>>>>>>>>>>>>>>>>;" << std::endl;
					break;
				default:
					break;
				}
			}
			else if (nCtrlSignalling == 5)     //To set the camera network and device parameters and write into ini-setting file;
			{
				WriteIni(MagneticVolveMsg->parameters_setting_trans);
				memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));

				switch (nCameraSerialNum)
				{
				case 1:
					camera1.index_num = nCameraSerialNum;
					set_camera_params(camera1, MagneticVolveMsg->parameters_setting_trans);
					std::cout << "1:  To set  cameraA detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
					break;
				case 2:
					camera2.index_num = nCameraSerialNum;
					set_camera_params(camera2, MagneticVolveMsg->parameters_setting_trans);
					std::cout << "2:  To set  cameraB detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
					break;
				case 3:
					camera3.index_num = nCameraSerialNum;
					set_camera_params(camera3, MagneticVolveMsg->parameters_setting_trans);
					std::cout << "3:  To set  cameraC detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
					break;
				case 4:
					camera4.index_num = nCameraSerialNum;
					set_camera_params(camera4, MagneticVolveMsg->parameters_setting_trans);
					std::cout << "4:  To set  cameraD detect parameters settting>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
					break;
				default:
					break;
				}
			}
			else if (nCtrlSignalling == 6)      //To set model seletct parameters;
			{
				WriteIni(MagneticVolveMsg->parameters_setting_trans);
				memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				std::cout << "To set modele select parameters." << std::endl;
				//****************Here need to reload the AI model and restart the detect thread***************************
			}
			else if (nCtrlSignalling == 7)     //To start grab image and save image.
			{
				memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				std::cout << "To start grab image and save image." << std::endl;
				aq::Communicator::m_bGrabTrainImage = true;
				aq::Communicator::m_bRunFiberDetect = false;

			}
			else if (nCtrlSignalling == 8)     //To stop grab image and save image.
			{
				memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				std::cout << "To stop grab image and save image." << std::endl;
				aq::Communicator::m_bGrabTrainImage = false;
				aq::Communicator::m_bRunFiberDetect = true;
			}
			else if (nCtrlSignalling == 9)     //To upload  grab image to server.
			{
				//memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				std::cout << "To upload  grab image to server." << std::endl;
			}
			else if (nCtrlSignalling == 11)     //To start a camera grab thread.
			{
				switch (nCameraSerialNum)
				{
				case 1:
					open_camera(camera1);
					break;
				case 2:
					open_camera(camera2);
					break;
				case 3:
					open_camera(camera3);
					break;
				case 4:
					open_camera(camera4);
					break;
				default:
					break;
				}

				std::cout << "To start a camera grab thread." << std::endl;
			}
			else if (nCtrlSignalling == 12)     //To stop a camera grab thread.
			{
				//memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				switch (nCameraSerialNum)
				{
				case 1:
					stop_camera(camera1);
					break;
				case 2:
					stop_camera(camera2);
					break;
				case 3:
					stop_camera(camera3);
					break;
				case 4:
					stop_camera(camera4);
					break;
				default:
					break;
				}
				std::cout << "To stop a camera grab thread." << std::endl;
			}
			else if (nCtrlSignalling == 13)     //To start a camera grab image mode.
			{
				//memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				switch (nCameraSerialNum)
				{
				case 1:
					start_grab(camera1);
					break;
				case 2:
					start_grab(camera2);
					break;
				case 3:
					start_grab(camera3);
					break;
				case 4:
					start_grab(camera4);
					break;
				default:
					break;
				}
				std::cout << "To start a camera grab mode." << std::endl;
			}
			else if (nCtrlSignalling == 14)     //To stop a camera grab image mode.
			{
				//memcpy(&parameters_setting_trans, &MagneticVolveMsg->parameters_setting_trans, sizeof(parameters_setting_trans));
				switch (nCameraSerialNum)
				{
				case 1:
					stop_grab(camera1);
					break;
				case 2:
					stop_grab(camera2);
					break;
				case 3:
					stop_grab(camera3);
					break;
				case 4:
					stop_grab(camera4);
					break;
				default:
					break;
				}
				std::cout << "To stop a camera grab mode." << std::endl;
			}
		}
		catch (...)
		{
			m_bMagneticValveClientRun = false;
		}
	}
	else
	{
		std::cout << "The setting of AI BOX  is not this one>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
	}

	return 1;
}


int aq::Communicator::show_ctrl_message(asio::basic_stream_socket<asio::ip::tcp>* socket, int  wParam, void* lParam, int length, uchar* m_buf)
{
	std::string strMsg;
	int nCtrlSignalling = (int)wParam;
	Vsee::VCameraMessageTrans* ImageMsg = (Vsee::VCameraMessageTrans*)lParam;

	try
	{

		if (nCtrlSignalling == 20)      //To test the magnetic valve hardware;
		{
			memcpy(aq::Communicator::show_img_msg, ImageMsg, sizeof(Vsee::VCameraMessageTrans));
			auto& io = show_image_socket_->get_io_service();
			//if (!show_image_socket_->is_open())
			if(!m_bShowImageConnetc)
				return false;
			asio::error_code er;
			std::cout << "this is size " << sizeof(Vsee::VCameraMessageTrans) << std::endl;
			auto buf = asio::buffer(aq::Communicator::show_img_msg, sizeof(Vsee::VCameraMessageTrans));
			socket->write_some(buf);
			auto buf1 = asio::buffer(m_buf, length);
			//sleep(80);
			socket->write_some(buf1);
			aq::Communicator::show_img_msg->signalling = 0;
			std::cout << "=============================To send defect Image=======================;" << std::endl;
		}
	}
	catch (...)
	{
		m_bImageClientRun = false;
		m_bShowImageConnetc = false;
	}

	return 1;
}


/********************************************************************************************************
Author :LF
time:   2020-10-26
function: To process with the command from the control compurator,to set the camera device parameters
int camera_index:the series number of the camera;
          1--cameraA; 2--cameraB;  3----cameraC;  4-----cameraD;
aq::gigeVCamera camera: The object of the camera
CParameterSetting parameters_setting_trans: The camera parameters transfer from the contral program; 
***********************************************************************************************************/
void	aq::Communicator::set_camera_params(aq::gigeVCamera &camera, CParameterSetting parameters_setting_trans)
{
	int width;
	int height;
	int exposure_time;
	int gain;
	int trigger;
	int camera_index = camera.index_num;
	//camera_index += 1;
	string camera_ip;

	if (camera_index == 1)
	{
		camera_ip=parameters_setting_trans.cameraA_ip;
		width = parameters_setting_trans.cameraA_width;
		height = parameters_setting_trans.cameraA_height;
		exposure_time = parameters_setting_trans.cameraA_exposure_time;
		gain = parameters_setting_trans.cameraA_gain;
		trigger = parameters_setting_trans.cameraA_trigger_source;
	}
	else if (camera_index == 2)
	{
		camera_ip = parameters_setting_trans.cameraB_ip;
		width = parameters_setting_trans.cameraB_width;
		height = parameters_setting_trans.cameraB_height;
		exposure_time = parameters_setting_trans.cameraB_exposure_time;
		gain = parameters_setting_trans.cameraB_gain;
		trigger = parameters_setting_trans.cameraB_trigger_source;
	}
	else if (camera_index == 3)
	{
		camera_ip = parameters_setting_trans.cameraC_ip;
		width = parameters_setting_trans.cameraC_width;
		height = parameters_setting_trans.cameraC_height;
		exposure_time = parameters_setting_trans.cameraC_exposure_time;
		gain = parameters_setting_trans.cameraC_gain;
		trigger = parameters_setting_trans.cameraC_trigger_source;
	}
	else if (camera_index == 4)
	{
		camera_ip = parameters_setting_trans.cameraD_ip;
		width = parameters_setting_trans.cameraD_width;
		height = parameters_setting_trans.cameraD_height;
		exposure_time = parameters_setting_trans.cameraD_exposure_time;
		gain = parameters_setting_trans.cameraD_gain;
		trigger = parameters_setting_trans.cameraD_trigger_source;
	}
	else
	{
		return;
	}
	std::cout << "beging set camera parameters." << std::endl;
	if(!camera.b_camera_open)
		camera.handle = camera.create_camera_handle(camera_ip, 1);
	camera.set_camera_width(camera.handle, width);
	camera.set_camera_height(camera.handle, height);
	camera.set_camera_exposuretime(camera.handle, exposure_time);
	camera.set_camera_gain(camera.handle, gain);
	trigger = 0;
	camera.set_camer_trigger(camera.handle, trigger);
	return;
}


bool aq::Communicator::magnetic_valve_connect_thread(std::string _ip, int _port)
{

	if (m_bMageneticValveClientSendMode)  //if the mode is client sending,and server receive image 
	{
		while (true)
		{
			if (!m_bMagneticValveClientRun)
			{
				try
				{
					std::string error_str;
					if (!magnetic_valve_connect(_ip, _port, error_str))
					{
						std::cerr << error_str << std::endl;
						//return false;
						std::this_thread::sleep_for(std::chrono::milliseconds(300));
						continue;
					}
					m_bMagneticValveClientRun = true;
					std::cout << "connect to magnetic_valve_connect server, connected " << _ip << ":" << _port << std::endl;
					//Vsee::VCameraFrame VFrame;
					cv::Mat image;
					image = cv::imread("f:/" + std::to_string(1) + ".bmp");
					int nWidth = image.cols;
					int nHeight = image.rows;
					int nChannnels = image.channels();
					
					while (m_bClientRun && false)  //	Here the code will not execute!!!!!!!!
					{
						try {
							if (valve_ctrl_msg->signalling == 0)		 //Idle time,keep loop;
							{
								//sleep(200);
								std::this_thread::sleep_for(std::chrono::microseconds(2000));
								continue;
							}
							/*else if (valve_ctrl_msg->signalling == 1)       //Start detect fiber
							{

								auto& io = magnetic_valve_socket_->get_io_service();
								if (!socket_->is_open())
									return false;
								asio::error_code er;
								auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
								int nLenght = msg.byteSize();
								magnetic_valve_socket_->write_some(buf);
								box_ctrl_msg.signalling = 0;
							}
							else if (valve_ctrl_msg->signalling == 2)       //Stop detect fiber
							{
								auto& io = socket_->get_io_service();
								if (!socket_->is_open())
									return false;
								asio::error_code er;
								auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
								int nLenght = msg.byteSize();
								socket_->write_some(buf);
								box_ctrl_msg.signalling = 0;
							}*/
							else if (valve_ctrl_msg->signalling == 3)       //Start test  electro magnetic valve
							{

								//box_ctrl_msg.magnetic_valve_num: the serial number of electro magnetic valve;
								auto& io = magnetic_valve_socket_->get_io_service();
								if (!magnetic_valve_socket_->is_open())
									return false;
								asio::error_code er;

								//std::int64_t size = asio::read(*magnetic_valve_socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
								//VCameraMessageTrans* msg_in = (VCameraMessageTrans*)data_buffer_.data();

								auto buf = asio::buffer(valve_ctrl_msg, sizeof(VCameraMessageTrans));
								magnetic_valve_socket_->write_some(buf);
								valve_ctrl_msg->signalling = 0;
							}
							else if (box_ctrl_msg.signalling == 4)       //To set camera detect parameters
							{
								auto& io = socket_->get_io_service();
								if (!socket_->is_open())
									return false;
								asio::error_code er;
								auto buf = asio::buffer(&box_ctrl_msg, sizeof(box_ctrl_msg));
								//int nLenght = msg.byteSize();
								socket_->write_some(buf);
								box_ctrl_msg.signalling = 0;
							}
							else if (box_ctrl_msg.signalling == 20)
							{
								size_t size = 10000000;
								char* buffer = new char[size];
								VCameraMessageTrans msg(buffer, size);
								int n_size = sizeof(VCameraMessageTrans);
								int n_size2 = sizeof(msg);
								msg.width = image.cols;
								msg.height = image.rows;
								msg.channels = image.channels();
								int length = nWidth * nHeight * nChannnels;
								msg.signalling = 3;
								msg.magnetic_valve[2] = 1;
								msg.magnetic_valve_num = 3;
								
								int file_size = file_list.size();
								std::string file_path;
								msg.setData((char*)image.data, nWidth * nHeight * nChannnels, true);
								
								int file_total = file_list.size();
								int file_countr = 0;
								while (file_countr < file_total - 1)
								{
									if (aq::Communicator::batch_image_load)
									{
										file_path = file_list[file_countr];
										file_countr++;
										image = cv::imread(file_path);
										nWidth = image.cols;
										nHeight = image.rows;
										nChannnels = image.channels();
										msg.width = image.cols;
										msg.height = image.rows;
										msg.channels = image.channels();
										length = nWidth * nHeight * nChannnels;

										//data_buffer_.clear();
										//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
										size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
										VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
										nWidth = msg2->width;
										nHeight = msg2->height;
										int nSignalling = msg2->signalling;

										if (size != 0)
										{
											//sessions[0]->sendMessage(msg);
											{
												auto& io = socket_->get_io_service();
												if (!socket_->is_open())
													return false;
												asio::error_code er;
												auto buf = asio::buffer(&msg, sizeof(msg));
												int nLenght = msg.byteSize();
												socket_->write_some(buf);

												//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
												buf = asio::buffer(image.data, length);

												//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
												//asio::write(*_socket, buf, NULL, &er);
												socket_->write_some(buf);
											}
											std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;

										}
										else
										{
											//sleep(1000);
											std::this_thread::sleep_for(std::chrono::milliseconds(200));
										}
										int sleep_times = 100 / camera_scan_speed;
										//sleep(sleep_times);
										std::this_thread::sleep_for(std::chrono::milliseconds(200));
									}
									else if (aq::Communicator::single_image_load)
									{
										aq::Communicator::single_image_load = false;
										file_path = file_list[file_countr];
										file_countr++;
										image = cv::imread(file_path);
										nWidth = image.cols;
										nHeight = image.rows;
										nChannnels = image.channels();
										msg.width = image.cols;
										msg.height = image.rows;
										msg.channels = image.channels();
										length = nWidth * nHeight * nChannnels;

										//data_buffer_.clear();
										//std::int64_t size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), 8));
										size = asio::read(*socket_, asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)));
										VCameraMessageTrans* msg2 = (VCameraMessageTrans*)data_buffer_.data();
										nWidth = msg2->width;
										nHeight = msg2->height;
										int nSignalling = msg2->signalling;

										if (size != 0)
										{
											//sessions[0]->sendMessage(msg);
											{
												auto& io = socket_->get_io_service();
												if (!socket_->is_open())
													return false;
												asio::error_code er;
												auto buf = asio::buffer(&msg, sizeof(msg));
												int nLenght = msg.byteSize();
												socket_->write_some(buf);

												//auto buf = asio::buffer(msg.bytes(), msg.byteSize());
												buf = asio::buffer(image.data, length);

												//asio::async_write(_socket, buf, [keep = std::move(msg)](asio::error_code, std::size_t){});
												//asio::write(*_socket, buf, NULL, &er);
												socket_->write_some(buf);
											}
											std::cout << "client:ip:" << socket_->remote_endpoint().address() << "   port:" << socket_->remote_endpoint().port() << std::endl;
										}
										else
										{
											//sleep(1000);
											std::this_thread::sleep_for(std::chrono::microseconds(200));
										}
									}
									else
									{
										//sleep(300);
										std::this_thread::sleep_for(std::chrono::microseconds(200));
									}

								}
								delete buffer;
								buffer = nullptr;

							}
							else
							{
								//sleep(300);
								std::this_thread::sleep_for(std::chrono::microseconds(2000));
							}
						}
						catch (...)
						{

						}
					}
				}
				catch (...)
				{
					m_bMagneticValveClientRun = false;
				}
				//m_bClientRun = false;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
	}
	else
	{
		;
	}

	return true;
}


bool aq::Communicator::show_image_connect(std::string _ip, int _port, std::string& _error_str)
{
	_error_str.clear();
	// check input
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
	if ((_port < 0) || (_port > 65535))
	{
		_error_str = std::string("Invalid Port. ") + std::to_string(_port) + " is out of range.";
		return false;
	}
	asio::io_service* io_service_;
	io_service_ = new asio::io_service;
	ep_ = new asio::ip::tcp::endpoint(asio::ip::address::from_string(_ip), _port);
	show_image_socket_ = new asio::ip::tcp::socket(*io_service_);
	asio::error_code ec;

	show_image_socket_->connect(*ep_, ec);
	if (ec)
	{
		_error_str = "The connection could have timed out.";
		return false;
	}
	m_bShowImageConnetc = true;
	data_buffer_.clear();
	data_buffer_.resize(DATA_BUFFER_LENGTH);
	_error_str = "ok.";
	return true;
}


void aq::Communicator::WriteIni(CParameterSetting parameters_setting_trans)
{
	//char IniRead[255];
	//memset(IniRead, 0, 255);
	char IniPath[255];
	memset(IniPath, 0, 255);

	strcat(IniPath, "./param.ini");
	char IniWrite[255];
	memset(IniWrite, 0, 255);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.model_select);
	WritePrivateProfileString("System", "model_select", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_begin", IniWrite, IniPath);
	//parameters_setting_trans.cameraA_begin = atoi(IniWrite);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_end", IniWrite, IniPath);
	//parameters_setting_trans.cameraA_end = atoi(IniWrite);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_align_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_align_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_align_begin", IniWrite, IniPath);
	//parameters_setting_trans.cameraA_align_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_align_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_align_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_align_end", IniWrite, IniPath);
	//parameters_setting_trans.cameraA_align_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_detect_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_detect_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_detect_begin", IniWrite, IniPath);
	//parameters_setting_trans.cameraA_detect_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_detect_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_detect_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_detect_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_valve_amount);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_valve_amount);
	WritePrivateProfileString("BaseDetectParameter", "cameraA_valve_amount", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_align_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_align_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_align_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_align_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_align_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_align_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_detect_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_detect_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_detect_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_detect_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_detect_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_detect_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_valve_amount);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_valve_amount);
	WritePrivateProfileString("BaseDetectParameter", "cameraB_valve_amount", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_align_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_align_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_align_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_align_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_align_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_align_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_detect_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_detect_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_detect_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_detect_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_detect_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_detect_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_valve_amount);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_valve_amount);
	WritePrivateProfileString("BaseDetectParameter", "cameraC_valve_amount", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_align_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_align_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_align_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_align_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_align_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_align_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_detect_begin);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_detect_begin);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_detect_begin", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_detect_end);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_detect_end);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_detect_end", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_valve_amount);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_valve_amount);
	WritePrivateProfileString("BaseDetectParameter", "cameraD_valve_amount", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_delay);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_delay);
	WritePrivateProfileString("ValveParameter", "cameraA_delay", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_hold);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_hold);
	WritePrivateProfileString("ValveParameter", "cameraA_hold", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_delay);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_delay);
	WritePrivateProfileString("ValveParameter", "cameraB_delay", IniWrite, IniPath);
	//-----------------------------------------------------------------------------------------

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_hold);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_hold);
	WritePrivateProfileString("ValveParameter", "cameraB_hold", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_delay);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_delay);
	WritePrivateProfileString("ValveParameter", "cameraC_delay", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_hold);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_hold);
	WritePrivateProfileString("ValveParameter", "cameraC_hold", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_delay);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_delay);
	WritePrivateProfileString("ValveParameter", "cameraD_delay", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_hold);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_hold);
	WritePrivateProfileString("ValveParameter", "cameraD_hold", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.valve_work_times);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.valve_work_times);
	WritePrivateProfileString("ValveParameter", "valve_work_times", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.valve_stop_time);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.valve_stop_time);
	WritePrivateProfileString("ValveParameter", "valve_stop_time", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_area_min);
	WritePrivateProfileString("DetectParameter", "cameraA_point_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_area_max);
	WritePrivateProfileString("DetectParameter", "cameraA_point_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_length_min);
	WritePrivateProfileString("DetectParameter", "cameraA_point_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_length_max);
	WritePrivateProfileString("DetectParameter", "cameraA_point_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_width_min);
	WritePrivateProfileString("DetectParameter", "cameraA_point_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_point_width_max);
	WritePrivateProfileString("DetectParameter", "cameraA_point_width_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_area_min);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_area_max);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_length_min);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_length_max);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_width_min);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_thread_width_max);
	WritePrivateProfileString("DetectParameter", "cameraA_thread_width_max", IniWrite, IniPath);

	//Camera B detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_area_min);
	WritePrivateProfileString("DetectParameter", "cameraB_point_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_area_max);
	WritePrivateProfileString("DetectParameter", "cameraB_point_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_length_min);
	WritePrivateProfileString("DetectParameter", "cameraB_point_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_length_max);
	WritePrivateProfileString("DetectParameter", "cameraB_point_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_width_min);
	WritePrivateProfileString("DetectParameter", "cameraB_point_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_point_width_max);
	WritePrivateProfileString("DetectParameter", "cameraB_point_width_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_area_min);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_area_max);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_length_min);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_length_max);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_width_min);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_thread_width_max);
	WritePrivateProfileString("DetectParameter", "cameraB_thread_width_max", IniWrite, IniPath);


	//Camera C detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_area_min);
	WritePrivateProfileString("DetectParameter", "cameraC_point_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_area_max);
	WritePrivateProfileString("DetectParameter", "cameraC_point_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_length_min);
	WritePrivateProfileString("DetectParameter", "cameraC_point_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_length_max);
	WritePrivateProfileString("DetectParameter", "cameraC_point_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_width_min);
	WritePrivateProfileString("DetectParameter", "cameraC_point_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_point_width_max);
	WritePrivateProfileString("DetectParameter", "cameraC_point_width_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_area_min);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_area_max);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_length_min);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_length_max);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_width_min);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_width_min", IniWrite, IniPath);
\
	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_thread_width_max);
	WritePrivateProfileString("DetectParameter", "cameraC_thread_width_max", IniWrite, IniPath);


	//Camera D detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_area_min);
	WritePrivateProfileString("DetectParameter", "cameraD_point_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_area_max);
	WritePrivateProfileString("DetectParameter", "cameraD_point_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_length_min);
	WritePrivateProfileString("DetectParameter", "cameraD_point_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_length_max);
	WritePrivateProfileString("DetectParameter", "cameraD_point_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_width_min);
	WritePrivateProfileString("DetectParameter", "cameraD_point_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_point_width_max);
	WritePrivateProfileString("DetectParameter", "cameraD_point_width_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_area_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_area_min);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_area_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_area_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_area_max);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_area_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_length_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_length_min);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_length_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_length_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_length_max);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_length_max", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_width_min);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_width_min);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_width_min", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_width_max);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_thread_width_max);
	WritePrivateProfileString("DetectParameter", "cameraD_thread_width_max", IniWrite, IniPath);


	//DebugParaneters setting;
	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraA);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.debug_cameraA);
	WritePrivateProfileString("DebugParameter", "debug_cameraA", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraB);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.debug_cameraB);
	WritePrivateProfileString("DebugParameter", "debug_cameraB", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraC);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.debug_cameraC);
	WritePrivateProfileString("DebugParameter", "debug_cameraC", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraD);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.debug_cameraD);
	WritePrivateProfileString("DebugParameter", "debug_cameraD", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.stop_volve);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.stop_volve);
	WritePrivateProfileString("DebugParameter", "stop_volve", IniWrite, IniPath);

	//IniWrite.Format("%d", parameters_setting_trans.save_blob);
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.save_blob);
	WritePrivateProfileString("DebugParameter", "save_blob", IniWrite, IniPath);

	/*************************************************************************************/
	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraD_ip);
	WritePrivateProfileString("CameraSetting", "cameraD_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_port);
	WritePrivateProfileString("CameraSetting", "cameraD_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_scan_speed);
	WritePrivateProfileString("CameraSetting", "cameraD_scan_speed", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_tirgging_mode);
	WritePrivateProfileString("CameraSetting", "cameraD_tirgging_mode", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_exposure_time);
	WritePrivateProfileString("CameraSetting", "cameraD_exposure_time", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_gain);
	WritePrivateProfileString("CameraSetting", "cameraD_gain", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_width);
	WritePrivateProfileString("CameraSetting", "cameraD_width", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraD_height);
	WritePrivateProfileString("CameraSetting", "cameraD_height", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraD_mac);
	WritePrivateProfileString("CameraSetting", "cameraD_mac", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraC_ip);
	WritePrivateProfileString("CameraSetting", "cameraC_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_port);
	WritePrivateProfileString("CameraSetting", "cameraC_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_scan_speed);
	WritePrivateProfileString("CameraSetting", "cameraC_scan_speed", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_tirgging_mode);
	WritePrivateProfileString("CameraSetting", "cameraC_tirgging_mode", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_exposure_time);
	WritePrivateProfileString("CameraSetting", "cameraC_exposure_time", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_gain);
	WritePrivateProfileString("CameraSetting", "cameraC_gain", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_width);
	WritePrivateProfileString("CameraSetting", "cameraC_width", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraC_height);
	WritePrivateProfileString("CameraSetting", "cameraC_height", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraC_mac);
	WritePrivateProfileString("CameraSetting", "cameraC_mac", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraB_ip);
	WritePrivateProfileString("CameraSetting", "cameraB_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_port);
	WritePrivateProfileString("CameraSetting", "cameraB_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_scan_speed);
	WritePrivateProfileString("CameraSetting", "cameraB_scan_speed", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_tirgging_mode);
	WritePrivateProfileString("CameraSetting", "cameraB_tirgging_mode", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_exposure_time);
	WritePrivateProfileString("CameraSetting", "cameraB_exposure_time", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_gain);
	WritePrivateProfileString("CameraSetting", "cameraB_gain", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_width);
	WritePrivateProfileString("CameraSetting", "cameraB_width", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraB_height);
	WritePrivateProfileString("CameraSetting", "cameraB_height", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraB_mac);
	WritePrivateProfileString("CameraSetting", "cameraB_mac", IniWrite, IniPath);


	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraA_ip);
	WritePrivateProfileString("CameraSetting", "cameraA_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_port);
	WritePrivateProfileString("CameraSetting", "cameraA_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_scan_speed);
	WritePrivateProfileString("CameraSetting", "cameraA_scan_speed", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_tirgging_mode);
	WritePrivateProfileString("CameraSetting", "cameraA_tirgging_mode", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_exposure_time);
	WritePrivateProfileString("CameraSetting", "cameraA_exposure_time", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_gain);
	WritePrivateProfileString("CameraSetting", "cameraA_gain", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_width);
	WritePrivateProfileString("CameraSetting", "cameraA_width", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.cameraA_height);
	WritePrivateProfileString("CameraSetting", "cameraA_height", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.cameraA_mac);
	WritePrivateProfileString("CameraSetting", "cameraA_mac", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.local_ip);
	WritePrivateProfileString("System", "local_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.local_port);
	WritePrivateProfileString("System", "local_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.electric_board_ip);
	WritePrivateProfileString("System", "electric_board_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.electric_board_port);
	WritePrivateProfileString("System", "electric_board_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.box2_ip);
	WritePrivateProfileString("System", "box2_ip", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.box2_port);
	WritePrivateProfileString("System", "box2_port", IniWrite, IniPath);

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%s", parameters_setting_trans.box1_ip);
	WritePrivateProfileString("System", "box1_ip", IniWrite, IniPath);	

	memset(IniWrite, 0, 255);
	sprintf(IniWrite, "%d", parameters_setting_trans.box1_port);
	WritePrivateProfileString("System", "box1_port", IniWrite, IniPath);
	return;
}


//***********************************************************************************************
//author:LF
//date: 2020-10-10
//function:Read device parameters from ini setting files,adn read parameters into structrue object: parameters_setting_trans
//ini file path:"./param.ini"
//***********************************************************************************************
void aq::Communicator::ReadIni()
{
	int camera_total;
	char IniRead[255];
	memset(IniRead, 0, 255);
	char IniPath[255];
	memset(IniPath, 0, 255);
	//camera_ip_info ci_info;
	//strcpy(IniPath, 255, strPath.GetBuffer());
	strcpy(IniPath, "./param.ini");

	GetPrivateProfileString("System", "box1_ip", "", IniRead, 24, IniPath);
	//m_dlgNetworkParameters.GetDlgItem(IDC_EDIT_IP_ADRESS)->SetWindowTextA(IniRead);
	GetPrivateProfileString("System", "box1_port", "", IniRead, 24, IniPath);
	//m_dlgNetworkParameters.GetDlgItem(IDC_EDIT_IP_PORT_NUMBER)->SetWindowTextA(IniRead);
	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box_num", "", IniRead, 24, IniPath);
	parameters_setting_trans.box_num = atoi(IniRead);

	parameters_setting_trans.signalling = 1;     //Use siggnalling = 1 to indicate send the network setting command;

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_end);
	//memset(IniRead, 0, 255);
	//GetPrivateProfileString("BaseDetectParameter", "cameraA_end", IniRead,24 IniPath);
	//parameters_setting_trans.cameraA_end = atoi(IniRead);

	//****************************To read the device parameters form ini files*****************************
	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "write_log", "", IniRead, 24, IniPath);
	m_bWriteLog = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box_index", "", IniRead, 24, IniPath);
	box_index = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "model_select", "", IniRead, 24, IniPath);
	parameters_setting_trans.model_select = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box1_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.box1_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box1_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.box1_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box2_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.box2_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "box2_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.box2_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "electric_board_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.electric_board_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "electric_board_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.electric_board_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "local_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.local_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("System", "local_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.local_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraA_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_scan_speed", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_scan_speed = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_tirgging_mode", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_tirgging_mode = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_exposure_time", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_exposure_time = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_gain", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_gain = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_width", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_width = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_height", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_height = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraA_mac", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraA_mac, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraB_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_scan_speed", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_scan_speed = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_tirgging_mode", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_tirgging_mode = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_exposure_time", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_exposure_time = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_gain", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_gain = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_width", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_width = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "camerB_height", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_height = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraB_mac", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraB_mac, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraC_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_scan_speed", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_scan_speed = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_tirgging_mode", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_tirgging_mode = atoi(IniRead);
	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_exposure_time", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_exposure_time = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_gain", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_gain = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_width", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_width = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_height", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_height = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraC_mac", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraC_mac, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_ip", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraD_ip, IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_port", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_port = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_scan_speed", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_scan_speed = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_tirgging_mode", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_tirgging_mode = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_exposure_time", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_exposure_time = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_gain", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_gain = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_width", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_width = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_height", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_height = atoi(IniRead);

	memset(IniRead, 0, 255);
	GetPrivateProfileString("CameraSetting", "cameraD_mac", "", IniRead, 24, IniPath);
	strcpy(parameters_setting_trans.cameraD_mac, IniRead);
	//****************************End read the device parameters form ini files*****************************

	//****************************To read the fiber detect parameters form ini files************************
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_begin", "",IniRead,24, IniPath);
	parameters_setting_trans.cameraA_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_align_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_align_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_align_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_align_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_align_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_align_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_align_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_align_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_detect_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_detect_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_detect_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_detect_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_detect_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_detect_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_detect_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_detect_begin = atoi(IniRead);//-----------------------------------------------

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_valve_amount);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_valve_amount);
	GetPrivateProfileString("BaseDetectParameter", "cameraA_valve_amount", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_valve_amount = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_align_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_align_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_align_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_align_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_align_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_align_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_align_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_align_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_detect_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_detect_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_detect_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_detect_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_detect_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_detect_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_detect_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_detect_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_valve_amount);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_valve_amount);
	GetPrivateProfileString("BaseDetectParameter", "cameraB_valve_amount", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_valve_amount = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_align_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_align_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_align_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_align_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_align_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_align_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_align_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_align_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_detect_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_detect_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_detect_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_detect_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_detect_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_detect_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_detect_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_detect_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_valve_amount);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_valve_amount);
	GetPrivateProfileString("BaseDetectParameter", "cameraC_valve_amount", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_valve_amount = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_align_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_align_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_align_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_align_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_align_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_align_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_align_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_align_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_detect_begin);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_detect_begin);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_detect_begin", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_detect_begin = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_detect_end);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_detect_end);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_detect_end", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_detect_end = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_valve_amount);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_valve_amount);
	GetPrivateProfileString("BaseDetectParameter", "cameraD_valve_amount", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_valve_amount = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_delay);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_delay);
	GetPrivateProfileString("ValveParameter", "cameraA_delay", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_delay = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_hold);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_hold);
	GetPrivateProfileString("ValveParameter", "cameraA_hold", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_hold = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_delay);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_delay);
	GetPrivateProfileString("ValveParameter", "cameraB_delay", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_delay = atoi(IniRead);
	//-----------------------------------------------------------------------------------------

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_hold);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_hold);
	GetPrivateProfileString("ValveParameter", "cameraB_hold", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_hold = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_delay);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_delay);
	GetPrivateProfileString("ValveParameter", "cameraC_delay", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_delay = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_hold);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_hold);
	GetPrivateProfileString("ValveParameter", "cameraC_hold", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_hold = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_delay);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_delay);
	GetPrivateProfileString("ValveParameter", "cameraD_delay", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_delay = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_hold);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_hold);
	GetPrivateProfileString("ValveParameter", "cameraD_hold", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_hold = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.valve_work_times);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.valve_work_times);
	GetPrivateProfileString("ValveParameter", "valve_work_times", "", IniRead,24, IniPath);
	parameters_setting_trans.valve_work_times = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.valve_stop_time);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.valve_stop_time);
	GetPrivateProfileString("ValveParameter", "valve_stop_time", "", IniRead, 24, IniPath);
	parameters_setting_trans.valve_stop_time = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_area_min);
	GetPrivateProfileString("DetectParameter", "cameraA_point_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_area_max);
	GetPrivateProfileString("DetectParameter", "cameraA_point_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_length_min);
	GetPrivateProfileString("DetectParameter", "cameraA_point_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_length_max);
	GetPrivateProfileString("DetectParameter", "cameraA_point_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_width_min);
	GetPrivateProfileString("DetectParameter", "cameraA_point_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_point_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_point_width_max);
	GetPrivateProfileString("DetectParameter", "cameraA_point_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_point_width_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_area_min);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_area_max);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_length_min);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_length_max);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_width_min);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraA_thread_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraA_thread_width_max);
	GetPrivateProfileString("DetectParameter", "cameraA_thread_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraA_thread_width_max = atoi(IniRead);

	//Camera B detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_area_min);
	GetPrivateProfileString("DetectParameter", "cameraB_point_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_area_max);
	GetPrivateProfileString("DetectParameter", "cameraB_point_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_length_min);
	GetPrivateProfileString("DetectParameter", "cameraB_point_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_length_max);
	GetPrivateProfileString("DetectParameter", "cameraB_point_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_width_min);
	GetPrivateProfileString("DetectParameter", "cameraB_point_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_point_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_point_width_max);
	GetPrivateProfileString("DetectParameter", "cameraB_point_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_point_width_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_area_min);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_area_max);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_length_min);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_length_max);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_length_max = atoi(IniRead);


	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_width_min);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_width_min = atoi(IniRead);


	//IniWrite.Format("%d", parameters_setting_trans.cameraB_thread_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraB_thread_width_max);
	GetPrivateProfileString("DetectParameter", "cameraB_thread_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraB_thread_width_max = atoi(IniRead);



	//Camera C detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_area_min);
	GetPrivateProfileString("DetectParameter", "cameraC_point_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_area_min = atoi(IniRead);


	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_area_max);
	GetPrivateProfileString("DetectParameter", "cameraC_point_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_length_min);
	GetPrivateProfileString("DetectParameter", "cameraC_point_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_length_max);
	GetPrivateProfileString("DetectParameter", "cameraC_point_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_width_min);
	GetPrivateProfileString("DetectParameter", "cameraC_point_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_point_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_point_width_max);
	GetPrivateProfileString("DetectParameter", "cameraC_point_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_point_width_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_area_min);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_area_max);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_length_min);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_length_max);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_width_min);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_width_min = atoi(IniRead);
	
	//IniWrite.Format("%d", parameters_setting_trans.cameraC_thread_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraC_thread_width_max);
	GetPrivateProfileString("DetectParameter", "cameraC_thread_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraC_thread_width_max = atoi(IniRead);


	//Camera D detect parameter setting;
	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_area_min);
	GetPrivateProfileString("DetectParameter", "cameraD_point_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_area_max);
	GetPrivateProfileString("DetectParameter", "cameraD_point_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_length_min);
	GetPrivateProfileString("DetectParameter", "cameraD_point_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_length_max);
	GetPrivateProfileString("DetectParameter", "cameraD_point_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_width_min);
	GetPrivateProfileString("DetectParameter", "cameraD_point_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_point_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_point_width_max);
	GetPrivateProfileString("DetectParameter", "cameraD_point_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_point_width_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_area_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_area_min);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_area_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_area_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_area_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_area_max);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_area_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_area_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_length_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_length_min);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_length_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_length_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_length_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_length_max);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_length_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_length_max = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_width_min);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_width_min);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_width_min", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_width_min = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.cameraD_thread_width_max);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.cameraD_thread_width_max);
	GetPrivateProfileString("DetectParameter", "cameraD_thread_width_max", "", IniRead, 24, IniPath);
	parameters_setting_trans.cameraD_thread_width_max = atoi(IniRead);


	//DebugParaneters setting;
	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraA);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.debug_cameraA);
	GetPrivateProfileString("DebugParameter", "debug_cameraA", "", IniRead, 24, IniPath);
	parameters_setting_trans.debug_cameraA = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraB);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.debug_cameraB);
	GetPrivateProfileString("DebugParameter", "debug_cameraB", "", IniRead, 24, IniPath);
	parameters_setting_trans.debug_cameraB = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraC);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.debug_cameraC);
	GetPrivateProfileString("DebugParameter", "debug_cameraC", "", IniRead, 24, IniPath);
	parameters_setting_trans.debug_cameraC = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.debug_cameraD);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.debug_cameraD);
	GetPrivateProfileString("DebugParameter", "debug_cameraD", "", IniRead, 24, IniPath);
	parameters_setting_trans.debug_cameraD = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.stop_volve);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.stop_volve);
	GetPrivateProfileString("DebugParameter", "stop_volve", "", IniRead, 24, IniPath);
	parameters_setting_trans.stop_volve = atoi(IniRead);

	//IniWrite.Format("%d", parameters_setting_trans.save_blob);
	memset(IniRead, 0, 255);
	//sprintf(IniWrite, "%d,", parameters_setting_trans.save_blob);
	GetPrivateProfileString("DebugParameter", "save_blob", "", IniRead, 24, IniPath);
	parameters_setting_trans.save_blob = atoi(IniRead);
	//****************************End read the fiber detect parameters form ini files************************

	return;
}

int aq::Communicator::udp_send_server(std::string _ip, int _port)
{
	io_service ios;
	ip::udp::socket udp_server(ios);
	ip::udp::endpoint local_addr(ip::address::from_string(_ip), 9001);
	udp_server.open(local_addr.protocol());
	udp_server.bind(local_addr);
	char buf[BUF_SIZE] = "";
	ip::udp::endpoint send_point;
	int recv_len = 0;
	while (true) 
	{
		//recv_len = udp_server.receive_from(buffer(buf, BUF_SIZE), send_point);
		recv_len = udp_server.receive_from(asio::buffer((char*)data_buffer_.data(), sizeof(VCameraMessageTrans)), send_point);
		std::cout << "server recv size = " << recv_len << std::endl;
		//std::cout << "server recv message = " << buf << std::endl;
		//std::cout << "server send back size = " << udp_server.send_to(buffer(buf, recv_len), send_point) << std::endl;
	}
	return 0;
}

int aq::Communicator::udp_send_client(std::string _ip, int _port)
{
	Vsee::VCameraMessageTrans MagneticVolveMsg;
	io_service ios;
	ip::udp::socket sock(ios);
	ip::udp::endpoint end_point(ip::address::from_string(_ip), 9001);
	sock.open(end_point.protocol());
	char buf[BUF_SIZE] = "";
	string str;
	while (true) 
	{
		std::cout << "input str = ";
		std::cin >> str;
		try 
		{
			//std::cout << "client send size = " << sock.send_to(buffer(str.c_str(), str.size()), end_point) << std::endl;
			//std::cout << "client recv size = " << sock.receive_from(buffer(buf, BUF_SIZE), end_point) << std::endl;
			//std::cout << "client recv message = " << buf << std::endl;

			MutexLock lock(mutex_magnetic_valve);
			memcpy(aq::Communicator::valve_ctrl_msg, &MagneticVolveMsg, sizeof(Vsee::VCameraMessageTrans));
			if (!sock.is_open())
				return false;
			asio::error_code er;
			auto buf = asio::buffer(aq::Communicator::valve_ctrl_msg, sizeof(Vsee::VCameraMessageTrans));
			sock.send_to(buf, end_point);
			valve_ctrl_msg->signalling = 0;
		}
		catch (asio::system_error & e) 
		{
			std::cerr << e.what() << std::endl;
		}
	}


	return 0;
}

void    aq::Communicator::open_camera(aq::gigeVCamera& camera)
{
	if (!camera.b_camera_open)
		camera.handle = camera.create_camera_handle(camera._ip, 1);
	camera.create_captrue_thread(camera.handle);

	/*camera.set_camera_width(camera.handle, width);
	camera.set_camera_height(camera.handle, height);
	camera.set_camera_exposuretime(camera.handle, exposure_time);
	camera.set_camera_gain(camera.handle, gain);
	trigger = 0;
	camera.set_camer_trigger(camera.handle, trigger);*/
}

void    aq::Communicator::stop_camera(aq::gigeVCamera& camera)
{
	camera.context.exit=true;
}

void    aq::Communicator::start_grab(aq::gigeVCamera& camera)
{
	camera.start_transfer(camera.handle,-1);    //continues grab image
}

void    aq::Communicator::stop_grab(aq::gigeVCamera& camera)
{
	camera.stop_transfer(camera.handle);    //stop camera grab image
}