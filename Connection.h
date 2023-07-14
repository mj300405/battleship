#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <deque>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = boost::beast::websocket;

class Connection {
  public:
	Connection(const std::string& host, const std::string& port)
		: host_(host), port_(port), io_context_(), resolver_(io_context_),
		  ws_(io_context_) {}

	~Connection() {
		if (ws_.is_open()) {
			ws_.close(websocket::close_code::normal);
		}
		if (runner.joinable()) runner.join();
	}

	void Connect() {
		asio::ip::tcp::resolver::query query(host_, port_);
		resolver_.async_resolve(query,
								std::bind(&Connection::OnResolve,
										  this,
										  std::placeholders::_1,
										  std::placeholders::_2));
		runner = std::thread([this](){
			io_context_.run();
		});
	}

	void Send(const std::string& message) {
		asio::post(io_context_, [this, message]() {
			std::lock_guard<std::mutex> lock(mutex_);
			sendQueue_.push_back(message);
			if (!sending_) {
				StartSending();
			}
		});
	}

	void Receive(std::function<void(const std::string&)> callback) {
		std::unique_lock<std::mutex> lock(mutex_);
		receiveCallback_ = std::move(callback);
		condVar_.wait(lock, [this]() { return !receiveQueue_.empty(); });
		std::string message = receiveQueue_.front();
		receiveQueue_.pop_front();
		lock.unlock();
		receiveCallback_(message);
	}

  private:
	void OnResolve(beast::error_code ec,
				   asio::ip::tcp::resolver::results_type results) {
		if (ec) {
			std::cerr << "Failed to resolve host: " << ec.message()
					  << std::endl;
			return;
		}

		asio::async_connect(ws_.next_layer(),
							results.begin(),
							results.end(),
							std::bind(&Connection::OnConnect,
									  this,
									  std::placeholders::_1));
	}

	void OnConnect(beast::error_code ec) {
		if (ec) {
			std::cerr << "Failed to connect: " << ec.message() << std::endl;
			return;
		}

		ws_.async_handshake(host_,
							"/",
							std::bind(&Connection::OnHandshake,
									  this,
									  std::placeholders::_1));
	}

	void OnHandshake(beast::error_code ec) {
		if (ec) {
			std::cerr << "Handshake failed: " << ec.message() << std::endl;
			return;
		}

		StartReceiving();
	}

	void StartReceiving() {
		ws_.async_read(receiveBuffer_,
					   std::bind(&Connection::OnReceive,
								 this,
								 std::placeholders::_1,
								 std::placeholders::_2));
	}

	void OnReceive(beast::error_code ec, std::size_t bytes_transferred) {
		if (ec) {
			std::cerr << "Receive failed: " << ec.message() << std::endl;
			return;
		}

		std::lock_guard<std::mutex> lock(mutex_);
		receiveQueue_.emplace_back(
			beast::buffers_to_string(receiveBuffer_.data()));
		receiveBuffer_.consume(bytes_transferred);
		condVar_.notify_one();
		StartReceiving();
	}

	void StartSending() {
		if (sendQueue_.empty()) {
			sending_ = false;
			return;
		}

		sending_ = true;
		ws_.async_write(asio::buffer(sendQueue_.front()),
						std::bind(&Connection::OnSend,
								  this,
								  std::placeholders::_1,
								  std::placeholders::_2));
	}

	void OnSend(beast::error_code ec, std::size_t bytes_transferred) {
		if (ec) {
			std::cerr << "Send failed: " << ec.message() << std::endl;
			return;
		}

		std::lock_guard<std::mutex> lock(mutex_);
		sendQueue_.pop_front();
		StartSending();
	}

  private:
	std::string host_;
	std::string port_;
	asio::io_context io_context_;
	asio::ip::tcp::resolver resolver_;
	websocket::stream<asio::ip::tcp::socket> ws_;
	beast::flat_buffer receiveBuffer_;
	std::mutex mutex_;
	std::condition_variable condVar_;
	std::deque<std::string> sendQueue_;
	std::deque<std::string> receiveQueue_;
	bool sending_ = false;
	std::function<void(const std::string&)> receiveCallback_;
	std::thread runner;

  public:
	std::mutex send_mutex;
	std::condition_variable send_data_var;
	std::string message;

    void send_data(nlohmann::json j){
        std::string mes = j.dump();
        message = mes;
        send_data_var.notify_one();
    }

    void join_game(const std::string& game_id) {
       nlohmann::json message;
       message["type"] = "join_game";
       message["game_id"] = game_id;
       Send(message.dump());
   }

   void create_game(const std::string& game_id) {
       nlohmann::json message;
       message["type"] = "create_game";
       message["game_id"] = game_id;
       Send(message.dump());
   }
};



