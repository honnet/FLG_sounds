#!/usr/bin/env python
# vim: set ai: set et: set ts=4; set sw=4:
import time, os, sys
import glob
import subprocess
#from pyfirmata import Arduino, util
import itertools
import random
import threading
from Queue import Queue, Empty, Full
from collections import deque
import re
import serial

SOUND_ROOT = '/home/pi/FLG_sounds'
INHALE_SOUNDS = glob.glob( os.path.join(SOUND_ROOT, 'normalized/inhale_[0-9]*.wav' ) )
EXHALE_SOUNDS = glob.glob( os.path.join(SOUND_ROOT, 'normalized/exhale_[0-9]*.wav' ) )
MIN_BREATH_SPEED = 0.6
MAX_BREATH_SPEED = 2.5
GROWTH_LIMIT=0.0003
GROWTH_INCREMENT = 5
DECAY_INTERVAL=1 # seconds

SERIAL_PORT_PATTERN = '/dev/ttyAMA*'
RING_BUFFER_SIZE = 8

IR_PINS = [0,1,2,3]
#IR_EVENT_THRESHOLD = 0.05
IR_EVENT_THRESHOLD = 20

FELT_PINS = [4]


class SerialReader(threading.Thread):
    daemon = True
    msg_pattern = re.compile(r'^!([0-9,i]+)\.$')
    #device_pattern = '/dev/ttyACM*'
    #device_pattern = '/dev/ttyUSB*'
    device_pattern = '/dev/ttyASM*'

    def __init__(self):
        threading.Thread.__init__(self)
        self.channel_buffers = None
        self.connected = False

    def run(self):
        devs = glob.glob(self.device_pattern)
        print "Waiting for serial device to come up..."
        while len(devs) == 0:
            time.sleep(0.5)
            devs = glob.glob(SERIAL_PORT_PATTERN)
        dev = devs[0]
        print "Connecting to device %s" % dev
        self.serial = serial.Serial(dev, 19200, timeout=1)
        self.connected = True
        print "Connected"

        while True:
            line = self.serial.readline()
            line = line.strip()
            match = self.msg_pattern.search(line)
            if not match:
                print "Bad packet."
                continue
            analog_values = match.groups()[0].split(',')
            if not self.channel_buffers:
                # initalize one ring buffer per channel in the sample
                self.channel_buffers = [ deque(maxlen=RING_BUFFER_SIZE) for v in analog_values ]
            for i, v in enumerate(analog_values):
                self.channel_buffers[i].append(int(v))
            time.sleep(0.05) # secs

    def get_pin_value(self, n):
        try:
            return self.channel_buffers[n].popleft()
        except :
            return None

def median(mylist):
    sorts = sorted(mylist)
    length = len(sorts)
    if not length % 2:
        return (sorts[length / 2] + sorts[length / 2 - 1]) / 2.0
    return sorts[length / 2]

def play_sound(filename, speed=1.0, vol=1.0, block=False):
    pass
    filename = os.path.join(SOUND_ROOT, filename)
    print "Playing %s" % filename
    out = open('/dev/null', 'w')
    #out = None
    p = subprocess.Popen(('play',filename, 'tempo', str(speed), 'vol', str(vol)), stdout=out, stderr=out)
    if block:
        p.wait()
    return p


def readIR(analog_pin):
    return serial_reader.get_pin_value(analog_pin)
    num_samples = 8
    sample = None
    while True:
        samples = []
        while len(samples) < num_samples:
           sample = serial_reader.get_pin_value(analog_pin)
           if sample:
               samples.append(sample)
        return median(samples)

def gen_breathing_sounds():
    while True:
        yield random.choice(INHALE_SOUNDS)
        yield random.choice(EXHALE_SOUNDS)


breathspeed = 0.8
class Breather(threading.Thread):
    def __init__(self, *args, **kwargs):
        super(Breather, self).__init__(*args, **kwargs)
        self.daemon = True
        self.speed = 0.8
        self.queue = Queue()

    def set_speed(self, speed):
        print "Set speed: %d" % speed
        self.queue.put(speed)

    def run(self):
        print "Starting Breather"
        while True:
            while not self.queue.empty():
                self.speed = self.queue.get()
            #print "Breathing speed: %f" % self.speed
            play_sound(breathing_sounds.next(), speed=self.speed, block=True)

    @classmethod
    def counter_to_speed(klass, counter):
        return float(counter.value) / float(counter.max_value) * (MAX_BREATH_SPEED - MIN_BREATH_SPEED) + MIN_BREATH_SPEED

