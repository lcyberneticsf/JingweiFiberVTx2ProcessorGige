#ifndef VTX2SESSION_H
#define VTX2SESSION_H

#include "VCameraEnum.h"
#include "VCameraFrame.h"
#include "VCameraMessage.h"
#include "VCameraProcessorTypes.h"
#include "VCameraSession.h"
#include "VCameraEnumPrivate.h"
#include <array>
#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <thread>
#include <iostream>
#include <functional>
using namespace std;

namespace Vsee
{
	class VTx2Session : public VCameraSession<VTx2Session>
	{
	public:
		using Base = VCameraSession<VTx2Session>;
		using ProcessFrameFunc = std::function<void(VCameraFrame&&)>;
		using SessionAbortedFunc = std::function<void()>;


		VTx2Session(tcp::socket& socket) :
			Base(socket),
			_process_frame_func(),
			_session_aborted_func()
		{}

		void startSession()
		{
			static std::array<char, 1> mode = { TransMode::Up };

			Base::startSession();
			sendMessage(VCameraMessage(Token::SetTransMode, mode.data(), mode.size()));
		}

		void setFrameResult(const VFrameResult& res)
		{
			sendMessage(VCameraMessage(Token::PushProcessResult, res.bytes(), res.byteSize()));
		}

		void setProcessFrameFunc(ProcessFrameFunc&& func) { _process_frame_func = std::move(func); }
		void setProcessFrameFunc(const ProcessFrameFunc& func) { _process_frame_func = func; }

		void setSessionAbortedFunc(SessionAbortedFunc&& func) { _session_aborted_func = std::move(func); }
		void setSessionAbortedFunc(const SessionAbortedFunc& func) { _session_aborted_func = func; }

	private:
		friend class VCameraSession<VTx2Session>;

		void processMessage(VCameraMessage&& msg) /* override */
		{
			if (msg.empty())
				return;

			if (msg.token() != Token::RetFrame)
				return;

			if (!_process_frame_func)
				return;

			_process_frame_func(VCameraFrame(std::move(msg)));
		}

		void sessionAborted() /* override */
		{
			if (_session_aborted_func)
				_session_aborted_func();
		}

	private:
		ProcessFrameFunc _process_frame_func;
		SessionAbortedFunc _session_aborted_func;

	};
	struct InferFrame
	{
	public:

		InferFrame() :
			_session(nullptr), _frame() {}

		InferFrame(VTx2Session* s, VCameraFrame&& f) :
			_session(s), _frame(std::move(f)) {}

		InferFrame(InferFrame&& other) :
			_session(other._session), _frame(std::move(other._frame)),
			label(other.label), width(other.width), height(other.height),
			channels(other.channels),camera_serial(other.camera_serial),
			data(other.data)
		{
			std::cout << "InterFame:99" << std::endl;
			other.data = nullptr;
			other._session = nullptr;
		}
		InferFrame(const Vsee::InferFrame& f) :
			//_session(f._session), _frame(std::move(f._frame)),
			label(f.label), width(f.width), height(f.height),
			channels(f.channels),camera_serial(f.camera_serial),
			data(std::move(f.data))
		{
			//f.data = nullptr;
			label = f.label;
			width = f.width;
			std::cout << "Infer:104,width=" <<width<< std::endl;
			std::cout << "Infer:104,height=" << height << std::endl;
			std::cout<<"Infer:104,InferFrame(const Vsee::InferFrame& f)"<<std::endl;
		}

		InferFrame& operator=(InferFrame&& rhs)
		{
			if (&rhs != this)
			{
				_session = rhs._session;
				rhs._session = nullptr;
				_frame = std::move(rhs._frame);
				label = rhs.label;
				width = rhs.width;
				height = rhs.height;
				channels = rhs.channels;
				camera_serial = rhs.camera_serial;
				data = std::move(rhs.data);
			}

			return *this;
		}

		VTx2Session* _session;
		VCameraFrame _frame;
		std::uint32_t label = 0;
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t channels;
		std::uint32_t camera_serial;
		char* data=nullptr;
	};
}
#endif