#include <string>	 
#include <iostream>	 // cout
#include <fstream>	 // ifstream
#include <chrono>	 // milliseconds
#include <vector>
#include <numeric>	 // std::iota

#include <zmq_addon.hpp>
#include <json/json.h>


// TODO: некоторый объект конфига, который будет удобно читать
int main(){
    zmq::context_t context{1};

    Json::Value addresses_obj;
    std::ifstream addr_file("addresses.json", std::ifstream::binary);
    addr_file >> addresses_obj;

    std::vector<std::string> addrs;
    for (Json::Value::ArrayIndex i = 0; i != addresses_obj["addresses"].size(); i++) {
        addrs.push_back(addresses_obj["addresses"][i].asString());
    }
    
    int addr_count = addrs.size();

    std::vector<zmq::socket_t> sockets;
    zmq::poller_t<int> poller;

    for (int i = 0; i < addr_count; i++){
	sockets.emplace_back(context, zmq::socket_type::req);    
	std::cout << "nullpointer ли?: " << ((sockets[i]) == nullptr) << std::endl;
        sockets[i].set(zmq::sockopt::linger, 0);
	std::cout << ("tcp://" + addrs[i]) << std::endl;
        sockets[i].connect("tcp://" + addrs[i]);
        poller.add(sockets[i], zmq::event_flags::pollin);
    }
    std::cout << "nullpointer ли вне цикла?: " << ((sockets[0]) == nullptr) << std::endl;

    const std::chrono::milliseconds timeout {3000};
    
    std::vector<zmq::poller_event<int>> in_events{addr_count};
    
    std::vector<int> dead_addrs(addr_count);
    std::iota(dead_addrs.begin(), dead_addrs.end(), 0);

    const std::string data = "get_status";

    while (true){
	for (auto& socket : sockets){
            (socket).send(zmq::buffer(data), zmq::send_flags::none);
	}
        const int nin = poller.wait_all(in_events, timeout);

        for (int i = 0; i < nin; i++){
	    // Находим индекс сокета, который нам ответил
	    auto s_it = std::find(sockets.begin(), sockets.end(), in_events[i].socket);
	    int s_n = std::distance(sockets.begin(), s_it);

            // И удаляем его из неответивших адресов
            dead_addrs.erase(std::find(dead_addrs.begin(), dead_addrs.end(), s_n));

	    std::vector<zmq::message_t> reply_multi;
	    auto ret = zmq::recv_multipart(in_events[i].socket, 
			        std::back_inserter(reply_multi),
			        zmq::recv_flags::none);
	    if (!ret){
	        std::cout << "Ошибка в получении сообщения" << std::endl;
	        return 1;
	    }
	    else {
	    
	        for (int i = 0; i < reply_multi.size(); i++){
	            std::cout << addrs[s_n] << ":" << reply_multi[i].to_string() << " ";
	        }
	        std::cout << std::endl;
	    }
	}

	for (int i : dead_addrs) {
            std::cout << i << "Тайм-аут для сокета " << addrs[i] << std::endl;
	    
	    poller.remove(sockets[i]);
	    sockets[i].close();
	    sockets.emplace(sockets.begin()+i, context, zmq::socket_type::req);
            sockets[i].set(zmq::sockopt::linger, 0);
	    sockets[i].connect("tcp://" + addrs[i]);
	    poller.add(sockets[i], zmq::event_flags::pollin);
	}
        dead_addrs = std::vector<int>(addr_count);
	std::iota(dead_addrs.begin(), dead_addrs.end(), 0);

    }
    return 0;
}
