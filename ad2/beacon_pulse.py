import math 
import matplotlib.pyplot as plt 
import dwfpy as dwf
import time
import threading

print(f"DWF Version: {dwf.Application.get_version()}")

freq = 457e3

with dwf.Device() as device: 

    def gen_sig(event):
        while not event.is_set():
            # pulse on
            print("Pulsing on for 70 ms.")
            wavegen[0].setup("sine", frequency=freq, amplitude=0.1, offset=1.65, start=True)
            wavegen[1].setup("sine", frequency=freq, amplitude=1.65, offset=1.65, start=True)
            time.sleep(70e-3)
            # pulse off
            print("Pulsing off for 400 ms.")
            wavegen[0].setup("dc", offset=1.65, start=True)
            wavegen[1].setup("dc", offset=1.65, start=True)
            time.sleep(400e-3)

    print(f"Found device: {device.name} ({device.serial_number})") 

    scope = device.analog_input 
    wavegen = device.analog_output 

    # input("Connect waveform generator to oscilloscope:\n- W1 to 1+\n- GND to 1-\nPress Enter to continue...")

    stop_event = threading.Event()
    thread1 = threading.Thread(target=gen_sig, args=(stop_event,))
    thread1.start()

    # stop here
    while 1: 
        pass

    print("Starting oscilloscope...")
    scope[0].setup(range=2)
    # scope.setup_edge_trigger(mode="normal", channel=0, slope="rising", level=0, hysteresis=0.01)
    recorder = scope.record(sample_rate=1e6, length=1, configure=True, start=True)

    if recorder.lost_samples > 0:
        print("Samples lost, reduce sample rate.")
    if recorder.corrupted_samples > 0:
        print("Samples corrupted, reduce sample rate.")

    print(
        f"Processed {recorder.total_samples} samples total, "
        f"received {len(recorder.channels[0].data_samples)} samples."
    )

    channels = recorder.channels

    stop_event.set() 
    thread1.join()

for channel in channels:
    plt.plot(channel.data_samples, drawstyle="steps-post")

plt.show()
