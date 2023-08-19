import sys
import socket
import kconfiglib
import argparse
import collections
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore

parser = argparse.ArgumentParser(description="Parses incoming data / features in the format provided by the receiver "
                                             "ESP in realtime and plots the selected features.")

parser.add_argument("--use_udp_stream",
                    default=False)

parser.add_argument("--plot_sti",
                    default=True)
parser.add_argument("--plot_fused_breath",
                    default=True)
parser.add_argument("--plot_fused_heart",
                    default=True)
parser.add_argument("--plot_filtered_breath",
                    default=True)
parser.add_argument("--plot_filtered_heart",
                    default=True)

parser.add_argument("--plot_breath_pois",
                    default=True)
parser.add_argument("--plot_heart_pois",
                    default=True)

parser.add_argument("--plot_detected_presence",
                    default=True)
parser.add_argument("--plot_detected_small_movement",
                    default=True)
parser.add_argument("--plot_detected_large_movement",
                    default=True)


parser.add_argument("--plot_breath_features",
                    default=True)
parser.add_argument("--plot_heart_features",
                    default=True)

# to avoid even more arguments which features are plotted is selected for both heart and breath features
# if both heart and breath feature plotting is enabled the enabled features are plotted for both
parser.add_argument("--plot_instantaneous_peak_rate",
                    default=True)
parser.add_argument("--plot_instantaneous_valley_rate",
                    default=False)
parser.add_argument("--plot_mean_peak_rate_over_window",
                    default=True)
parser.add_argument("--plot_mean_valley_rate_over_window",
                    default=False)
parser.add_argument("--plot_fft_rate_over_window",
                    default=True)
parser.add_argument("--plot_variance_of_peak_rate_in_window",
                    default=True)
parser.add_argument("--plot_variance_of_valley_rate_in_window",
                    default=False)

parser.add_argument("--plot_mean_up_stroke_length",
                    default=False)
parser.add_argument("--plot_mean_down_stroke_length",
                    default=False)
parser.add_argument("--plot_up_stroke_length_variance",
                    default=False)
parser.add_argument("--plot_down_stroke_length_variance",
                    default=False)
parser.add_argument("--plot_up_to_down_length_ratio",
                    default=False)
parser.add_argument("--plot_fractional_up_stroke_time",
                    default=True)

parser.add_argument("--plot_mean_up_stroke_amplitude",
                    default=False)
parser.add_argument("--plot_mean_down_stroke_amplitude",
                    default=False)
parser.add_argument("--plot_up_stroke_amplitude_variance",
                    default=False)
parser.add_argument("--plot_down_stroke_amplitude_variance",
                    default=False)
parser.add_argument("--plot_up_to_down_amplitude_ratio",
                    default=False)
parser.add_argument("--plot_fractional_up_stroke_amplitude",
                    default=False)

parser.add_argument("--plot_amplitudes",
                    default=True)

args = parser.parse_args()

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
    "detection": "SENSE_PRINT_DETECTION",
    "thresholds": "SENSE_PRINT_THRESHOLDS",
    "amplitudes": "SENSE_PRINT_AMPLITUDES",
    "sleep_stage": "SENSE_PRINT_SLEEP_STAGE_CLASSIFICATION"
}

option_suffix = "_SD"
logging_enabled = False
read_from_serial = False
read_from_udp = False
if kconfig.syms["SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS"].user_value == 2:
    print("log different things")
    if kconfig.syms["SENSE_LOG_TO_UDP"].user_value == 2:
        print("log to udp")
        option_suffix = "_U"
        HOST = ''
        PORT = 8081
        udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_socket.bind((HOST, PORT))
        logging_enabled = True
        read_from_udp = True

    if kconfig.syms["SENSE_LOG_TO_SERIAL"].user_value == 2 and not (kconfig.syms["SENSE_LOG_TO_UDP"].user_value == 2 and args.use_udp_stream):
        print("log to serial")
        option_suffix = "_S"
        logging_enabled = True
        read_from_serial = True


if kconfig.syms["SENSE_LOG_DIFFERENT_THINGS_TO_DIFFERENT_OUTPUTS"].user_value == 0:
    print("log same things")
    if kconfig.syms["SENSE_LOG_TO_UDP"].user_value == 2:
        print("log to udp")
        HOST = ''
        PORT = 8081
        udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_socket.bind((HOST, PORT))
        logging_enabled = True
        read_from_udp = True

    if kconfig.syms["SENSE_LOG_TO_SERIAL"].user_value == 2 and not (kconfig.syms["SENSE_LOG_TO_UDP"].user_value == 2 and args.use_udp_stream):
        print("log to serial")
        logging_enabled = True
        read_from_serial = True

if not logging_enabled:
    print("CSI receiver does not log data to serial or UDP, so there is nothing to plot -- exiting now")
    exit(-1)

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
    "detected_large_movement": 14,
    "amplitudes": 21,
    "sleep_stage": 22
}

# set up the window for pyqtgraph plots
app = pg.mkQApp("Live plotting")
win = pg.GraphicsLayoutWidget(show=True, title="CSI plots")
win.resize(1000, 2000)
win.setWindowTitle('Live CSI data plots')
pg.setConfigOptions(antialias=True)

# use the options in the config file to know what to expect in a line
row = 0

MAXLEN = int(kconfig.syms["FRAMES_PER_SECOND"].user_value) * int(kconfig.syms["TIME_RANGE_TO_KEEP"].user_value)

time_queue = collections.deque(maxlen=MAXLEN)
current_time = 0

number_of_subcarriers = 64

if kconfig.syms[option_names["amplitudes"] + option_suffix].user_value == 2 and args.plot_amplitudes:
    carrier_plot = win.addPlot(title="Carrier Plot", row=row, col=0)
    amplitudes_queue = collections.deque(maxlen=MAXLEN)
    carrier_curves = []
    for i in range(number_of_subcarriers):
        curve = carrier_plot.plot()
        carrier_curves.append(curve)
    row += 1

