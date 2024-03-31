# Written by Raj Pandya
# For a 3D plot, scanning 32 times at 11.25 degrees each time
# Generates 3 Z layers of data assuming they are spread out by 10 centimeters each

import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import math

num_scans = 4 # must be even number

# Initialize the serial port
s = serial.Serial(port='COM3', baudrate=115200, timeout=10)
print("Opening: " + s.name)

data = []

# Reset the buffers of the UART port to delete the remaining data in the buffers
s.reset_output_buffer()
s.reset_input_buffer()

# Wait for the user's signal to start the program



# Send the character 's' to MCU via UART
# This will signal MCU to start the transmission
s.write(b's')  # Assuming 's' is encoded as bytes

pi = math.pi

x_y_coords = []


for n in range(int(num_scans/2)):
    layer = []
    for i in range(32):
        val = int(s.readline().decode())
        y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
        layer.append((x/10, y/10))
    layer.append((layer[0][0], layer[0][1]))
    x_y_coords.append(layer)
    print(x_y_coords)

    layer = []
    for i in range(32, 0, -1):
        val = int(s.readline().decode())
        y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
        layer.append((x/10, y/10))
    layer.append((layer[0][0], layer[0][1]))
    x_y_coords.append(layer)
    print(x_y_coords)



print("Closing: " + s.name)
s.close()





x_segments = [[x[0] for x in sublist] for sublist in x_y_coords]
y_segments = [[x[1] for x in sublist] for sublist in x_y_coords]
z_values = [10 * i for i in range(num_scans)]

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

for i in range(num_scans):
    ax.plot(x_segments[i], y_segments[i], zs=z_values[i])

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title('3D Plot')

plt.show()