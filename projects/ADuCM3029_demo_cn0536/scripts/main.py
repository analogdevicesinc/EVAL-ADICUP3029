import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from parse import parse
from queue import Queue

import time

MEASURMENT_PERIOD=10

fig = plt.figure()
ax1 = fig.add_subplot(111)

q = Queue()

NB_SAMPLES = 10
TIME_BETWEEN_SAMPLES = 10000 #miliseconds
MAX_RADIATION = 0.7
SAFE_TRESHOLD = 0.21
SUBSCRIBE_TOPIC = "analog_test_topic"
# In ms
DATA_FREQUENCY = 1000

d = [0] * NB_SAMPLES
thres = [SAFE_TRESHOLD] * NB_SAMPLES
t = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
for i in range(0, 10):
    t[i] = t[i] * MEASURMENT_PERIOD
time_s = NB_SAMPLES * MEASURMENT_PERIOD

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(SUBSCRIBE_TOPIC)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global i
    #print(msg.topic+" "+str(msg.payload))
    try:
        data = msg.payload.decode('utf-8')
        print(data)
        values = parse('Total count:{}; CPM:{} us_per_hour:{}\r\n', data)
        new_data = values.fixed[2]
        q.put(float(new_data))
    except Exception as e:
        print(e)
    
def animate(i):
    global time_s
    try:
        d.pop(0)
        d.append(q.get())
        t.pop(0)
        t.append(time_s)
        time_s = time_s + MEASURMENT_PERIOD
        ax1.clear()
        ax1.axis([t[0], t[len(t) - 1], 0, MAX_RADIATION])
        plt.xlabel("Seconds")
        plt.ylabel("us/hour")
        plt.title("MQTT geiger counter data")
        ax1.plot(t, thres, label = 'Safe threshold')
        ax1.plot(t, d, label = 'Data')
        plt.legend(loc = 'upper right')
    except Exception as e:
        print(e)

def main():

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org", 1883, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_start()
    ani = animation.FuncAnimation(fig, animate, interval=TIME_BETWEEN_SAMPLES)
    plt.show()
    client.loop_stop()

if __name__ == "__main__":
    # execute only if run as a script
    main()