/*
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <queue>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

class Connection {
private:
    websocket::stream<tcp::socket>* ws_;
    boost::beast::multi_buffer buffer_;
    std::queue<std::string> send_queue_;

    tcp::resolver* resolver;
    boost::asio::io_context* ioc;
public:
    Connection() {
        ioc = new boost::asio::io_context;
        resolver = new tcp::resolver(*ioc);
        ws_ = new websocket::stream<tcp::socket>(*ioc);
    }
    ~Connection() {
        delete ws_;
        delete resolver;
        delete ioc;
    }

    void connect(const std::string& host, const std::string& port) {
        auto const results = resolver->resolve(host, port);

        boost::asio::connect(ws_->next_layer(), results.begin(), results.end());
        ws_->set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(boost::beast::http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                " websocket-client-coro");
            }));
        ws_->handshake(host, "/");
    }

    void send_message(nlohmann::json j) {
        send_queue_.push(j.dump());
        if (send_queue_.size() > 1) {
            // Additional messages will be sent once the first message's
            // async_write operation completes.
            return;
        }

        do_send();
    }

    void close() {
        ws_->async_close(websocket::close_code::normal,
            [](boost::system::error_code ec) {
                if (ec)
                    std::cerr << "close: " << ec.message() << "\n";
            });
    }

    void do_send() {
        ws_->async_write(
            boost::asio::buffer(send_queue_.front()),
            [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    std::cerr << "write: " << ec.message() << "\n";
                    return;
                }

                send_queue_.pop();
                if (!send_queue_.empty())
                    do_send();
            });
    }

    void do_read() {
        ws_->async_read(
            buffer_,
            [this](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    std::cerr << "read: " << ec.message() << "\n";
                    return;
                }

                std::string received_data =
                    boost::beast::buffers_to_string(buffer_.data());
                buffer_.consume(buffer_.size());

                // Process received message with the passed in callback
                message_handler_(nlohmann::json::parse(received_data));

                // Continue reading.
                do_read();
            });
    }

    void run() {
        // Start the reader loop.
        do_read();

        // Run the I/O service. The application will exit this function when
        // the WebSocket connection is closed.
        ioc->run();
    }

    void join_game(const std::string& game_id) {
       nlohmann::json message;
       message["type"] = "join_game";
       message["game_id"] = game_id;
       send_message(message);
   }

   void create_game(const std::string& game_id) {
       nlohmann::json message;
       message["type"] = "create_game";
       message["game_id"] = game_id;
       send_message(message);
   }
};*/




//#include <boost/beast/core.hpp>
//#include <boost/beast/websocket.hpp>
//#include <boost/asio/connect.hpp>
//#include <boost/asio/ip/tcp.hpp>
//#include <cstdlib>
//#include <functional>
//#include <iostream>
//#include <memory>
//#include <string>
//
//using tcp = boost::asio::ip::tcp;
//
//namespace websocket = boost::beast::websocket;
//
//class Connection {
//private:
//    websocket::stream<tcp::socket>* ws_;
//    boost::beast::multi_buffer buffer_;
//    std::mutex buffer_mutex;
//    std::mutex ws_mutex;
//    std::mutex async_write_mutex;
//
//    tcp::resolver* resolver;
//    boost::asio::io_context* ioc;
//public:
//    Connection(){
//        ioc = new boost::asio::io_context;
//        resolver = new tcp::resolver(*ioc);
//        ws_ = new websocket::stream<tcp::socket>(*ioc);
//    }
//    ~Connection(){
//        delete ws_;
//        delete resolver;
//        delete ioc;
//    }
//
//    void connect(const std::string& host, const std::string& port) {
//        auto const results = resolver->resolve(host, port);
//
//        boost::asio::connect(ws_->next_layer(), results.begin(), results.end());
//        ws_->set_option(websocket::stream_base::decorator(
//            [](websocket::request_type& req)
//            {
//                req.set(boost::beast::http::field::user_agent,
//                    std::string(BOOST_BEAST_VERSION_STRING) +
//                        " websocket-client-coro");
//            }));
//        ws_->handshake(host, "/");
//    }
//
//
//    void send_message(nlohmann::json j) {
//        std::string message = j.dump();
//        std::unique_lock ulock(async_write_mutex);
//        ws_->write(boost::asio::buffer(std::string(message)));
//        ulock.unlock();
//    }
//
//    void close() {
//        std::unique_lock ulock(async_write_mutex);
//        ws_->close(websocket::close_code::normal);
//        ulock.unlock();
//    }
//
//    void receive_message(std::function<void(nlohmann::json)> callback) {
//        // std::cout << "RISIW MESSAGE CZY NIE ?"<<std::endl;
//        std::unique_lock ulock(async_write_mutex);
//        ws_->async_read(buffer_, [this, callback,&ulock](boost::system::error_code ec, std::size_t bytes_transferred) {
//            // std::cout << "To jest w async_read" << std::endl;
//            if (ec) {
//                std::cout << "Read error: " << ec.message() << "\n";
//                return;
//            }
//            // std::cout << "To jest po async_read" << std::endl;
//
//            auto received_data = boost::beast::buffers_to_string(buffer_.data());
//            buffer_.consume(buffer_.size());
//            // std::cout << "A tutaj wywoluje callback" << std::endl;
//            callback(nlohmann::json::parse(received_data));
//            ulock.unlock();
//            });
//        ioc->run();
//    }
//
//
//
//    void join_game(const std::string& game_id) {
//        nlohmann::json message;
//        message["type"] = "join_game";
//        message["game_id"] = game_id;
//        send_message(message);
//    }
//
//    void create_game(const std::string& game_id) {
//        nlohmann::json message;
//        message["type"] = "create_game";
//        message["game_id"] = game_id;
//        send_message(message);
//    }
//};





