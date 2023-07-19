import sys
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

mac = collections.deque(maxlen=50 * 30)
filtered_breath = collections.deque(maxlen=50 * 30)
time = collections.deque(maxlen=50 * 30)
up_intercepts = collections.deque(maxlen=50 * 30)
down_intercepts = collections.deque(maxlen=50 * 30)
peaks = collections.deque(maxlen=50 * 30)
valleys = collections.deque(maxlen=50 * 30)

peak_helper = collections.deque(maxlen=500)
valley_helper = collections.deque(maxlen=500)

app = pg.mkQApp("Live plotting")
win = pg.GraphicsLayoutWidget(show=True, title="CSI plots")
win.resize(1000, 2000)
win.setWindowTitle('Live CSI data plots')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)

filtered_breath_plot = win.addPlot(title="Estimated breathing rate", row=4, col=0)

filtered_breath_curve = filtered_breath_plot.plot()
mac_curve = filtered_breath_plot.plot()
up_intercepts_curve = filtered_breath_plot.plot(pen=None, symbol='o', symbolBrush=(255, 0, 0))
down_intercepts_curve = filtered_breath_plot.plot(pen=None, symbol='+', symbolBrush=(0, 255, 0))
peaks_curve = filtered_breath_plot.plot(pen=None, symbol='x', symbolBrush=(0, 0, 255))
valleys_curve = filtered_breath_plot.plot(pen=None, symbol='t', symbolBrush=(255, 255, 0))

udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind((HOST, PORT))


def update():
    line = ""
    try:
        sys.stdin.buffer.flush()
        line = sys.stdin.buffer.readline().decode('utf-8').replace("\n", "")
    except:
        pass  # might not be an utf-8 string!


    print(line)
    if "testing data" in line:
        msg, time_value, filtered_breath_value, mac_value, found_up_intercept, found_down_intercept, \
            found_valley, valley_index_difference_from_last_intercept, \
            found_peak, peak_index_difference_from_last_intercept = line.split(',')
        # msg, heart_rate_value, breathing_rate_value = data.decode("utf-8").split(',')

        time.append(int(time_value))
        filtered_breath.append(float(filtered_breath_value))
        mac.append(float(mac_value))

        filtered_breath_array = np.array(filtered_breath)
        time_array = np.array(time)
        mac_array = np.array(mac)


        peak_helper.append([int(time_value), float(filtered_breath_value)])
        valley_helper.append([int(time_value), float(filtered_breath_value)])

        if int(found_peak) > 0:
            print("found_peak", int(found_peak), int(peak_index_difference_from_last_intercept), len(peak_helper))
            peaks.append(peak_helper[int(peak_index_difference_from_last_intercept)-1])
        if int(found_valley) > 0:
            print("found_valley")
            valleys.append(valley_helper[int(valley_index_difference_from_last_intercept)-1])

        if int(found_up_intercept) > 0:
            print("found_up_intercept")
            up_intercepts.append([time_array[-2], float(mac_value)])
            peak_helper.clear()
        if int(found_down_intercept) > 0:
            print("found_down_intercept")
            down_intercepts.append([time_array[-2], float(mac_value)])
            valley_helper.clear()

        filtered_breath_curve.setData(time_array, filtered_breath_array)
        mac_curve.setData(time_array, mac_array)
        if peaks:
            peaks_curve.setData(np.array(peaks)[:, 0], np.array(peaks)[:, 1])
        if valleys:
            valleys_curve.setData(np.array(valleys)[:, 0], np.array(valleys)[:, 1])
        if up_intercepts:
            up_intercepts_curve.setData(np.array(up_intercepts)[:, 0], np.array(up_intercepts)[:, 1])
        if down_intercepts:
            down_intercepts_curve.setData(np.array(down_intercepts)[:, 0], np.array(down_intercepts)[:, 1])
    # breathing_rate_text.setText(text=breathing_rate_value, color=(200, 200, 200))


if __name__ == '__main__':
    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(1)
    pg.exec()
