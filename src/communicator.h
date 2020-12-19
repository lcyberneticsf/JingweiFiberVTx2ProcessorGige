#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H
#include "VAsyncIO.h"
#include <queue>
#include <mutex>
#include <vector>
#include "VTx2Session.h"
#include "VCameraMessageTrans.h"
#include "gigevcamera.h"
#define  DATA_BUFFER_LENGTH 5000000
//#define Xavive_NX_MODE
//#define TX2_MODE
using namespace asio::ip;
using namespace Vsee;
//using namespace aq;
typedef unsigned char uchar;

using MutexLock = std::lock_guard<std::mutex>;

namespace asio {
	class io_service;
	
	namespace ip {
		class tcp;
		template <typename InternetProtocol>
		class basic_endpoint;
	}
	template <typename Protocol>
	class stream_socket_service;
//	template <typename Protocol,typename StreamSocketService = stream_socket_service<Protocol> >
//	class basic_stream_socket;
}

namespace Vsee{
	class VCameraFrame;
	class InferFrame;
	class VTx2Session;
}

namespace aq {
//namespace Vsee {
class Communicator;

enum CMD
{
    SEND_RESULTS   = 71,
    START_RECV_IMG = 1,
    STOP_RECV_IMG  = 2
};

enum RESULT_STATE
{
    NO         = 0,
    OK         = 1,
    V_WRINKLED = 2,
    WRINKLED   = 3,
    V_DEFORMED = 4,
    DEFORMED   = 5
};


//***********Protocol,use to driver Leihang Valve electric board,Begin*******************************
/*
Name : typedef struct _time_calib_req_tag_:
Function: Time correction ask frame;
时间校准帧，任意一个Agent都可以向I/O-Server在任意时刻发送时间校准包,该包用于同步Agent和I/O-Server的时间上下文。
Agent向I/O-Server发送如下的包（payload_len=8），其中 type 必须填充 0x80000010 （请求）;src 必须填充 0xCC880000（请求类型：为时间校准）;
*/
typedef struct _time_calib_req_tag_
{
	uint32_t type;
	uint32_t src;
}time_calib_req_t;


/*
Name : typedef struct _ack_tag_:
Function: Time correcting frame response 
I/O-Server收到上述包后，会向Agent返回如下的包（payload_len=16）, 其中 type 是 0x80000000（响应）; 
其中 ack_src 是 0xCC8800DE（响应源：为时间校准请求），（即之前发送给I/O-Server的时间校准请求类型）, 
而ack_val是一个 64-bit的绝对非负整数，代表I/O-Server内部的绝对时间滴答值，每个滴答（也就是每个当量‘1’）代表 0.1us（对应内部10mhz的管理时钟源）
注意：ack_val 是 int64 然而，此处的上下文决定其值必须是非负的, 如果当 ack_src 表示 “为时间校准请求” 的响应源; 
而 ack_val 为负数时；那么这是一个未定义的包; Agent应当忽略此包.
*/
typedef struct _ack_tag_
{
	uint32_t type;
	uint32_t ack_src;
	int64_t ack_val;
} ack_t;


/*
Name : typedef struct _io_output_transac_tag_
function: I/O-Server 的驱动帧
Agent 向 I/O-Server发送如下的包（payload_len=20）, 其中 type 必须填充 0x80000010 （请求）;src 必须填充 0xCC880004（请求类型：为I/O驱动请求）;
port对应硬件输出端口号码，合法值为：1 ~ 28；如果值不合法那么该包将丢弃, 同时I/O驱动业务将不被任何一个硬件端口执行, ref - 参考时间，即在第 1 小节中，
通过时间校准获得的 ‘ack_val’, 代表I/O-Server内部的绝对时间滴答值，每个滴答（也就是每个当量‘1’）代表0.1us（对应内部10mhz的管理时钟源）;
delay - 代表延迟值，该值 × 0.1us，即为该I/O 驱动相对于 ref 的延迟值, dura - I/O驱动时间，该值 × 0.1us，即为 I/O 处于驱动状态（ON State）的时间
*/
typedef struct _io_output_transac_tag_
{
	uint32_t type;         //0x80000010 （请求）
	uint32_t src;          //0xCC880004（请求类型：为I / O驱动请求）
	uint32_t port;         //port对应硬件输出端口号码，合法值为：1 ~ 28
	uint32_t ref;          //ref - 参考时间
	uint32_t delay;        //delay - 代表延迟值，该值 × 0.1us，即为该I/O 驱动相对于 ref 的延迟值
	uint32_t dura;         //dura - I/O驱动时间，该值 × 0.1us，即为 I/O 处于驱动状态（ON State）的时间
}io_output_transac_t;


/*
name: typedef struct _ack_tag_
function:
当上述I/O驱动业务完成后，I/O-Server会：
		1、将I/O立即重新置为 OFF State；
		2、向Agent返回如下的_ack_tag_响应包;
其中 type 是 0x80000000（响应）; ack_src 是 0xCC880004（响应源：为 I/O驱动 请求），（即之前发送给I/O-Server的时间校准请求类型）;
而ack_val是一个 64-bit的绝对非负整数，代表I/O-Server内部的绝对时间滴答值,每个滴答（也就是每个当量‘1’）代表 0.1us（对应内部10mhz的管理时钟源）,
该值为I/O Server成功执行完成瞬间时，的对应的绝对滴答坐标。如果I/O 驱动请求由于某种原因执行失败，那么ack_val将为负数，并且表示相应的错误原因：
		1、 0xFF00000000000001 - port 非法
		2、 0xFF00000000000002 - ref + delay 已经小于当前I/O-Server内部的绝对滴答坐标
*/
//typedef struct _ack_tag_
//{
//	uint32_t type;
//	uint32_t ack_src;
//	int64_t ack_val;
//}ack_t;
//***********Protocol,use to driver Leihang Valve electric board,end *******************************

class Communicator
{
private:
    static std::vector<std::string> split(std::string str, char seg);
    static void print_hex(unsigned char *_buf,int _len);
public:
    Communicator();

