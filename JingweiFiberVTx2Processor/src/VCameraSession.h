#ifndef VSEE_CAMERA_SESSION_H_
#define VSEE_CAMERA_SESSION_H_

#include "VAsyncIO.h"
#include "VCameraMessage.h"

#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>


namespace Vsee
{
	//接收优先
	template <typename Derived>
	class VCameraSession
	{
	public:
		VCameraSession(tcp::socket& socket);
		~VCameraSession();

		bool isOpen() const;
		bool started() const;
		bool taskStarted() const;

	protected:
		void sendMessage(const VCameraMessage& msg);
		void sendMessage(VCameraMessage&& msg);

		void startSession();
		void stopSession();

		void startTask();
		void stopTask();

		void closeSession();

	private:
		/* virtual */ void processTask() {}
		/* virtual */void processMessage(VCameraMessage&& msg); /* = 0; */
		/* virtual */ void sessionAborted(); /* = 0; */

	private:
		void asyncTask();

		void asyncSend();
		void asyncRecv();

		void asyncError(asio::error_code ec);

		void doProcessTask();

	private:		
		using Buffer = std::vector<char>;
		using MessageQueue = std::queue<VCameraMessage>;
		using MutexLock = std::lock_guard<std::mutex>;

		tcp::socket& _socket;

		std::atomic_bool _started;
		std::atomic_bool _task;

		Buffer _recv_buf;
		std::size_t _recv_size;
		std::size_t _recv_offset;

		std::mutex _send_mutex;
		MessageQueue _send_queue;

	};


	template<typename Derived>
	inline VCameraSession<Derived>::VCameraSession(tcp::socket& socket) :
		_socket(socket),
		_started(false),
		_task(false),

		_recv_buf(8),
		_recv_size(8),
		_recv_offset(0),

		_send_mutex(),
		_send_queue()
	{
	}

	template<typename Derived>
	inline VCameraSession<Derived>::~VCameraSession()
	{
		if (isOpen())
			closeSession();
	}

	template<typename Derived>
	inline bool VCameraSession<Derived>::isOpen() const { return _socket.is_open(); }

	template<typename Derived>
	inline bool VCameraSession<Derived>::started() const { return _started; }

	template<typename Derived>
	inline bool VCameraSession<Derived>::taskStarted() const { return _task; }

	template<typename Derived>
	inline void VCameraSession<Derived>::sendMessage(const VCameraMessage & msg)
	{
		sendMessage(VCameraMessage(msg));
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::sendMessage(VCameraMessage && msg)
	{
		//{
		//	MutexLock lock(_send_mutex);
		//	_send_queue.push(std::move(msg));
		//}
		auto& io = _socket.get_io_service();
		io.post([this, m = std::move(msg)]() mutable
		{
			if (!_socket.is_open())
				return;

			auto buf = asio::buffer(m.bytes(), m.byteSize());
			asio::async_write(_socket, buf, [keep = std::move(m)](asio::error_code, std::size_t){});
		});
	}
	//function: to receive image date from camera simulator program
	template<typename Derived>
	inline void VCameraSession<Derived>::startSession()
	{
		if (_socket.is_open())
		{
			_started = true;

			asyncRecv();
			//asyncSend();
		}
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::stopSession()
	{
		if (_started)
		{
			asio::error_code ec;
			_socket.cancel(ec);
			_started = false;
		}
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::startTask()
	{
		if (_socket.is_open())
		{
			_task = true;
			asyncTask();
		}
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::stopTask()
	{
		if (_task)
			_task = false;
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::closeSession()
	{
		if (_socket.is_open())
		{
			stopSession();
			asio::error_code ec;
			_socket.close(ec);
		}
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::asyncSend()
	{
		if (!_socket.is_open() || !_started)
			return;

		VCameraMessage msg;
		{
			MutexLock lock(_send_mutex);
			if (_send_queue.size())
			{
				msg = std::move(_send_queue.front());
				_send_queue.pop();
			}
		}

		if (msg.empty())
			return;



		auto buf = asio::buffer(msg.bytes(), msg.byteSize());
		asio::async_write(_socket, buf, [this](asio::error_code ec, std::size_t)
		{
			if (ec)
				asyncError(ec);
			else
				asyncSend();
		});
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::asyncRecv()
	{
		if (!_socket.is_open() || !_started)
			return;

		asio::async_read(_socket, asio::buffer(_recv_buf.data() + _recv_offset, _recv_size), [this](asio::error_code ec, std::size_t)
		{
			if (ec)
			{
				asyncError(ec);
				return;
			}

			do
			{
				if (_recv_offset == 0 && _recv_size == 8)
				{
					if (!VCameraMessage::findLeadCode(_recv_buf.data(), _recv_buf.size()))
						break;

					auto next = VCameraMessage::nextLoad(_recv_buf.data(), _recv_buf.size());
					if (!next)
						break;

					if (_recv_buf.size() < next)
						_recv_buf.resize(next + 8);

					_recv_offset = 8;
					_recv_size = next;

				}
				else
				{
					auto msg = VCameraMessage::load(_recv_buf.data(), _recv_buf.size());
					processMessage(std::move(msg));
					_recv_offset = 0;
					_recv_size = 8;
				}

			} while (0);

			asyncRecv();
		});
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::asyncError(asio::error_code ec)
	{
		if (!_socket.is_open() || !_started)
			return;

		namespace error = asio::error;

		switch (ec.value())
		{
		case error::eof:
		case error::broken_pipe:
		case error::connection_aborted:
		case error::connection_refused:
		case error::connection_reset:
			closeSession();
			sessionAborted();
		case error::access_denied:
		case error::bad_descriptor:
		case error::shut_down:
			return;

		case error::no_buffer_space:
			//notifiy or not
			return;
		}
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::asyncTask()
	{
		static auto task_buf = asio::buffer((char*)nullptr, 0);

		if (!_socket.is_open() || !_task)
			return;

		_socket.async_write_some(task_buf, [this](asio::error_code ec, std::size_t)
		{
			if (ec)
			{
				asyncError(ec);
				return;
			}

			doProcessTask();
			asyncTask();

		});
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::doProcessTask()
	{
		static_cast<Derived*>(this)->processTask();
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::processMessage(VCameraMessage&& msg)
	{
		static_cast<Derived*>(this)->processMessage(std::move(msg));
	}

	template<typename Derived>
	inline void VCameraSession<Derived>::sessionAborted()
	{
		static_cast<Derived*>(this)->sessionAborted();
	}
}

#endif
