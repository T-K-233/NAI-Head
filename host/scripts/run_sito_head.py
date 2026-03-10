"""
this script runs on the low-level control computer that connects to the Sito actuators on the neck via CAN bus.
"""

from udpack import UDP

import numpy as np
from loop_rate_limiters import RateLimiter

from actuator_control.sito import SitoBus, Motor


udp = UDP(recv_addr=("0.0.0.0", 7000))


channel = "can0"
bitrate = 1000000


motors = {
    "neck_yaw": Motor(id=0x61, model="TA40-100"),
    "neck_roll": Motor(id=0x62, model="TA40-100"),
    "neck_pitch": Motor(id=0x63, model="TA40-100"),
}

offsets = np.array([0, -0.2, -0.3], dtype=np.float32)


# can go up to 1 kHz with 2 actuators
control_frequency = 50.0


rate = RateLimiter(frequency=control_frequency)
bus = SitoBus(channel=channel, motors=motors, control_frequency=control_frequency)
bus.connect()

bus.write_mit_kp_kd("neck_yaw", 5.0, 1.0)
bus.write_mit_kp_kd("neck_roll", 5.0, 1.0)
bus.write_mit_kp_kd("neck_pitch", 5.0, 1.0)

for name, motor in motors.items():
    bus.enable(name)


target_positions = np.array([0, 0, 0], dtype=np.float32)
measured_positions = np.array([0, 0, 0], dtype=np.float32)


try:
    while True:

        data = udp.recv_numpy(timeout=0.001)
        if data is not None:
            data = data * 0.75
            target_positions[0] = -data[1]  # face X -> yaw
            target_positions[1] = -data[3]  # face Z -> roll
            target_positions[2] = data[2]  # face Y -> pitch
            print("received target positions:", target_positions)

        target_positions = np.clip(target_positions, -0.3, 0.3)

        bus.write_mit_control(motor="neck_yaw", position=target_positions[0] + offsets[0], velocity=0, torque=0)
        bus.write_mit_control(motor="neck_roll", position=target_positions[1] + offsets[1], velocity=0, torque=0)
        bus.write_mit_control(motor="neck_pitch", position=target_positions[2] + offsets[2], velocity=0, torque=0)

        measured_positions[0], _ = bus.read_mit_state(motor="neck_yaw")
        measured_positions[1], _ = bus.read_mit_state(motor="neck_roll")
        measured_positions[2], _ = bus.read_mit_state(motor="neck_pitch")
        # print(f"position: {measured_positions[0]:.3f}, {measured_positions[1]:.3f}, {measured_positions[2]:.3f}")
        rate.sleep()

except KeyboardInterrupt:
    pass

for name in motors.keys():
    bus.disable(name)
bus.disconnect()

print("Program terminated.")
