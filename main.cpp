#include <string>	 
#include <iostream>	 // cout
#include <fstream>	 // ifstream
#include <chrono>	 // milliseconds
#include <vector>
#include <numeric>	 // std::iota

#include <zmq_addon.hpp>
#include <json/json.h>


// TODO: некоторый объект конфига, который будет удобно читать В ОТДЕЛЬНОМ ФАЙЛЕ
int main(int argc, char* argv[]){
    zmq::context_t context{1};
    
    // Чтение конфиг-файла
    // 
    Json::Value addresses_obj;
    std::ifstream addr_file("addresses.json", std::ifstream::binary);
    addr_file >> addresses_obj;
    
    // Заполнение списка адресов
    std::vector<std::string> addrs;
    for (Json::Value::ArrayIndex i = 0; i != addresses_obj["addresses"].size(); i++) {
        addrs.push_back(addresses_obj["addresses"][i].asString());
    }

    // Количество адресов
    int addr_count = addrs.size();

    // Опрашиваемые сокеты
    std::vector<zmq::socket_t> sockets;

    // Опрашивающий поллер
    zmq::poller_t<int> poller;

    // Создание сокетов
    for (int i = 0; i < addr_count; i++){
    	sockets.emplace_back(context, zmq::socket_type::req);    
        sockets[i].set(zmq::sockopt::linger, 0);
        // Подключаемся
        sockets[i].connect("tcp://" + addrs[i]);
        // Добавляем к поллеру
        poller.add(sockets[i], zmq::event_flags::pollin);
    }

    // Тайм-аут для серверов
    const std::chrono::milliseconds timeout {3000};
    
    // Входящие события от сокетов
    std::vector<zmq::poller_event<int>> in_events{addr_count};
    
    // Ответившие адреса. Индекс в этом векторе является индексом в векторе сокетов,
    // true - сокет ответил, false - нет
    std::vector<bool> responding_addrs(addr_count, false);

    const std::string data = "get_status";

    while (true){

        // Отправляем get_status
	    for (auto& socket : sockets){
            (socket).send(zmq::buffer(data), zmq::send_flags::none);
	    }

	    // Дожидаемся ответ и получаем количество сокетов, с которых есть ответ
        const int nin = poller.wait_all(in_events, timeout);

        for (int i = 0; i < nin; i++){
	        // Находим индекс сокета, который нам ответил
	        auto s_it = std::find(sockets.begin(), sockets.end(), in_events[i].socket);
	        int s_n = std::distance(sockets.begin(), s_it);

            // Раз сокет ответил, то в векторе нужно по тому же индексу поставить истину  
            responding_addrs[s_n] = true;
            
            // Читаем сообщение
	        std::vector<zmq::message_t> reply_multi;
	        auto ret = zmq::recv_multipart(in_events[i].socket, 
			        std::back_inserter(reply_multi),
			        zmq::recv_flags::none);
	        if (!ret){
	            std::cout << "Ошибка в получении сообщения" << std::endl;
	            return 1;
	        } else {
	            // Записываем сообщение
	            for (int i = 0; i < reply_multi.size(); i++){
	                std::cout << addrs[s_n] << ":" << reply_multi[i].to_string() << " ";
	            }
	            std::cout << std::endl;
	        }
	    }

	    for (int i = 0; i < addr_count; i++) {
	        if (!responding_addrs[i]){
                std::cout << i << "Тайм-аут для сокета " << addrs[i] << std::endl;
            
                // Пересоздаём сокет
	            poller.remove(sockets[i]);
	            sockets[i].close();
	            zmq::socket_t s {context, zmq::socket_type::req};
	            sockets[i] = std::move(s);
                sockets[i].set(zmq::sockopt::linger, 0);
	            sockets[i].connect("tcp://" + addrs[i]);
	            poller.add(sockets[i], zmq::event_flags::pollin);
	        }
	    }
        
        // Зануляем ответившие адреса
        std::fill(responding_addrs.begin(), responding_addrs.end(), false);
    }
    return 0;
}