class Looper(threading.Thread):
    def __init__(self, soundfile, speed=1.0, vol=1.0, *args, **kwargs):
        super(Looper, self).__init__(*args, **kwargs)
        self.daemon = True
        self.speed = speed
        self.vol = vol
        self.soundfile = soundfile


    def run(self):
        while True:
            play_sound(self.soundfile, speed=self.speed, vol=self.vol, block=True)
            time.sleep(0.10)

class ActivityCounter(object):
    def __init__(self, max_value=60, growth_limit=GROWTH_LIMIT, decay_interval=DECAY_INTERVAL):
        self.value = 0
        self.max_value = max_value

        self.decay_interval = decay_interval # in seconds
        self.growth_limit = growth_limit # in seconds

        self.last_decay_time = time.time()
        self.last_grow_time = time.time()

    def update(self):
        if time.time() - self.last_decay_time > self.decay_interval:
            if self.value > 0:
                self.value -= 1
            self.last_decay_time = time.time()
        if int(self) > 0:
            print "Counter: %d" % int(self)
        sys.stdout.flush()

    def __iadd__(self, n):
        if time.time() - self.last_grow_time > self.growth_limit:
            while self.value < self.max_value and n > 0:
                self.value += 1
                n -= 1
            self.last_grow_time = time.time()
        return self

    def __isub__(self, n):
        self.value -= n
        return self

    def __int__(self):
        return self.value

class IRSensor(object):
    def __init__(self, pin, counter):
        self.pin = pin
        self.value = 0
        self.prior_value = 0
        self.counter = counter
        print "IR Sensor on analog %d" % pin

    def update(self):
        self.prior_value = self.value
        newvalue = readIR(self.pin)
        print newvalue
        if newvalue:
            self.value = newvalue
        delta = self.value - self.prior_value
        if delta > IR_EVENT_THRESHOLD:
            print "IR %d delta: %d" % (self.pin, self.value - self.prior_value)
            self.counter += GROWTH_INCREMENT
            print "Counter ++"

class FeltSensor(object):
    def __init__(self, pin):
        self.pin = pin
        self.value = 0
        self.last_value = 0
        self.sounds = glob.glob( os.path.join(SOUND_ROOT, 'ddt_felt_sounds/Felt*.wav') )

    def scale(self, x , in_min,in_max , out_min,out_max):
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    def update(self):
        self.last_value = self.value
        r = readIR(self.pin)
        if r:
            self.value = {True: 1, False:0}[r >= 50]
            #print "Felt %d: %F" % (self.pin, self.value)
            pitch = self.scale(r , 50,1023 , 2.5,0.6)
            # sensor range: min => 1023
            # pitch range:  2.5 => 0.6
            if self.last_value == 0 and self.value == 1:
                self.trigger_sound(pitch)

    def trigger_sound(self, pitch):
        soundfile = random.choice(self.sounds)
        play_sound(soundfile, speed = pitch, vol="5dB")



if __name__ == '__main__':

    # Setup pyfirmata for arduino reads
    acm_no = 0

    breathing_sounds = gen_breathing_sounds()

    serial_reader = SerialReader()
    serial_reader.start()

    breather = Breather()
    breather.start()

    scraper = Looper('normalized/scrapes_loop.wav', vol='-3dB')
    scraper.start()

    counter = ActivityCounter()
    ir_sensors = [ IRSensor(pin, counter) for pin in IR_PINS  ]
    felt_sensors = [ FeltSensor(pin) for pin in FELT_PINS ]

    speed = 0.8
    while True:
        for irs in ir_sensors:
            irs.update()
        counter.update()
        #print("Speed: %f" % Breather.counter_to_speed(counter))
        breather.set_speed(Breather.counter_to_speed(counter))

        for felt in felt_sensors:
            felt.update()

        time.sleep(0.10)