if kconfig.syms[option_names["sti"] + option_suffix].user_value == 2 and args.plot_sti:
    sti_queue = collections.deque(maxlen=MAXLEN)
    sti_plot = win.addPlot(title="sti", row=row, col=0)
    sti_curve = sti_plot.plot()
    row += 1

if kconfig.syms[option_names["fused_heart"] + option_suffix].user_value == 2 and args.plot_fused_heart:
    fused_heart_queue = collections.deque(maxlen=MAXLEN)
    fused_heart_plot = win.addPlot(title="fused_heart", row=row, col=0)
    fused_heart_curve = fused_heart_plot.plot()
    row += 1

if kconfig.syms[option_names["fused_breath"] + option_suffix].user_value == 2 and args.plot_fused_breath:
    fused_breath_queue = collections.deque(maxlen=MAXLEN)
    fused_breath_plot = win.addPlot(title="fused_breath", row=row, col=0)
    fused_breath_curve = fused_breath_plot.plot()
    row += 1

if kconfig.syms[option_names["filtered_heart"] + option_suffix].user_value == 2 and args.plot_filtered_heart:
    filtered_heart_queue = collections.deque(maxlen=MAXLEN)
    filtered_heart_plot = win.addPlot(title="filtered_heart", row=row, col=0)
    filtered_heart_curve = filtered_heart_plot.plot()
    row += 1

if kconfig.syms[option_names["filtered_breath"] + option_suffix].user_value == 2 and args.plot_filtered_breath:
    filtered_breath_queue = collections.deque(maxlen=MAXLEN)
    filtered_breath_plot = win.addPlot(title="filtered_breath", row=row, col=0)
    filtered_breath_curve = filtered_breath_plot.plot()
    row += 1

if kconfig.syms[option_names["heart_features"]+option_suffix].user_value == 2 and args.plot_heart_features:
    hf_row = 0
    if args.plot_instantaneous_peak_rate:
        heart_instantaneous_peak_rate_queue = collections.deque(maxlen=MAXLEN)
        heart_instantaneous_peak_rate_plot = win.addPlot(title="heart_instantaneous_peak_rate", row=hf_row, col=1)
        heart_instantaneous_peak_rate_curve = heart_instantaneous_peak_rate_plot.plot()
        hf_row += 1

    if args.plot_instantaneous_valley_rate:
        heart_instantaneous_valley_rate_queue = collections.deque(maxlen=MAXLEN)
        heart_instantaneous_valley_rate_plot = win.addPlot(title="heart_instantaneous_valley_rate", row=hf_row, col=1)
        heart_instantaneous_valley_rate_curve = heart_instantaneous_valley_rate_plot.plot()
        hf_row += 1

    if args.plot_mean_peak_rate_over_window:
        heart_mean_peak_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_peak_rate_over_window_plot = win.addPlot(title="heart_mean_peak_rate_over_window", row=hf_row, col=1)
        heart_mean_peak_rate_over_window_curve = heart_mean_peak_rate_over_window_plot.plot()
        hf_row += 1

    if args.plot_mean_valley_rate_over_window:
        heart_mean_valley_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_valley_rate_over_window_plot = win.addPlot(title="heart_mean_valley_rate_over_window", row=hf_row, col=1)
        heart_mean_valley_rate_over_window_curve = heart_mean_valley_rate_over_window_plot.plot()
        hf_row += 1

    if args.plot_fft_rate_over_window:
        heart_fft_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        heart_fft_rate_over_window_plot = win.addPlot(title="heart_fft_rate_over_window", row=hf_row, col=1)
        heart_fft_rate_over_window_curve = heart_fft_rate_over_window_plot.plot()
        hf_row += 1

    if args.plot_variance_of_peak_rate_in_window:
        heart_variance_of_peak_rate_in_window_queue = collections.deque(maxlen=MAXLEN)
        heart_variance_of_peak_rate_in_window_plot = win.addPlot(title="heart_variance_of_peak_rate_in_window", row=hf_row, col=1)
        heart_variance_of_peak_rate_in_window_curve = heart_variance_of_peak_rate_in_window_plot.plot()
        hf_row += 1

    if args.plot_variance_of_valley_rate_in_window:
        heart_variance_of_valley_rate_in_window_queue = collections.deque(maxlen=MAXLEN)
        heart_variance_of_valley_rate_in_window_plot = win.addPlot(title="heart_variance_of_valley_rate_in_window", row=hf_row, col=1)
        heart_variance_of_valley_rate_in_window_curve = heart_variance_of_valley_rate_in_window_plot.plot()
        hf_row += 1

    if args.plot_mean_up_stroke_length:
        heart_mean_up_stroke_length_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_up_stroke_length_plot = win.addPlot(title="heart_mean_up_stroke_length", row=hf_row, col=1)
        heart_mean_up_stroke_length_curve = heart_mean_up_stroke_length_plot.plot()
        hf_row += 1

    if args.plot_mean_down_stroke_length:
        heart_mean_down_stroke_length_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_down_stroke_length_plot = win.addPlot(title="heart_mean_down_stroke_length", row=hf_row, col=1)
        heart_mean_down_stroke_length_curve = heart_mean_down_stroke_length_plot.plot()
        hf_row += 1

    if args.plot_up_stroke_length_variance:
        heart_up_stroke_length_variance_queue = collections.deque(maxlen=MAXLEN)
        heart_up_stroke_length_variance_plot = win.addPlot(title="heart_up_stroke_length_variance", row=hf_row, col=1)
        heart_up_stroke_length_variance_curve = heart_up_stroke_length_variance_plot.plot()
        hf_row += 1

    if args.plot_down_stroke_length_variance:
        heart_down_stroke_length_variance_queue = collections.deque(maxlen=MAXLEN)
        heart_down_stroke_length_variance_plot = win.addPlot(title="heart_down_stroke_length_variance", row=hf_row, col=1)
        heart_down_stroke_length_variance_curve = heart_down_stroke_length_variance_plot.plot()
        hf_row += 1

    if args.plot_up_to_down_length_ratio:
        heart_up_to_down_length_ratio_queue = collections.deque(maxlen=MAXLEN)
        heart_up_to_down_length_ratio_plot = win.addPlot(title="heart_up_to_down_length_ratio", row=hf_row, col=1)
        heart_up_to_down_length_ratio_curve = heart_up_to_down_length_ratio_plot.plot()
        hf_row += 1

    if args.plot_fractional_up_stroke_time:
        heart_fractional_up_stroke_time_queue = collections.deque(maxlen=MAXLEN)
        heart_fractional_up_stroke_time_plot = win.addPlot(title="heart_fractional_up_stroke_time", row=hf_row, col=1)
        heart_fractional_up_stroke_time_curve = heart_fractional_up_stroke_time_plot.plot()
        hf_row += 1

    if args.plot_mean_up_stroke_amplitude:
        heart_mean_up_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_up_stroke_amplitude_plot = win.addPlot(title="heart_mean_up_stroke_amplitude", row=hf_row, col=1)
        heart_mean_up_stroke_amplitude_curve = heart_mean_up_stroke_amplitude_plot.plot()
        hf_row += 1

    if args.plot_mean_down_stroke_amplitude:
        heart_mean_down_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        heart_mean_down_stroke_amplitude_plot = win.addPlot(title="heart_mean_down_stroke_amplitude", row=hf_row, col=1)
        heart_mean_down_stroke_amplitude_curve = heart_mean_down_stroke_amplitude_plot.plot()
        hf_row += 1

    if args.plot_up_stroke_amplitude_variance:
        heart_up_stroke_amplitude_variance_queue = collections.deque(maxlen=MAXLEN)
        heart_up_stroke_amplitude_variance_plot = win.addPlot(title="heart_up_stroke_amplitude_variance", row=hf_row, col=1)
        heart_up_stroke_amplitude_variance_curve = heart_up_stroke_amplitude_variance_plot.plot()
        hf_row += 1

    if args.plot_down_stroke_amplitude_variance:
        heart_down_stroke_amplitude_variance_queue = collections.deque(maxlen=MAXLEN)
        heart_down_stroke_amplitude_variance_plot = win.addPlot(title="heart_down_stroke_amplitude_variance", row=hf_row, col=1)
        heart_down_stroke_amplitude_variance_curve = heart_down_stroke_amplitude_variance_plot.plot()
        hf_row += 1

    if args.plot_up_to_down_amplitude_ratio:
        heart_up_to_down_amplitude_ratio_queue = collections.deque(maxlen=MAXLEN)
        heart_up_to_down_amplitude_ratio_plot = win.addPlot(title="heart_up_to_down_amplitude_ratio", row=hf_row, col=1)
        heart_up_to_down_amplitude_ratio_curve = heart_up_to_down_amplitude_ratio_plot.plot()
        hf_row += 1

    if args.plot_fractional_up_stroke_amplitude:
        heart_fractional_up_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        heart_fractional_up_stroke_amplitude_plot = win.addPlot(title="heart_fractional_up_stroke_amplitude", row=hf_row, col=1)
        heart_fractional_up_stroke_amplitude_curve = heart_fractional_up_stroke_amplitude_plot.plot()
        hf_row += 1

