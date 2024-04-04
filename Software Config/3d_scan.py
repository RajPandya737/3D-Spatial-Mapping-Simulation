# Written by Raj Pandya
# For a 3D plot, scanning 32 times at 11.25 degrees each time
# Generates 3 Z layers of data assuming they are spread out by 10 centimeters each

# Note this is written to be run with matplotlib

import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import math

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

a = []
b = []
c = []
for i in range(32):
    val = int(s.readline().decode())
    y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
    a.append((x/10, y/10))
a.append((a[0][0], a[0][1]))
print(a)


for i in range(32, 0, -1):
    val = int(s.readline().decode())
    y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
    b.append((x/10, y/10))
b.append((b[0][0], b[0][1]))
print(b)

for i in range(32):
    val = int(s.readline().decode())
    y,x = val * math.cos(pi*i/16), val * math.sin(pi*i/16)
    c.append((x/10, y/10))

c.append((c[0][0], c[0][1]))

print(data)


print("Closing: " + s.name)
s.close()



x1 = [x[0] for x in a]
x2 = [x[0] for x in b]
x3 = [x[0] for x in c]
y1 = [x[1] for x in a]
y2 = [x[1] for x in b]
y3 = [x[1] for x in c]



x_segments = [x1, x2, x3]
y_segments = [y1, y2, y3]
z_values = [0, 10, 20]


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

for i in range(3):
    ax.plot(x_segments[i], y_segments[i], zs=z_values[i], color='black')  # Connect points with black lines
    ax.scatter(x_segments[i], y_segments[i], z_values[i], color='blue', marker='o')  # Plot points

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title('3D Plot')

plt.show()

# fig = plt.figure()
# ax = fig.add_subplot(111, projection='3d')

# for i in range(3):
#     ax.plot(x_segments[i], y_segments[i], zs=z_values[i])

# ax.set_xlabel('X')
# ax.set_ylabel('Y')
# ax.set_zlabel('Z')
# ax.set_title('3D Plot')

# plt.show()