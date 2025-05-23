import socket
import select

def echo_server(host='127.0.0.1', port=8080):
    # 创建服务器套接字
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(5)
    server_socket.setblocking(False)

    print(f"Echo server is running on {host}:{port}")

    # 使用 select 进行多路复用
    inputs = [server_socket]  # 监听的输入套接字列表
    outputs = []  # 需要写入的套接字列表
    message_queues = {}  # 存储客户端消息的队列

    try:
        while True:
            readable, writable, exceptional = select.select(inputs, outputs, inputs)

            # 处理可读的套接字
            for s in readable:
                if s is server_socket:
                    # 接受新连接
                    client_socket, client_address = server_socket.accept()
                    print(f"New connection from {client_address}")
                    client_socket.setblocking(False)
                    inputs.append(client_socket)
                    message_queues[client_socket] = b""
                else:
                    # 接收客户端消息
                    data = s.recv(1024)
                    if data:
                        print(f"Received: {data.decode()} from {s.getpeername()}")
                        message_queues[s] += data
                        if s not in outputs:
                            outputs.append(s)
                    else:
                        # 客户端关闭连接
                        try:
                            print(f"Closing connection to {s.getpeername()}")
                        except socket.error:
                            print("Socket already closed")
                        if s in outputs:
                            outputs.remove(s)
                        inputs.remove(s)
                        s.close()
                        if s in message_queues:
                            del message_queues[s]

            # 处理可写的套接字
            for s in writable:
                if message_queues[s]:
                    sent = s.send(message_queues[s])  # 回射消息
                    message_queues[s] = message_queues[s][sent:]
                if not message_queues[s]:
                    outputs.remove(s)

            # 处理异常的套接字
            for s in exceptional:
                print(f"Handling exceptional condition for socket {s}")
                inputs.remove(s)
                if s in outputs:
                    outputs.remove(s)
                s.close()
                if s in message_queues:
                    del message_queues[s]

    except KeyboardInterrupt:
        print("Server is shutting down...")
    finally:
        server_socket.close()

if __name__ == "__main__":
    echo_server()