    bool	tcp_connect(std::string _ip, int _port, std::string& _error_str);
	bool	magnetic_valve_connect(std::string _ip, int _port, std::string& _error_str);
	bool    show_image_connect(std::string _ip, int _port, std::string& _error_str);

    bool	receive_img(std::string& _error_str);
	bool	receive_img(std::string &_error_str, asio::basic_stream_socket<asio::ip::tcp>* socket_);
    bool	send_cmd(CMD _cmd, std::string& _error_str, std::vector<int> _state = std::vector<int>(), int _sequence = -1);

	bool	server(std::string _ip, int _port, std::string &_error_str);
	bool	server_run(asio::basic_stream_socket<asio::ip::tcp>* socket_, std::string _ip, int _port, std::string &_error_str);
	bool	get_file_list(std::string file_load_path);
	int		get_files(std::string fileFolderPath, std::string fileExtension, int& nFileNum);
	bool	run_tcp_connect(std::string _ip, int _port);
	bool	run_magnetic_valve_connect(std::string _ip, int _port);
	bool	run_show_image_connect(std::string _ip, int _port);
	bool	tcp_connect_thread(std::string _ip, int _port);
	bool	magnetic_valve_connect_thread(std::string _ip, int _port);
	bool    image_show_connect_thread(std::string _ip, int _port);

	int		find_dir_file(const char* dir_name, std::vector<std::string>& v);
	int		show_ctrl_message(asio::basic_stream_socket<asio::ip::tcp>* socket_,int  wParam, void* lParam);
	int		show_ctrl_message(asio::basic_stream_socket<asio::ip::tcp>* socket, int  wParam, void* lParam, int length, uchar* m_buf);
	void	WriteIni(CParameterSetting parameters_setting_trans);
	void	ReadIni();
	void	set_camera_params(aq::gigeVCamera &camera, CParameterSetting parameters_setting_trans);
	void    open_camera(aq::gigeVCamera& camera);
	void    stop_camera(aq::gigeVCamera& camera);
	void    start_grab(aq::gigeVCamera& camera);
	void    stop_grab(aq::gigeVCamera& camera);
	int		readMaxBytesInTime(asio::ip::tcp::socket& socket, char* strBuf, int nMaxBytes, int nMilSec);
	int     streamReadMaxBytesInTime(asio::basic_stream_socket<asio::ip::tcp>& socket_, char* strBuf, int nMaxBytes, int nMilSec);

	int     udp_send_server(std::string _ip, int _port);
	int		udp_send_client(std::string _ip, int _port);


//private:
public:
	std::queue<Vsee::VCameraFrame>* frame_queue_;
	std::mutex m_;
	static std::mutex mutex_magnetic_valve;
	asio::io_service* io_service_;
	//asio::ip::basic_endpoint<asio::ip::tcp>* ep_;
	tcp::endpoint  * ep_;
	asio::basic_stream_socket<asio::ip::tcp>* socket_;
	static asio::basic_stream_socket<asio::ip::tcp>* magnetic_valve_socket_;
	static asio::ip::udp::socket* magnetic_valve_udp_socket_;
	static asio::basic_stream_socket<asio::ip::tcp>* show_image_socket_;
	std::string data_buffer_;
	asio::ip::tcp::socket* _socket;
	static asio::ip::udp::endpoint* end_point;
	//asio::tcp::acceptor acceptor_;

	//Vsee::VTx2Session* sessions[8];
	tcp::socket* sockets[8];
	asio::io_service* ios[8];
	//std::queue<Vsee::InferFrame> infer_queue;
	std::mutex             infer_mutex;
	tcp::endpoint eps[8];
	std::vector<std::string> file_list;
	int file_amount;
	static bool		batch_image_load;
	static bool		single_image_load;
	static bool     m_bServerSendMode;
	static bool     m_bClientSendMode;
	static bool     m_bMageneticValveClientSendMode;
	static bool		m_bServerRun;
	static bool		m_bClientRun;
	static bool		m_bMagneticValveClientRun;
	static bool     m_bRunFiberDetect;
	static bool     m_bImageClientRun;
	static bool     m_bGrabTrainImage;
	static bool     m_bTcpMode;
	static bool     m_bShowImageConnetc;

	int		counter;
	int     box_index;
	int		camera_scan_speed;
	static  Vsee::VCameraMessageTrans  box_ctrl_msg;
	static  Vsee::VCameraMessageTrans*  valve_ctrl_msg;
	static  Vsee::VCameraMessageTrans_new  box_ctrl_msg_2;
	static  CParameterSetting			parameters_setting_trans;
	static  Vsee::VCameraMessageTrans* show_img_msg;
	static bool m_bWriteLog;
};

}

#endif // COMMUNICATOR_H
