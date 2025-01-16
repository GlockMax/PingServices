#include <string>
#include <iostream>

#include <zmq.hpp>

int main(){
    zmq::context_t context{1};

    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.connect("tcp://localhost:5555");

    const std::string data = "Hello!";

    for (int req_num = 0; ; req_num++){
        std::cout << "Шлём привет под номером " << req_num << "..." << std::endl;
        socket.send(zmq::buffer(data), zmq::send_flags::none);

        // Ждём привет
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

	std::cout << "Получили: " << reply.to_string();
	std::cout << " (" << req_num << ")";
	std::cout << std::endl;
    }
    return 0;
}
