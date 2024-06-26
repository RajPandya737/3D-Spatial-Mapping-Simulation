# Written by Raj Pandya
# For a 3D plot, scanning 32 times at 11.25 degrees each time
# Generates num_scan Z layers of data assuming they are spread out by 1 meter each

# this code is written to be run with matplotlib, to run with open3d, print the 
# list contents, paste them into the conversion.py script, then run the open3d script

import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import math
num_scans = 8 # must be even number

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
        layer.append((x, y))
    layer.append((layer[0][0], layer[0][1]))
    x_y_coords.append(layer)
    print(x_y_coords)

    layer = []
    for i in range(32, 0, -1):
        val = int(s.readline().decode())
        y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
        layer.append((x, y))
    layer.append((layer[0][0], layer[0][1]))
    x_y_coords.append(layer)
    print(x_y_coords)



print("Closing: " + s.name)
s.close()





x_segments = [[x[0] for x in sublist] for sublist in x_y_coords]

y_segments = [[x[1] for x in sublist] for sublist in x_y_coords]

x_segments = [segment[::-1] if i % 2 == 0 else segment for i, segment in enumerate(x_segments)]
y_segments = [segment[::-1] if i % 2 == 0 else segment for i, segment in enumerate(y_segments)]

z_values = [2 * i for i in range(num_scans)]

lines = [[(x_segments[i][j], y_segments[i][j], 2*i) for i in range(num_scans)] for j in range(33)]
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

for i in range(num_scans):
    ax.plot(x_segments[i], y_segments[i], zs=z_values[i], color='black', linewidth=1)  # Lines in black
    ax.scatter(x_segments[i], y_segments[i], zs=z_values[i], color='black', s=5)  # Points in red, adjust size (s) as needed

for single_line in lines:
    single_line_x, single_line_y, single_line_z = zip(*single_line)
    ax.plot(single_line_x, single_line_y, zs=single_line_z, color='black', linewidth=2)  # Plotting single_line in blue



print(x_segments)
print(y_segments)
print(z_values)


ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title('3D Plot')

plt.show()