//#pragma once
//
//#include <functional>
//#include <mutex>
//#include <websocketpp/config/asio_no_tls_client.hpp>
//#include <websocketpp/client.hpp>
//#include <nlohmann/json.hpp>
//
//typedef websocketpp::client<websocketpp::config::asio_client> client;
//
//class Connection {
//private:
//    client c;
//    client::connection_ptr con;
//    client::message_ptr msg;
//    std::string uri;
//    std::function<void(const nlohmann::json&)> messageHandler;
//
//public:
//
//    std::queue<nlohmann::json> messageQueue;
//    std::mutex queueMutex;
//
//    Connection(std::string uri, std::function<void(const nlohmann::json&)> handler)
//        : uri(uri), messageHandler(handler) {
//        c.init_asio();
//
//        c.set_open_handler(std::bind(&Connection::on_open, this, std::placeholders::_1));
//        c.set_fail_handler(std::bind(&Connection::on_fail, this, std::placeholders::_1));
//        c.set_close_handler(std::bind(&Connection::on_close, this, std::placeholders::_1));
//        c.set_message_handler(std::bind(&Connection::on_message, this, std::placeholders::_1, std::placeholders::_2));
//        c.set_open_handshake_timeout(100000); // in milliseconds
//
//    }
//
//    void start() {
//        websocketpp::lib::error_code ec;
//        con = c.get_connection(uri, ec);
//        c.connect(con);
//
//        // Run the connection on a separate thread
//        std::thread([this]() {
//            c.run();
//            }).detach();
//    }
//
//
//    void on_open(websocketpp::connection_hdl hdl) {
//        // Connection opened
//        std::cout << "Opened connection" << std::endl;
//    }
//
//    void on_fail(websocketpp::connection_hdl hdl) {
//        // Check the fail code and reason
//        auto fail_code = con->get_ec();
//        std::cout << "Fail connection with code: " << fail_code << " and reason: " << fail_code.message() << std::endl;
//    }
//
//    void on_close(websocketpp::connection_hdl hdl) {
//        // Check the close code and reason
//        auto close_code = con->get_remote_close_code();
//        auto close_reason = con->get_remote_close_reason();
//        std::cout << "Closed connection with code: " << close_code << " and reason: " << close_reason << std::endl;
//    }
//
//    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
//        // Parse the received message as JSON and pass it to the handler
//        nlohmann::json receivedData = nlohmann::json::parse(msg->get_payload());
//        messageHandler(receivedData);
//        {
//            std::lock_guard<std::mutex> lock(queueMutex);
//            messageQueue.push(receivedData);
//        }
//    }
//
//    void send_message(const nlohmann::json& message) {
//        websocketpp::lib::error_code ec;
//        // Convert JSON to a string before sending
//        std::string messageStr = message.dump();
//        c.send(con, messageStr, websocketpp::frame::opcode::text, ec);
//        if (ec) {
//			std::cout << "Send failed: " << ec.message() << std::endl;
//		}
//    }
//
//    bool connect() {
//        websocketpp::lib::error_code ec;
//        con = c.get_connection(uri, ec);
//        if (ec) {
//            std::cout << "Connect initialization error: " << ec.message() << std::endl;
//            return false;
//        }
//        c.connect(con);
//        return true;
//    }
//
//    void set_message_handler(std::function<void(const nlohmann::json&)> handler) {
//        messageHandler = handler;
//    }
//
//    bool is_connected() const {
//        return con->get_state() == websocketpp::session::state::open;
//    }
//
//    void create_game(const std::string& game_id) {
//        // Prepare the request message
//        nlohmann::json requestMsg;
//        requestMsg["type"] = "create_game";
//        requestMsg["game_id"] = game_id;
//
//        // Send the request message to the server
//        send_message(requestMsg);
//    }
//
//    void join_game(const std::string& game_id) {
//		// Prepare the request message
//		nlohmann::json requestMsg;
//		requestMsg["type"] = "join_game";
//		requestMsg["game_id"] = game_id;
//
//		// Send the request message to the server
//		send_message(requestMsg);
//	}
//
//    nlohmann::json receive_message() {
//        
//            std::lock_guard<std::mutex> lock(queueMutex);
//            if (!messageQueue.empty()) {
//                nlohmann::json message = messageQueue.front();
//                messageQueue.pop();
//                return message;
//            }
//        
//    }
//
//
//};
