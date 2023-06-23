import socket
import collections

import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore

HOST = ''
PORT = 8081


# time range (in seconds) for which to plot the last breathing and heart rate estimates for
estimate_plot_time_range = 60

estimates_deque_length = round(estimate_plot_time_range / 2)

breathing_rate = collections.deque(maxlen=estimates_deque_length)
heart_rate = collections.deque(maxlen=estimates_deque_length)

app = pg.mkQApp("Live plotting")
win = pg.GraphicsLayoutWidget(show=True, title="CSI plots")
win.resize(1000, 2000)
win.setWindowTitle('Live CSI data plots')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)

breathing_rate_plot = win.addPlot(title="Estimated breathing rate", row=4, col=0)
breathing_rate_text = pg.TextItem(text="--", color=(200, 200, 200))
breathing_rate_text.setPos(-estimate_plot_time_range, 5)
breathing_rate_plot.addItem(breathing_rate_text)

heart_rate_plot = win.addPlot(title="Estimated heart rate", row=5, col=0)
heart_rate_text = pg.TextItem(text="--", color=(200, 200, 200))
heart_rate_text.setPos(-estimate_plot_time_range, 40)
heart_rate_plot.addItem(heart_rate_text)

breathing_rate_curve = breathing_rate_plot.plot()
heart_rate_curve = heart_rate_plot.plot()

udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind((HOST, PORT))


def update():
    data = udp_socket.recv(1024)
    # update graph
    msg, heart_rate_value, breathing_rate_value = data.decode("utf-8").split(',')

    heart_rate.append(float(heart_rate_value))
    x = np.arange(len(heart_rate)) * 2 * -1
    heart_rate_curve.setData(np.flip(x), np.array(heart_rate))
    heart_rate_text.setText(text=heart_rate_value, color=(200, 200, 200))

    breathing_rate.append(float(breathing_rate_value))
    x = np.arange(len(breathing_rate)) * 2 * -1
    breathing_rate_curve.setData(np.flip(x), np.array(breathing_rate))
    breathing_rate_text.setText(text=breathing_rate_value, color=(200, 200, 200))



if __name__ == '__main__':
    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(1)
    pg.exec()




