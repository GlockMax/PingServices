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

    socket.send_string("Ну здравствуй")
