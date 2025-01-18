import time
import zmq

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

while True:
    message = socket.recv()
    print("Получили сообщение: ", message)
    
    # Имитация бурной деятельности
    time.sleep(1)
    
    print("Шлём, что всё хорошо")
    socket.send_string("ok")
    
    print("Получили ещё сообщение")
    message = socket.recv()
    time.sleep(1)
    
    print("Шлём, что всё плохо")
    socket.send_multipart([b"error", b"An infernal error!"])
    
    message = socket.recv()
    print("Ничего не шлём")
    time.sleep(5)
    
    print("Что-то присылаем")
    socket.send_string("ich bin krank")
