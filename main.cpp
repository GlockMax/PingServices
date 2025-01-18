#include <string>
#include <iostream>
#include <chrono>

#include <zmq_addon.hpp>


// TODO: некоторый объект конфига, который будет удобно читать
int main(){
    zmq::context_t context{1};

    zmq::socket_t* socket = new zmq::socket_t(context, zmq::socket_type::req);
    
    int linger = 0;

    socket->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    socket->connect("tcp://localhost:5555");

    zmq::poller_t<int> poller;
    poller.add(*socket, zmq::event_flags::pollin);

    const std::chrono::milliseconds timeout {3000};
    
    std::vector<zmq::poller_event<int>> in_events{1};

    const std::string data = "get_status";

    for (int req_num = 0; ; req_num++){
        std::cout << "Шлём привет под номером " << req_num << "..." << std::endl;
        socket->send(zmq::buffer(data), zmq::send_flags::none);
        
        const auto nin = poller.wait_all(in_events, timeout);
	if (!nin) {
	    std::cout << "Тайм-аут!" << std::endl;
	    poller.remove(*socket);
	    socket->close();
	    delete socket;
	    socket = new zmq::socket_t(context, zmq::socket_type::req);
            socket->setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
	    socket->connect("tcp://localhost:5555");
	    poller.add(*socket, zmq::event_flags::pollin);
	    continue;
	}

        // Ждём привет
	std::vector<zmq::message_t> reply_multi;
	auto ret = zmq::recv_multipart(*socket, std::back_inserter(reply_multi),
			zmq::recv_flags::none);
	if (!ret){
	    std::cout << "Ошибка в получении сообщения" << std::endl;
	    return 1;
	}
	else {
	    
	    for (int i = 0; i < reply_multi.size(); i++){
	        std::cout << reply_multi[i].to_string() << " ";
	    }
	    std::cout << std::endl;
	}

    }
    return 0;
}
