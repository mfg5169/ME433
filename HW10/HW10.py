import os
import pandas as pd

# Path to the raw_data folder
raw_data_folder = os.path.join(os.path.dirname(__file__), 'raw_data')

# List all CSV files in the folder
csv_files = [f for f in os.listdir(raw_data_folder) if f.endswith('.csv')]

# Load each CSV file into a DataFrame
dataframes = {}
for csv_file in csv_files:
    file_path = os.path.join(raw_data_folder, csv_file)
    df = pd.read_csv(file_path)
    dataframes[csv_file] = df

# Example: print the first few rows of each DataFrame
for name, df in dataframes.items():
    print(f"--- {name} ---")
    print(df.head())


#  import matplotlib.pyplot as plt
# import numpy as np

# dt = 1.0/10000.0 # 10kHz
# t = np.arange(0.0, 1.0, dt) # 10s
# # a constant plus 100Hz and 1000Hz
# s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25

# Fs = 10000 # sample rate
# Ts = 1.0/Fs; # sampling interval
# ts = np.arange(0,t[-1],Ts) # time vector
# y = s # the data to make the fft from
# n = len(y) # length of the signal
# k = np.arange(n)
# T = n/Fs
# frq = k/T # two sides frequency range
# frq = frq[range(int(n/2))] # one side frequency range
# Y = np.fft.fft(y)/n # fft computing and normalization
# Y = Y[range(int(n/2))]

# fig, (ax1, ax2) = plt.subplots(2, 1)
# ax1.plot(t,y,'b')
# ax1.set_xlabel('Time')
# ax1.set_ylabel('Amplitude')
# ax2.loglog(frq,abs(Y),'b') # plotting the fft
# ax2.set_xlabel('Freq (Hz)')
# ax2.set_ylabel('|Y(freq)|')
# plt.show()