if kconfig.syms[option_names["breath_features"]+option_suffix].user_value == 2 and args.plot_breath_features:
    bf_row = 0
    if args.plot_instantaneous_peak_rate:
        breath_instantaneous_peak_rate_queue = collections.deque(maxlen=MAXLEN)
        breath_instantaneous_peak_rate_plot = win.addPlot(title="breath_instantaneous_peak_rate", row=bf_row, col=2)
        breath_instantaneous_peak_rate_curve = breath_instantaneous_peak_rate_plot.plot()
        bf_row += 1

    if args.plot_instantaneous_valley_rate:
        breath_instantaneous_valley_rate_queue = collections.deque(maxlen=MAXLEN)
        breath_instantaneous_valley_rate_plot = win.addPlot(title="breath_instantaneous_valley_rate", row=bf_row, col=2)
        breath_instantaneous_valley_rate_curve = breath_instantaneous_valley_rate_plot.plot()
        bf_row += 1

    if args.plot_mean_peak_rate_over_window:
        breath_mean_peak_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_peak_rate_over_window_plot = win.addPlot(title="breath_mean_peak_rate_over_window", row=bf_row, col=2)
        breath_mean_peak_rate_over_window_curve = breath_mean_peak_rate_over_window_plot.plot()
        bf_row += 1

    if args.plot_mean_valley_rate_over_window:
        breath_mean_valley_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_valley_rate_over_window_plot = win.addPlot(title="breath_mean_valley_rate_over_window", row=bf_row, col=2)
        breath_mean_valley_rate_over_window_curve = breath_mean_valley_rate_over_window_plot.plot()
        bf_row += 1

    if args.plot_fft_rate_over_window:
        breath_fft_rate_over_window_queue = collections.deque(maxlen=MAXLEN)
        breath_fft_rate_over_window_plot = win.addPlot(title="breath_fft_rate_over_window", row=bf_row, col=2)
        breath_fft_rate_over_window_curve = breath_fft_rate_over_window_plot.plot()
        bf_row += 1

    if args.plot_variance_of_peak_rate_in_window:
        breath_variance_of_peak_rate_in_window_queue = collections.deque(maxlen=MAXLEN)
        breath_variance_of_peak_rate_in_window_plot = win.addPlot(title="breath_variance_of_peak_rate_in_window", row=bf_row, col=2)
        breath_variance_of_peak_rate_in_window_curve = breath_variance_of_peak_rate_in_window_plot.plot()
        bf_row += 1

    if args.plot_variance_of_valley_rate_in_window:
        breath_variance_of_valley_rate_in_window_queue = collections.deque(maxlen=MAXLEN)
        breath_variance_of_valley_rate_in_window_plot = win.addPlot(title="breath_variance_of_valley_rate_in_window", row=bf_row, col=2)
        breath_variance_of_valley_rate_in_window_curve = breath_variance_of_valley_rate_in_window_plot.plot()
        bf_row += 1

    if args.plot_mean_up_stroke_length:
        breath_mean_up_stroke_length_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_up_stroke_length_plot = win.addPlot(title="breath_mean_up_stroke_length", row=bf_row, col=2)
        breath_mean_up_stroke_length_curve = breath_mean_up_stroke_length_plot.plot()
        bf_row += 1

    if args.plot_mean_down_stroke_length:
        breath_mean_down_stroke_length_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_down_stroke_length_plot = win.addPlot(title="breath_mean_down_stroke_length", row=bf_row, col=2)
        breath_mean_down_stroke_length_curve = breath_mean_down_stroke_length_plot.plot()
        bf_row += 1

    if args.plot_up_stroke_length_variance:
        breath_up_stroke_length_variance_queue = collections.deque(maxlen=MAXLEN)
        breath_up_stroke_length_variance_plot = win.addPlot(title="breath_up_stroke_length_variance", row=bf_row, col=2)
        breath_up_stroke_length_variance_curve = breath_up_stroke_length_variance_plot.plot()
        bf_row += 1

    if args.plot_down_stroke_length_variance:
        breath_down_stroke_length_variance_queue = collections.deque(maxlen=MAXLEN)
        breath_down_stroke_length_variance_plot = win.addPlot(title="breath_down_stroke_length_variance", row=bf_row, col=2)
        breath_down_stroke_length_variance_curve = breath_down_stroke_length_variance_plot.plot()
        bf_row += 1

    if args.plot_up_to_down_length_ratio:
        breath_up_to_down_length_ratio_queue = collections.deque(maxlen=MAXLEN)
        breath_up_to_down_length_ratio_plot = win.addPlot(title="breath_up_to_down_length_ratio", row=bf_row, col=2)
        breath_up_to_down_length_ratio_curve = breath_up_to_down_length_ratio_plot.plot()
        bf_row += 1

    if args.plot_fractional_up_stroke_time:
        breath_fractional_up_stroke_time_queue = collections.deque(maxlen=MAXLEN)
        breath_fractional_up_stroke_time_plot = win.addPlot(title="breath_fractional_up_stroke_time", row=bf_row, col=2)
        breath_fractional_up_stroke_time_curve = breath_fractional_up_stroke_time_plot.plot()
        bf_row += 1

    if args.plot_mean_up_stroke_amplitude:
        breath_mean_up_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_up_stroke_amplitude_plot = win.addPlot(title="breath_mean_up_stroke_amplitude", row=bf_row, col=2)
        breath_mean_up_stroke_amplitude_curve = breath_mean_up_stroke_amplitude_plot.plot()
        bf_row += 1

    if args.plot_mean_down_stroke_amplitude:
        breath_mean_down_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        breath_mean_down_stroke_amplitude_plot = win.addPlot(title="breath_mean_down_stroke_amplitude", row=bf_row, col=2)
        breath_mean_down_stroke_amplitude_curve = breath_mean_down_stroke_amplitude_plot.plot()
        bf_row += 1

    if args.plot_up_stroke_amplitude_variance:
        breath_up_stroke_amplitude_variance_queue = collections.deque(maxlen=MAXLEN)
        breath_up_stroke_amplitude_variance_plot = win.addPlot(title="breath_up_stroke_amplitude_variance", row=bf_row, col=2)
        breath_up_stroke_amplitude_variance_curve = breath_up_stroke_amplitude_variance_plot.plot()
        bf_row += 1

    if args.plot_down_stroke_amplitude_variance:
        breath_down_stroke_amplitude_variance_queue = collections.deque(maxlen=MAXLEN)
        breath_down_stroke_amplitude_variance_plot = win.addPlot(title="breath_down_stroke_amplitude_variance", row=bf_row, col=2)
        breath_down_stroke_amplitude_variance_curve = breath_down_stroke_amplitude_variance_plot.plot()
        bf_row += 1

    if args.plot_up_to_down_amplitude_ratio:
        breath_up_to_down_amplitude_ratio_queue = collections.deque(maxlen=MAXLEN)
        breath_up_to_down_amplitude_ratio_plot = win.addPlot(title="breath_up_to_down_amplitude_ratio", row=bf_row, col=2)
        breath_up_to_down_amplitude_ratio_curve = breath_up_to_down_amplitude_ratio_plot.plot()
        bf_row += 1

    if args.plot_fractional_up_stroke_amplitude:
        breath_fractional_up_stroke_amplitude_queue = collections.deque(maxlen=MAXLEN)
        breath_fractional_up_stroke_amplitude_plot = win.addPlot(title="breath_fractional_up_stroke_amplitude", row=bf_row, col=2)
        breath_fractional_up_stroke_amplitude_curve = breath_fractional_up_stroke_amplitude_plot.plot()
        bf_row += 1


