# Written by Raj Pandya


# This is for 8 45 degrees scans 


import serial
import matplotlib.pyplot as plt
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

for i in range(8):
    val = int(s.readline().decode())
    y,x = val * math.cos(pi*i/4), val * math.sin(pi*i/4)
    print(val)

    data.append((x, y))
       
# the encode() and decode() function are needed to convert string to bytes
# because pyserial library functions work with type "bytes"


#close the port
print("Closing: " + s.name)
s.close()

#plot the data

x = [item[0] for item in data]
y = [item[1] for item in data]



x.append(x[0])
y.append(y[0])


plt.plot(x, y)
plt.xlabel('Rotation')
plt.ylabel('Distance')
plt.title('Rotation Vs Distance')
plt.show()