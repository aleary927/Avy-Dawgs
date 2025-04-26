import math 
from matplotlib.animation import FuncAnimation
import matplotlib.pyplot as plt 
import dwfpy as dwf
import time
import threading
import numpy as np
import serial

print(f"DWF Version: {dwf.Application.get_version()}")

'''
********************************** 
Types 
**********************************
'''

class Vec2D: 
    '''
    2D vector.
    '''
    def __init__(self, x: float, y: float):
        self.x = x 
        self.y = y

class Polar:
    '''
    Polar coordinates.
    '''
    def __init__(self, mag: float, theta: float):
        self.mag = mag 
        self.theta = theta

'''
******************************** 
Globals 
********************************
'''

mu0 = 4 * np.pi * 1e-7

# closest distance 
min_dist = 0.5
# maximum amplitude to output 
max_amp = 1.65
# factor to scale by
amp_scale = 0

# current location (in meters)
location = Vec2D(2, 0)
# current heading (radians)
heading = np.pi

# timestep between simulation steps
timestep = 2

# amplitues
ant_amp = Vec2D(0, 0)

# previous command (direction)
prev_cmd = ""

# path that was taken
pathx = []
pathy = []

'''
********************************* 
Functions
*********************************
'''


def near_field_dipole_B(x, y): 
    '''
    Calculate near field dipole.
    '''

    global mu0 

    r2 = x**2 + y**2
    if np.any(r2 == 0):
        raise ValueError("Field point at origin (r=0) is singular.")
    prefac = mu0 / (4*np.pi * r2**(2.5))
    Bx = prefac * (3*x**2 - r2)
    By = prefac * (3*x*y)
    return Bx, By


def calc_location(location: Vec2D, heading) -> Vec2D:
    '''
    Calculate the new location based on heading and current location. 
    '''
    
    # movement distance
    dist = 0.05

    deltax = dist * np.cos(heading)
    deltay = dist * np.sin(heading)

    return Vec2D(location.x + deltax, location.y + deltay)


def B_polar(coor: Vec2D) -> Polar:
    '''
    Calculate magnetic field in polar form based on x and y coordinates.
    ''' 
    global mu0

    # distance squared
    r2 = coor.x**2 + coor.y**2
    # scale factor
    prefac = mu0 / (4*np.pi * r2**(2.5))
    # x and y components
    Bx = prefac * (3*coor.x**2 - r2)
    By = prefac * (3*coor.x*coor.y)
    # magnitude
    Bmag = np.sqrt(Bx**2 + By**2)
    # angle
    Bangle = np.atan2(By, Bx)

    return Polar(Bmag, Bangle)

def ant_xy(B: Polar, heading) -> Vec2D:
    '''
    Convert polar magntic field data to x and y components at given orientation.
    '''

    # find difference between field direction and antenna directions
    diff = abs(B.theta - heading)

    # difference can be a max of 90 degress
    if (diff > 3/2*np.pi):
        diff = 2*np.pi - diff
    elif (diff > np.pi):
        diff = 3/2*np.pi - diff
    elif (diff > 1/2*np.pi):
        diff = np.pi - diff

    # find difference between x and y
    diff_y = diff 
    diff_x = 1/2*np.pi - diff_y

    strength_x = np.cos(diff_x) * B.mag
    strength_y = np.cos(diff_y) * B.mag

    return Vec2D(strength_x, strength_y)

def parse_serial(event):
    '''
    Parse incoming serial messages.
    '''
    global heading
    global prev_cmd

    ser = serial.Serial("/dev/ttyUSB0", 115200)

    while not event.is_set():
        buf = ser.readline()

        if str(buf).find("RIGHT") and prev_cmd != "RIGHT": 
            prev_cmd = "RIGHT"
            heading -= 22.5 * np.pi / 180

        elif str(buf).find("LEFT") and prev_cmd != "LEFT":
            prev_cmd = "LEFT"
            heading += 22.5 * np.pi / 180

        elif str(buf).find("FWD") and prev_cmd != "FWD":
            prev_cmd = "FWD"

        elif str(buf).find("UTURN") and prev_cmd != "UTURN":
            prev_cmd = "UTURN"
            heading += np.pi

        if heading > 2*np.pi: 
            heading -= 2 * np.pi
        elif heading < 0: 
            heading += 2 * np.pi

        # buf = ser.read_all()
        print(buf)
        # time.sleep(1)

