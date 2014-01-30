import os
import time
import threading
import Queue

import serial
from pyfirmata import util


# Commands
SET_BALANCE = 'S'
SET_DEBUG_LEVEL = 'T'
SET_COIN_COUNT = 'U'
OUTPUT_TEST = 'O'
DO_SHAKE_TEST = 'P'


def get_first_port():
    for resource in os.listdir('/dev'):
        if resource.startswith('tty.usb'):
            return '/dev/%s' % resource


def send_cmd_serial(connection, to, operation, operand1=0, operand2=0):
    operand2_str = "%s%s" % util.to_two_bytes(operand2)
    string = 'AF%sZ%s%s%sFA' % (to, operation, operand1, operand2_str)
    return connection.write(string)


def send_cmd(to, operation, operand1=0, operand2=0):
    operand2_str = "%s%s" % util.to_two_bytes(operand2)
    string = 'AF%sZ%s%s%sFA' % (to, operation, operand1, operand2_str)
    print "sending %s" % string
    return connection.write(string)


def read(listen_to=None):
    try:
        while not queue.empty():
            if listen_to == None:
                print queue.get_nowait()
            else:
                msg = queue.get_nowait()
                if msg[1:2] in listen_to or msg[2:3] in listen_to:
                    print msg
    except KeyboardInterrupt:
        return


def read_sock():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    while True:
        data, addr = sock.recvfrom(1024)
        print data



class SerialFlusher(threading.Thread):
    def __init__(self, connection, queue):
        super(SerialFlusher, self).__init__()
        self.running = True
        self.connection = connection
        self.queue = queue

    def run(self):
        while self.running:
            try:
                while self.connection.inWaiting():
                    self.queue.put(self.connection.readline())
                time.sleep(0.001)
            except (AttributeError, serial.SerialException, OSError), e:
                # this way we can kill the thread by setting the connection object
                # to None, or when the serial port is closed by connection.exit()
                self.queue.put(e)
                break
            except Exception, e:
                # catch 'error: Bad file descriptor'
                # iterate may be called while the serial port is being closed,
                # causing an "error: (9, 'Bad file descriptor')"
                if getattr(e, 'errno', None) == 9:
                    break
                try:
                    if e[0] == 9:
                        break
                except (TypeError, IndexError):
                    pass
                self.queue.put(e)
                raise

    def quit(self):
        self.running = False


def init():
    global connection, queue, sf
    connection = serial.Serial(get_first_port(), 9600)
    queue = Queue.Queue()

    sf = SerialFlusher(connection, queue)
    sf.start()
    return connection, queue, sf


def help():
    print("""
    A. Start update sync
    B. Acknowledge update sync
    C. Update window is open
    D. Start shake sync
    E. Acknowledge shake sync
    F. Do the update!
    Z. Abort window and shake
    S. Set balance
    T. Set debug level
    U. Set coin count
    O. Turn all outputs on for 2 secs
    P. Force a shake to happen""")


# connection, queue, sf = init()

BAUDS = [
    1200,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
]

def setup_xbee():
    port = get_first_port()
    if not port:
        print("No port found")
        return

    print("Connecting on port {}".format(port))
    found = False
    for baudrate in BAUDS:
        print("Trying baudrate {}...".format(baudrate))
        conn = serial.Serial(port, baudrate, timeout=2)
        if not conn.open:
            continue
        conn.write('+++')
        response = conn.readline()
        if response == "OK\r":
            found = True
            break
        print(response)
        print("No luck.")

    if not found:
        print("Could not connect on any baudrate on this port")
        return

    print("Currently configured at {} baud".format(baudrate))

    # Setup ATID
    conn.write('ATID\r')
    response = conn.readline().strip()
    try:
        atid = raw_input("What do you want to set as ID [currently {}]? ".format(response))
    except EOFError:
        atid = reponse
    conn.write('ATID {}\r'.format(atid))

    # Setup ATBD
    try:
        rate = raw_input("What do you want to set as baudrate [currently {}]?"
                     " ".format(baudrate)) or ""
        if not rate:
            rate = baudrate
        try:
            rate = int(rate)
        except ValueError:
            rate = 0
        while rate not in BAUDS and rate is not "":
            print("{0} is not a valid baudrate. Valid rates are:"
                  " {1}".format(rate, ", ".join([str(b) for b in BAUDS])))
            rate = raw_input("What do you want to set as baudrate [currently {}]?"
                         " ".format(baudrate))
    except EOFError:
        rate = baudrate
    if not rate:
        rate = baudrate
    conn.write('ATBD {}\r'.format(BAUDS.index(int(rate))))

    # Write settings
    conn.write('ATWR\r')
    responses = []
    while conn.inWaiting():
        responses.append(conn.readline())
    if responses == ["OK", "OK", "OK"]:
        print("Done. Setup with ATID={0} and ATBD={1}".format(atid, BAUDS.index(rate)))
    else:
        print("Something went wrong. Responses: {}".format(", ".join(responses)))




