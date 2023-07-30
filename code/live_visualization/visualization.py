import kconfiglib
import collections
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore

# use kconfiglib to parse the receivers config file and change plotting options accordingly

kconfig = kconfiglib.Kconfig("../csi_receiver/main/Kconfig.projbuild")
kconfig.load_config("../csi_receiver/sdkconfig")

# test, just print some config values
f = kconfig.syms["FRAMES_PER_SECOND"]
print("fps", f.user_value)

f = kconfig.syms["MRC_PCA_ON_TIMER"]
print("on timer", f.user_value)

f = kconfig.syms["MRC_PCA_EVERY_X_SECONDS"]
print("timer interval", f.user_value)

f = kconfig.syms["ESP_WIFI_SSID"]
print("ssid", f.user_value)

option_names = {
    "sti": "SENSE_PRINT_STI",
    "fused_heart": "SENSE_PRINT_FUSED_HEART",
    "fused_breath": "SENSE_PRINT_FUSED_BREATH",
    "filtered_heart": "SENSE_PRINT_FILTERED_HEART",
    "filtered_breath": "SENSE_PRINT_FILTERED_BREATH",
    "heart_features": "SENSE_PRINT_HEART_FEATURES",
    "breath_features": "SENSE_PRINT_BREATHING_FEATURES",
    "heart_poi": "SENSE_PRINT_HEART_POI",
    "breath_poi": "SENSE_PRINT_BREATHING_POI",
    "detection": "SENSE_PRINT_DETECTION"
}
# TODO change suffix according to which output is selected in config and for this viz
option_suffix = "_SD"

if kconfig.syms["SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS"].user_value == 2:
    print("log different things")
    if kconfig.syms["SENSE_LOG_TO_UDP"].user_value == 2:
        print("log to udp")
        option_suffix = "_U"
    if kconfig.syms["SENSE_LOG_TO_SERIAL"].user_value == 2:
        print("log to serial")
        option_suffix = "_S"


option_indices = {
    "sti": 1,
    "fused_heart": 2,
    "fused_breath": 3,
    "filtered_heart": 4,
    "filtered_breath": 5,
    "heart_features": 6,
    "breath_features": 7,
    "heart_poi_found": 8,
    "new_heart_poi": 9,
    "breath_poi_found": 10,
    "new_breath_poi": 11,
    "detected_presence": 12,
    "detected_small_movement": 13,
    "detected_large_movement": 14
}

reduce_index_by = 0

if kconfig.syms[option_names["sti"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["fused_heart"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["fused_breath"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["filtered_heart"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["filtered_breath"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["heart_features"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["breath_features"]+option_suffix].user_value != 2:
    reduce_index_by += 1
if kconfig.syms[option_names["heart_poi"]+option_suffix].user_value != 2:
    reduce_index_by += 2
if kconfig.syms[option_names["breath_poi"]+option_suffix].user_value != 2:
    reduce_index_by += 2
if kconfig.syms[option_names["detection"]+option_suffix].user_value != 2:
    reduce_index_by += 3

print("reduce index by", reduce_index_by)
# use the options in the config file to know what to expect in a line
row = 0

if kconfig.syms[option_names["sti"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["fused_heart"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["fused_breath"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["filtered_heart"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["filtered_breath"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["heart_features"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["breath_features"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()

if kconfig.syms[option_names["heart_poi"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()
if kconfig.syms[option_names["breath_poi"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()
if kconfig.syms[option_names["detection"]+option_suffix].user_value == 2:
    filtered_breath = collections.deque(maxlen=MAXLEN)
    _plot = win.addPlot(title="", row=4, col=0)
    _curve = filtered_breath_plot.plot()



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