def sim_step(event):
    '''
    Update the position and heading, and then the x and y amplitudes.
    
    Update the plot to reflect directed movement.
    '''
    global location 
    global ant_amp
    global timestep
    global amp_scale
    global heading

    while not event.is_set():
        print("Updating")

        # get the magnetic field at given location 
        B = B_polar(location)
        # calculate amplitude 
        new_amp = ant_xy(B, heading)
        ant_amp.x = amp_scale * new_amp.x
        if ant_amp.x > max_amp:
            ant_amp.x = max_amp
        ant_amp.y = amp_scale * new_amp.y
        if ant_amp.y > max_amp: 
            ant_amp.y = max_amp

        # update location 
        new_loc = calc_location(location, heading)
        location.x = new_loc.x 
        location.y = new_loc.y

        # heading += 0.0

        # if (heading < 0):
        #     heading = 2 * np.pi + heading
        # elif (heading > 2 * np.pi):
        #     heading -= 2 * np.pi

        print(f"New amplitude: x: {ant_amp.x}; y: {ant_amp.y}")
        print(f"New location: x: {location.x}; y: {location.y}")

        # sleep until next update
        time.sleep(timestep)

def gen_sig(event, wavegen):
    '''
    Generate the beacon signal.
    '''
    while not event.is_set():
        # pulse on
        print("Pulsing on for 70 ms.")
        wavegen[0].setup("sine", frequency=457e3, amplitude=ant_amp.x, offset=1.65, start=True)
        wavegen[1].setup("sine", frequency=457e3, amplitude=ant_amp.y, offset=1.65, start=True)
        time.sleep(70e-3)
        # pulse off
        print("Pulsing off for 400 ms.")
        wavegen[0].setup("dc", offset=1.65, start=True)
        wavegen[1].setup("dc", offset=1.65, start=True)
        time.sleep(400e-3)


'''
**************************** 
Main Application
**************************** 
'''

if __name__ == "__main__":

    mag_test = B_polar(Vec2D(min_dist, 0.0))
    mag = mag_test.mag

    amp_scale = max_amp / mag

    print(f"max magnitude: {mag_test.mag}")
    print(f"mag scale: {amp_scale}")

    # while 1:
    #     pass

    # while 1:
    #     pass

    with dwf.Device() as device: 

        print(f"Found device: {device.name} ({device.serial_number})") 

        # supplies
        device.analog_io[0][1].value = 5
        device.analog_io[0][0].value = True
        device.analog_io[1][1].value = -2.5
        device.analog_io[1][0].value = True
        device.analog_io.master_enable = True


        halfsize = 5
        x = np.linspace(-halfsize, halfsize)
        y = np.linspace(-halfsize, halfsize)
        # x = np.linspace(-1, 1, 20)
        # y = np.linspace(-1, 1, 20)
        X, Y = np.meshgrid(x, y)

        # Compute B-field components
        Bx, By = near_field_dipole_B(X, Y)

        # Normalize vectors to equal length
        mag = np.sqrt(Bx**2 + By**2)
        U = Bx / mag
        V = By / mag

        # Plot quiver with uniform arrow length
        plt.ion()
        plt.figure()
        plt.quiver(X, Y, U, V)
        plt.axis('equal')
        plt.xlabel('x (m)')
        plt.ylabel('y (m)')
        plt.title('Normalized Near-Field Dipole Flux Lines (m along +x)')

        pathx = [location.x]
        pathy = [location.y]
        graph = plt.plot(pathx, pathy, color="g")[0]

        plt.show()

        scope = device.analog_input 
        wavegen = device.analog_output 

        # input("Connect waveform generator to oscilloscope:\n- W1 to 1+\n- GND to 1-\nPress Enter to continue...")

        stop_event = threading.Event()
        thread1 = threading.Thread(target=gen_sig, args=(stop_event, wavegen))
        thread1.start()

        thread2 = threading.Thread(target=sim_step, args=(stop_event,))
        thread2.start()# Create grid in the z=0 plane

        thread3 = threading.Thread(target=parse_serial, args=(stop_event,))
        thread3.start()

        # loop and update signal str
        while 1: 
            pathx.append(location.x)
            pathy.append(location.y)

            graph.remove()
            graph = plt.plot(pathx, pathy, color="g")[0]
            plt.pause(timestep)