# pois are detected on the filtered signal and require the signal to be plotted to plot them in top ot the signal
if kconfig.syms[option_names["heart_poi"] + option_suffix].user_value == 2 and args.plot_heart_pois and \
        kconfig.syms[option_names["filtered_heart"] + option_suffix].user_value == 2 and args.plot_filtered_heart:
    heart_peaks_queue = collections.deque(maxlen=MAXLEN)
    heart_valleys_queue = collections.deque(maxlen=MAXLEN)
    heart_peak_helper = collections.deque(maxlen=MAXLEN)
    heart_valley_helper = collections.deque(maxlen=MAXLEN)
    heart_peaks_curve = filtered_heart_plot.plot(pen=None, symbol='x', symbolBrush=(0, 0, 255))
    heart_valleys_curve = filtered_heart_plot.plot(pen=None, symbol='o', symbolBrush=(255, 255, 0))

if kconfig.syms[option_names["breath_poi"] + option_suffix].user_value == 2 and args.plot_breath_pois and \
        kconfig.syms[option_names["filtered_breath"] + option_suffix].user_value == 2 and args.plot_filtered_breath:
    breath_peaks_queue = collections.deque(maxlen=MAXLEN)
    breath_valleys_queue = collections.deque(maxlen=MAXLEN)
    breath_peak_helper = collections.deque(maxlen=MAXLEN)
    breath_valley_helper = collections.deque(maxlen=MAXLEN)
    breath_peaks_curve = filtered_breath_plot.plot(pen=None, symbol='x', symbolBrush=(0, 0, 255))
    breath_valleys_curve = filtered_breath_plot.plot(pen=None, symbol='o', symbolBrush=(255, 255, 0))


