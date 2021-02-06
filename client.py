import socket, struct
from Crypto.Cipher import ARC4

HOST = '192.168.0.105'  
PORT = 5555
#HOST = '81.70.160.41'
#PORT = 55555
socket_num = 1
rc4_key = b'\x01\x23\x45\x67\x89\xab\xcd\xef'

def rc4_decrypt(cipher, key):
    rc4 = ARC4.new(key)
    return rc4.decrypt(cipher)
def rc4_encrypt(cipher, key):
    rc4 = ARC4.new(key)
    return rc4.encrypt(cipher)


def test1():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        send_data = ('num' + str(i) + ' ' + 'a'*1000).encode()
        send_data = rc4_encrypt(send_data, rc4_key)
        s.sendall(send_data)
        print('send', i)

    for i in range(socket_num):
        s = ls[i]
        recv_data = s.recv(1024)
        print('recv', i)
        recv_data = rc4_decrypt(recv_data, rc4_key)
        print(len(recv_data), recv_data[:30])

def test2():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        with open('data.txt', 'rb')as f:
            send_data = str(i).encode() + f.read()
        send_data = rc4_encrypt(send_data, rc4_key)
        s.sendall(send_data)
        print('send', i)

    for i in range(socket_num):
        s = ls[i]
        recv_data = s.recv(1024)
        print('recv', i)
        recv_data = rc4_decrypt(recv_data, rc4_key)
        #print(recv_data)
        with open('.\client recv\\' + str(i) + '.txt', 'wb')as f:
            f.write(recv_data)
        print(len(recv_data), recv_data[:10])


def test3():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        send_data = ('num' + str(i) + ' ' + 'b'*1000).encode()
        s.sendall(send_data)
        print('send', i)

    for i in range(socket_num):
        s = ls[i]
        recv_data = s.recv(1024)
        print('recv', i)
        print(len(recv_data), recv_data[:30])

def test4():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        send_data = ('num' + str(i) + ' ' + 'b'*1024*15).encode()
        s.sendall(send_data)
        print('send', i)
    while True:
        for i in range(socket_num):
            s = ls[i]
            recv_data = s.recv(1024*16)
            print('recv', i)
            print(len(recv_data), recv_data[:30])


def test5():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        data = 'Hello world!'
        send_data = struct.pack('<I', len(data)) + (data).encode()
        #print(send_data)
        s.sendall(send_data)
        print('send', i)
    for i in range(socket_num):
        s = ls[i]
        recv_data = s.recv(1024)
        print('recv', i)
        print(len(recv_data), recv_data)

def test6():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)
    for i in range(socket_num):
        s = ls[i]
        recv_data = s.recv(1024)
        print('recv', i)
        print(len(recv_data), recv_data)

def test7():
    ls = []
    for i in range(socket_num):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        ls.append(s)
        print('connect', i)

    for i in range(socket_num):
        s = ls[i]
        data = 'Hello world! I am iyzyi. And I from BXS@CUMT! HOW ARE YOU?'
        send_data = struct.pack('<I', len(data)+7) + b'\x01\x02' + struct.pack('<I', 0x12345678) + b'\x69' + (data).encode()
        print(send_data)
        s.sendall(send_data)
        print('send', i)

test7()