if kconfig.syms[option_names["detection"]+option_suffix].user_value == 2 and args.plot_detected_presence:
    detected_presence_queue = collections.deque(maxlen=MAXLEN)
    detected_presence_plot = win.addPlot(title="detected presence", row=row, col=0)
    detected_presence_curve = detected_presence_plot.plot()
    row += 1

if kconfig.syms[option_names["detection"] + option_suffix].user_value == 2 and args.plot_detected_small_movement:
    detected_small_movement_queue = collections.deque(maxlen=MAXLEN)
    detected_small_movement_plot = win.addPlot(title="detected small movement", row=row, col=0)
    detected_small_movement_curve = detected_small_movement_plot.plot()
    row += 1

if kconfig.syms[option_names["detection"] + option_suffix].user_value == 2 and args.plot_detected_large_movement:
    detected_large_movement_queue = collections.deque(maxlen=MAXLEN)
    detected_large_movement_plot = win.addPlot(title="detected large movement", row=row, col=0)
    detected_large_movement_curve = detected_large_movement_plot.plot()
    row += 1



def update_plot():
    global current_time  # current time is set to 0 when the plotting starts and ever increases
    # read line -> depending on config either from serial or from udp broadcast
    line = ""

    if read_from_serial:
        # read from serial
        try:
            sys.stdin.buffer.flush()
            line = sys.stdin.buffer.readline().decode('utf-8').replace("\n", "")
        except:
            pass  # might not be an utf-8 string!
    if read_from_udp:
        # read from udp broadcast
        try:
            data = udp_socket.recv(1024)
            line = data.decode("utf-8").replace("\n", "")
        except:
            pass  # might not be an utf-8 string!

    values = line.split(",")

    # all data lines start with CSI_DATA
    if len(values)>0 and values[0] == "CSI_DATA":
        reduce_index_by = 0

        time_queue.append(current_time)
        time_array = np.array(time_queue)

        if kconfig.syms[option_names["sti"] + option_suffix].user_value == 2 and args.plot_sti:
            print("values", values)
            print("sti value", values[option_indices["sti"]])
            sti = float(values[option_indices["sti"]])
            sti_queue.append(sti)
            sti_curve.setData(time_array, np.array(sti_queue))

        if kconfig.syms[option_names["sti"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["fused_heart"] + option_suffix].user_value == 2 and args.plot_fused_heart:
            fused_heart = float(values[option_indices["fused_heart"] - reduce_index_by])
            fused_heart_queue.append(fused_heart)
            fused_heart_curve.setData(time_array, np.array(fused_heart_queue))

        if kconfig.syms[option_names["fused_heart"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["fused_breath"] + option_suffix].user_value == 2 and args.plot_fused_breath:
            fused_breath = float(values[option_indices["fused_breath"] - reduce_index_by])
            fused_breath_queue.append(fused_breath)
            fused_breath_curve.setData(time_array, np.array(fused_breath_queue))

        if kconfig.syms[option_names["fused_breath"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["filtered_heart"] + option_suffix].user_value == 2 and args.plot_filtered_heart:
            filtered_heart = float(values[option_indices["filtered_heart"] - reduce_index_by])
            filtered_heart_queue.append(filtered_heart)
            filtered_heart_curve.setData(time_array, np.array(filtered_heart_queue))

        if kconfig.syms[option_names["filtered_heart"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["filtered_breath"] + option_suffix].user_value == 2 and args.plot_filtered_breath:
            filtered_breath = float(values[option_indices["filtered_breath"] - reduce_index_by])
            filtered_breath_queue.append(filtered_breath)
            filtered_breath_curve.setData(time_array, np.array(filtered_breath_queue))

        if kconfig.syms[option_names["filtered_breath"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["heart_features"] + option_suffix].user_value == 2 and args.plot_heart_features:
            if args.plot_instantaneous_peak_rate:
                heart_instantaneous_peak_rate = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[0])
                heart_instantaneous_peak_rate_queue.append(heart_instantaneous_peak_rate)
                heart_instantaneous_peak_rate_curve.setData(time_array, np.array(heart_instantaneous_peak_rate_queue))

            if args.plot_instantaneous_valley_rate:
                heart_instantaneous_valley_rate = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[1])
                heart_instantaneous_valley_rate_queue.append(heart_instantaneous_valley_rate)
                heart_instantaneous_valley_rate_curve.setData(time_array, np.array(heart_instantaneous_valley_rate_queue))

            if args.plot_mean_peak_rate_over_window:
                heart_mean_peak_rate_over_window = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[2])
                heart_mean_peak_rate_over_window_queue.append(heart_mean_peak_rate_over_window)
                heart_mean_peak_rate_over_window_curve.setData(time_array, np.array(heart_mean_peak_rate_over_window_queue))

            if args.plot_mean_valley_rate_over_window:
                heart_mean_valley_rate_over_window = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[3])
                heart_mean_valley_rate_over_window_queue.append(heart_mean_valley_rate_over_window)
                heart_mean_valley_rate_over_window_curve.setData(time_array, np.array(heart_mean_valley_rate_over_window_queue))

            if args.plot_fft_rate_over_window:
                heart_fft_rate_over_window = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[4])
                heart_fft_rate_over_window_queue.append(heart_fft_rate_over_window)
                heart_fft_rate_over_window_curve.setData(time_array, np.array(heart_fft_rate_over_window_queue))

            if args.plot_variance_of_peak_rate_in_window:
                heart_variance_of_peak_rate_in_window = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[5])
                heart_variance_of_peak_rate_in_window_queue.append(heart_variance_of_peak_rate_in_window)
                heart_variance_of_peak_rate_in_window_curve.setData(time_array, np.array(heart_variance_of_peak_rate_in_window_queue))

            if args.plot_variance_of_valley_rate_in_window:
                heart_variance_of_valley_rate_in_window = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[6])
                heart_variance_of_valley_rate_in_window_queue.append(heart_variance_of_valley_rate_in_window)
                heart_variance_of_valley_rate_in_window_curve.setData(time_array, np.array(heart_variance_of_valley_rate_in_window_queue))

            if args.plot_mean_up_stroke_length:
                heart_mean_up_stroke_length = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[7])
                heart_mean_up_stroke_length_queue.append(heart_mean_up_stroke_length)
                heart_mean_up_stroke_length_curve.setData(time_array, np.array(heart_mean_up_stroke_length_queue))

            if args.plot_mean_down_stroke_length:
                heart_mean_down_stroke_length = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[8])
                heart_mean_down_stroke_length_queue.append(heart_mean_down_stroke_length)
                heart_mean_down_stroke_length_curve.setData(time_array, np.array(heart_mean_down_stroke_length_queue))

            if args.plot_up_stroke_length_variance:
                heart_up_stroke_length_variance = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[9])
                heart_up_stroke_length_variance_queue.append(heart_up_stroke_length_variance)
                heart_up_stroke_length_variance_curve.setData(time_array, np.array(heart_up_stroke_length_variance_queue))

            if args.plot_down_stroke_length_variance:
                heart_down_stroke_length_variance = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[10])
                heart_down_stroke_length_variance_queue.append(heart_down_stroke_length_variance)
                heart_down_stroke_length_variance_curve.setData(time_array, np.array(heart_down_stroke_length_variance_queue))

            if args.plot_up_to_down_length_ratio:
                heart_up_to_down_length_ratio = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[11])
                heart_up_to_down_length_ratio_queue.append(heart_up_to_down_length_ratio)
                heart_up_to_down_length_ratio_curve.setData(time_array, np.array(heart_up_to_down_length_ratio_queue))

            if args.plot_fractional_up_stroke_time:
                heart_fractional_up_stroke_time = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[12])
                heart_fractional_up_stroke_time_queue.append(heart_fractional_up_stroke_time)
                heart_fractional_up_stroke_time_curve.setData(time_array, np.array(heart_fractional_up_stroke_time_queue))

            if args.plot_mean_up_stroke_amplitude:
                heart_mean_up_stroke_amplitude = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[13])
                heart_mean_up_stroke_amplitude_queue.append(heart_mean_up_stroke_amplitude)
                heart_mean_up_stroke_amplitude_curve.setData(time_array, np.array(heart_mean_up_stroke_amplitude_queue))

            if args.plot_mean_down_stroke_amplitude:
                heart_mean_down_stroke_amplitude = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[14])
                heart_mean_down_stroke_amplitude_queue.append(heart_mean_down_stroke_amplitude)
                heart_mean_down_stroke_amplitude_curve.setData(time_array, np.array(heart_mean_down_stroke_amplitude_queue))

            if args.plot_up_stroke_amplitude_variance:
                heart_up_stroke_amplitude_variance = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[15])
                heart_up_stroke_amplitude_variance_queue.append(heart_up_stroke_amplitude_variance)
                heart_up_stroke_amplitude_variance_curve.setData(time_array, np.array(heart_up_stroke_amplitude_variance_queue))

            if args.plot_down_stroke_amplitude_variance:
                heart_down_stroke_amplitude_variance = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[16])
                heart_down_stroke_amplitude_variance_queue.append(heart_down_stroke_amplitude_variance)
                heart_down_stroke_amplitude_variance_curve.setData(time_array, np.array(heart_down_stroke_amplitude_variance_queue))

            if args.plot_up_to_down_amplitude_ratio:
                heart_up_to_down_amplitude_ratio = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[17])
                heart_up_to_down_amplitude_ratio_queue.append(heart_up_to_down_amplitude_ratio)
                heart_up_to_down_amplitude_ratio_curve.setData(time_array, np.array(heart_up_to_down_amplitude_ratio_queue))

            if args.plot_fractional_up_stroke_amplitude:
                heart_fractional_up_stroke_amplitude = float(values[option_indices["heart_features"] - reduce_index_by][1:-1].split(" ")[18])
                heart_fractional_up_stroke_amplitude_queue.append(heart_fractional_up_stroke_amplitude)
                heart_fractional_up_stroke_amplitude_curve.setData(time_array, np.array(heart_fractional_up_stroke_amplitude_queue))

        if kconfig.syms[option_names["heart_features"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["breath_features"] + option_suffix].user_value == 2 and args.plot_breath_features:
            if args.plot_instantaneous_peak_rate:
                breath_instantaneous_peak_rate = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[0])
                breath_instantaneous_peak_rate_queue.append(breath_instantaneous_peak_rate)
                breath_instantaneous_peak_rate_curve.setData(time_array, np.array(breath_instantaneous_peak_rate_queue))

            if args.plot_instantaneous_valley_rate:
                breath_instantaneous_valley_rate = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[1])
                breath_instantaneous_valley_rate_queue.append(breath_instantaneous_valley_rate)
                breath_instantaneous_valley_rate_curve.setData(time_array, np.array(breath_instantaneous_valley_rate_queue))

            if args.plot_mean_peak_rate_over_window:
                breath_mean_peak_rate_over_window = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[2])
                breath_mean_peak_rate_over_window_queue.append(breath_mean_peak_rate_over_window)
                breath_mean_peak_rate_over_window_curve.setData(time_array, np.array(breath_mean_peak_rate_over_window_queue))

            if args.plot_mean_valley_rate_over_window:
                breath_mean_valley_rate_over_window = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[3])
                breath_mean_valley_rate_over_window_queue.append(breath_mean_valley_rate_over_window)
                breath_mean_valley_rate_over_window_curve.setData(time_array, np.array(breath_mean_valley_rate_over_window_queue))

            if args.plot_fft_rate_over_window:
                breath_fft_rate_over_window = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[4])
                breath_fft_rate_over_window_queue.append(breath_fft_rate_over_window)
                breath_fft_rate_over_window_curve.setData(time_array, np.array(breath_fft_rate_over_window_queue))

            if args.plot_variance_of_peak_rate_in_window:
                breath_variance_of_peak_rate_in_window = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[5])
                breath_variance_of_peak_rate_in_window_queue.append(breath_variance_of_peak_rate_in_window)
                breath_variance_of_peak_rate_in_window_curve.setData(time_array, np.array(breath_variance_of_peak_rate_in_window_queue))

            if args.plot_variance_of_valley_rate_in_window:
                breath_variance_of_valley_rate_in_window = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[6])
                breath_variance_of_valley_rate_in_window_queue.append(breath_variance_of_valley_rate_in_window)
                breath_variance_of_valley_rate_in_window_curve.setData(time_array, np.array(breath_variance_of_valley_rate_in_window_queue))

            if args.plot_mean_up_stroke_length:
                breath_mean_up_stroke_length = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[7])
                breath_mean_up_stroke_length_queue.append(breath_mean_up_stroke_length)
                breath_mean_up_stroke_length_curve.setData(time_array, np.array(breath_mean_up_stroke_length_queue))

            if args.plot_mean_down_stroke_length:
                breath_mean_down_stroke_length = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[8])
                breath_mean_down_stroke_length_queue.append(breath_mean_down_stroke_length)
                breath_mean_down_stroke_length_curve.setData(time_array, np.array(breath_mean_down_stroke_length_queue))

            if args.plot_up_stroke_length_variance:
                breath_up_stroke_length_variance = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[9])
                breath_up_stroke_length_variance_queue.append(breath_up_stroke_length_variance)
                breath_up_stroke_length_variance_curve.setData(time_array, np.array(breath_up_stroke_length_variance_queue))

            if args.plot_down_stroke_length_variance:
                breath_down_stroke_length_variance = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[10])
                breath_down_stroke_length_variance_queue.append(breath_down_stroke_length_variance)
                breath_down_stroke_length_variance_curve.setData(time_array, np.array(breath_down_stroke_length_variance_queue))

            if args.plot_up_to_down_length_ratio:
                breath_up_to_down_length_ratio = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[11])
                breath_up_to_down_length_ratio_queue.append(breath_up_to_down_length_ratio)
                breath_up_to_down_length_ratio_curve.setData(time_array, np.array(breath_up_to_down_length_ratio_queue))

            if args.plot_fractional_up_stroke_time:
                breath_fractional_up_stroke_time = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[12])
                breath_fractional_up_stroke_time_queue.append(breath_fractional_up_stroke_time)
                breath_fractional_up_stroke_time_curve.setData(time_array, np.array(breath_fractional_up_stroke_time_queue))

            if args.plot_mean_up_stroke_amplitude:
                breath_mean_up_stroke_amplitude = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[13])
                breath_mean_up_stroke_amplitude_queue.append(breath_mean_up_stroke_amplitude)
                breath_mean_up_stroke_amplitude_curve.setData(time_array, np.array(breath_mean_up_stroke_amplitude_queue))

            if args.plot_mean_down_stroke_amplitude:
                breath_mean_down_stroke_amplitude = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[14])
                breath_mean_down_stroke_amplitude_queue.append(breath_mean_down_stroke_amplitude)
                breath_mean_down_stroke_amplitude_curve.setData(time_array, np.array(breath_mean_down_stroke_amplitude_queue))

            if args.plot_up_stroke_amplitude_variance:
                breath_up_stroke_amplitude_variance = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[15])
                breath_up_stroke_amplitude_variance_queue.append(breath_up_stroke_amplitude_variance)
                breath_up_stroke_amplitude_variance_curve.setData(time_array, np.array(breath_up_stroke_amplitude_variance_queue))

            if args.plot_down_stroke_amplitude_variance:
                breath_down_stroke_amplitude_variance = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[16])
                breath_down_stroke_amplitude_variance_queue.append(breath_down_stroke_amplitude_variance)
                breath_down_stroke_amplitude_variance_curve.setData(time_array, np.array(breath_down_stroke_amplitude_variance_queue))

            if args.plot_up_to_down_amplitude_ratio:
                breath_up_to_down_amplitude_ratio = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[17])
                breath_up_to_down_amplitude_ratio_queue.append(breath_up_to_down_amplitude_ratio)
                breath_up_to_down_amplitude_ratio_curve.setData(time_array, np.array(breath_up_to_down_amplitude_ratio_queue))

            if args.plot_fractional_up_stroke_amplitude:
                breath_fractional_up_stroke_amplitude = float(values[option_indices["breath_features"] - reduce_index_by][1:-1].split(" ")[18])
                breath_fractional_up_stroke_amplitude_queue.append(breath_fractional_up_stroke_amplitude)
                breath_fractional_up_stroke_amplitude_curve.setData(time_array, np.array(breath_fractional_up_stroke_amplitude_queue))

        if kconfig.syms[option_names["breath_features"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        if kconfig.syms[option_names["heart_poi"] + option_suffix].user_value == 2 and args.plot_heart_pois and \
                kconfig.syms[option_names["filtered_heart"] + option_suffix].user_value == 2 and \
                args.plot_filtered_heart:
            found_poi = int(values[option_indices["heart_poi_found"] - reduce_index_by])
            heart_peak_helper.append([current_time, float(filtered_heart)])
            heart_valley_helper.append([current_time, float(filtered_heart)])

            try:
                # check whether some plotted pois fall out of the time window to plot
                if current_time - heart_peaks_queue[0][0] >= MAXLEN-2:
                    heart_peaks_queue.popleft()
                    heart_peaks_curve.setData(np.array(heart_peaks_queue)[:, 0], np.array(heart_peaks_queue)[:, 1])
            except IndexError:
                pass

            try:
                if current_time - heart_valleys_queue[0][0] >= MAXLEN-2:
                    heart_valleys_queue.popleft()
                    heart_valleys_curve.setData(np.array(heart_valleys_queue)[:, 0],
                                                 np.array(heart_valleys_queue)[:, 1])
            except IndexError:
                pass

            if found_poi == 1:
                is_peak = int(values[option_indices["new_heart_poi"] - reduce_index_by][1:-1].split(" ")[0])
                if is_peak == 1:
                    try:
                        t = heart_peak_helper[int(values[option_indices["new_heart_poi"]
                                                     - reduce_index_by][1:-1].split(" ")[-1]) - 1]
                        heart_peaks_queue.append(t)
                        heart_peaks_curve.setData(np.array(heart_peaks_queue)[:, 0], np.array(heart_peaks_queue)[:, 1])
                        # clear the valley helper, because the next thing will be a valley, so it has to be x indices after this point
                        heart_valley_helper.clear()
                    except IndexError: ## can happen due to dropped packets when using UDP
                        pass
                else:
                    try:
                        heart_valleys_queue.append(heart_valley_helper[int(values[option_indices["new_heart_poi"]
                                                                                  - reduce_index_by][1:-1].split(" ")[-1]) - 1])
                        heart_valleys_curve.setData(np.array(heart_valleys_queue)[:, 0],
                                                    np.array(heart_valleys_queue)[:, 1])
                        # clear the peak helper, because the next thing will be a peak, so it has to be x indices after this point
                        heart_peak_helper.clear()
                    except IndexError: ## can happen due to dropped packets when using UDP
                        pass

        if kconfig.syms[option_names["heart_poi"] + option_suffix].user_value != 2:
            reduce_index_by += 2

        if kconfig.syms[option_names["breath_poi"] + option_suffix].user_value == 2 and args.plot_breath_pois and \
                kconfig.syms[option_names["filtered_breath"] + option_suffix].user_value == 2 and \
                args.plot_filtered_breath:
            found_poi = int(values[option_indices["breath_poi_found"] - reduce_index_by])
            breath_peak_helper.append([current_time, float(filtered_breath)])
            breath_valley_helper.append([current_time, float(filtered_breath)])

            try:
                # check whether some plotted pois fall out of the time window to plot
                if current_time - breath_peaks_queue[0][0] >= MAXLEN-2:
                    breath_peaks_queue.popleft()
                    breath_peaks_curve.setData(np.array(breath_peaks_queue)[:, 0], np.array(breath_peaks_queue)[:, 1])
            except IndexError:
                pass

            try:
                if current_time - breath_valleys_queue[0][0] >= MAXLEN-2:
                    breath_valleys_queue.popleft()
                    breath_valleys_curve.setData(np.array(breath_valleys_queue)[:, 0],
                                                 np.array(breath_valleys_queue)[:, 1])
            except IndexError:
                pass


            if found_poi == 1:
                is_peak = int(values[option_indices["new_breath_poi"] - reduce_index_by][1:-1].split(" ")[0])
                if is_peak == 1:
                    print("ndex", int(values[option_indices["new_breath_poi"]
                                                     - reduce_index_by][1:-1].split(" ")[-1]) - 1)
                    print("new breath poi", values[option_indices["new_breath_poi"]-reduce_index_by])
                    try:
                        t = breath_peak_helper[int(values[option_indices["new_breath_poi"]
                                                         - reduce_index_by][1:-1].split(" ")[-1]) - 1]
                        breath_peaks_queue.append(t)
                        breath_peaks_curve.setData(np.array(breath_peaks_queue)[:, 0], np.array(breath_peaks_queue)[:, 1])
                        breath_valley_helper.clear()
                    except IndexError: ## can happen due to dropped packets when using UDP
                        pass
                else:
                    try:
                        breath_valleys_queue.append(breath_valley_helper[int(values[option_indices["new_breath_poi"]
                                                                                  - reduce_index_by][1:-1].split(" ")[
                                                                               -1]) - 1])
                        breath_valleys_curve.setData(np.array(breath_valleys_queue)[:, 0],
                                                    np.array(breath_valleys_queue)[:, 1])
                        breath_peak_helper.clear()
                    except IndexError: ## can happen due to dropped packets when using UDP
                        pass

        if kconfig.syms[option_names["breath_poi"] + option_suffix].user_value != 2:
            reduce_index_by += 2

        if kconfig.syms[option_names["detection"] + option_suffix].user_value == 2 and args.plot_detected_presence:
            detected_presence = int(values[option_indices["detected_presence"] - reduce_index_by])
            detected_presence_queue.append(detected_presence)
            detected_presence_curve.setData(time_array, np.array(detected_presence_queue))

        if kconfig.syms[
            option_names["detection"] + option_suffix].user_value == 2 and args.plot_detected_small_movement:
            detected_small_movement = int(values[option_indices["detected_small_movement"] - reduce_index_by])
            detected_small_movement_queue.append(detected_small_movement)
            detected_small_movement_curve.setData(time_array, np.array(detected_small_movement_queue))

        if kconfig.syms[
            option_names["detection"] + option_suffix].user_value == 2 and args.plot_detected_large_movement:
            detected_large_movement = int(values[option_indices["detected_large_movement"] - reduce_index_by])
            detected_large_movement_queue.append(detected_large_movement)
            detected_large_movement_curve.setData(time_array, np.array(detected_large_movement_queue))

        if kconfig.syms[option_names["detection"] + option_suffix].user_value != 2:
            reduce_index_by += 3

        if kconfig.syms[option_names["thresholds"] + option_suffix].user_value != 2:
            reduce_index_by += 6

        if kconfig.syms[option_names["amplitudes"] + option_suffix].user_value == 2 and args.plot_amplitudes:
            amplitudes = []
            for value in values[option_indices["amplitudes"] - reduce_index_by][1:-1].split(" "):
                amplitudes.append(float(value))

            amplitudes_queue.append(amplitudes)
            amplitude = np.array(amplitudes_queue)
            for j, carrier_amplitudes in enumerate(np.transpose(amplitude)):
                if not j in [0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37]:
                    carrier_curves[j].setData(time_array, carrier_amplitudes)

        if kconfig.syms[option_names["amplitudes"] + option_suffix].user_value != 2:
            reduce_index_by += 1

        current_time += 1


# run the code
timer = QtCore.QTimer()
timer.timeout.connect(update_plot)
timer.start(1)
pg.exec()
