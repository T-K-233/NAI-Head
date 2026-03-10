import asyncio
import socket
from typing import Dict

import numpy as np

from loop_rate_limiters import RateLimiter
from common import create_vts_client


class UDPMessageIndex:
    FACE_ANGLE_X = 1
    FACE_ANGLE_Y = 2
    FACE_ANGLE_Z = 3
    BROW_HEIGHT_LEFT = 4
    BROW_HEIGHT_RIGHT = 5
    EYE_OPEN_LEFT = 6
    EYE_OPEN_RIGHT = 7
    EYE_LEFT_X = 8
    EYE_LEFT_Y = 9
    EYE_RIGHT_X = 10
    EYE_RIGHT_Y = 11


NUM_MESSAGE_FIELDS = 12

# Device IP (unicast). Must match firmware, e.g. 172.28.0.64
# ADDR = "172.28.0.64"
# broadcast to all devices on the network
ADDR = "172.28.0.255"
PORT = 7000


def _clamp_to_unit(value: float) -> float:
    return max(-1.0, min(1.0, value))


def _normalize_angle_45_deg_range(value_deg: float) -> float:
    """Map angle in [-45, 45] degrees to [-1, 1]."""
    return _clamp_to_unit(value_deg / 45.0)


def _normalize_unit_0_1(value: float) -> float:
    """Map [0, 1] to [-1, 1]."""
    return _clamp_to_unit(2.0 * value - 1.0)


def _build_udp_message(params: Dict[str, float]) -> np.ndarray:
    """
    Build normalized UDP message from Live2D parameters.

    Expected keys in `params` (from Live2D):
      - "ParamAngleX", "ParamAngleY", "ParamAngleZ"
      - "ParamBrowLY", "ParamBrowRY"
      - "ParamEyeLOpen", "ParamEyeROpen"
      - "ParamEyeBallX", "ParamEyeBallY"
    """
    message = np.zeros(NUM_MESSAGE_FIELDS, dtype=np.float32)

    # Angles: -45 (left/down/tilt-left) to +45 (right/up/tilt-right)
    face_angle_x = params["ParamAngleX"]
    face_angle_y = params["ParamAngleY"]
    face_angle_z = params["ParamAngleZ"]

    message[UDPMessageIndex.FACE_ANGLE_X] = _normalize_angle_45_deg_range(face_angle_x)
    message[UDPMessageIndex.FACE_ANGLE_Y] = _normalize_angle_45_deg_range(face_angle_y)
    message[UDPMessageIndex.FACE_ANGLE_Z] = _normalize_angle_45_deg_range(face_angle_z)

    # Brows: 0 (down) to 1 (up) -> [-1, 1]
    brow_height_left = params["ParamBrowLY"]
    brow_height_right = params["ParamBrowRY"]

    message[UDPMessageIndex.BROW_HEIGHT_LEFT] = _normalize_unit_0_1(brow_height_left)
    message[UDPMessageIndex.BROW_HEIGHT_RIGHT] = _normalize_unit_0_1(brow_height_right)

    # Eye open: 0 (closed) to 1 (open) -> [-1, 1]
    eye_open_left = params["ParamEyeLOpen"]
    eye_open_right = params["ParamEyeROpen"]

    message[UDPMessageIndex.EYE_OPEN_LEFT] = _normalize_unit_0_1(eye_open_left)
    message[UDPMessageIndex.EYE_OPEN_RIGHT] = _normalize_unit_0_1(eye_open_right)

    # Eye ball positions: already in [-1, 1], just clamp
    eye_x = params["ParamEyeBallX"]
    eye_y = params["ParamEyeBallY"]

    # Use same underlying value for both eyes, as in test_get_live2d_parameters.py
    message[UDPMessageIndex.EYE_LEFT_X] = _clamp_to_unit(eye_x)
    message[UDPMessageIndex.EYE_LEFT_Y] = _clamp_to_unit(eye_y)
    message[UDPMessageIndex.EYE_RIGHT_X] = _clamp_to_unit(eye_x)
    message[UDPMessageIndex.EYE_RIGHT_Y] = _clamp_to_unit(eye_y)

    return message


async def main():
    vts = await create_vts_client()

    # Direct UDP socket (unicast)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    rate = RateLimiter(frequency=30.0)

    try:
        while True:
            response = await vts.request(
                vts.vts_request.BaseRequest("Live2DParameterListRequest")
            )

            parameters: Dict[str, float] = {}
            for parameter in response["data"]["parameters"]:
                parameters[parameter["name"]] = parameter["value"]

            # Debug print of raw parameters (optional)
            face_angle_x = parameters["ParamAngleX"]
            face_angle_y = parameters["ParamAngleY"]
            face_angle_z = parameters["ParamAngleZ"]
            brow_height_left = parameters["ParamBrowLY"]
            brow_height_right = parameters["ParamBrowRY"]
            eye_open_left = parameters["ParamEyeLOpen"]
            eye_open_right = parameters["ParamEyeROpen"]
            eye_left_x = parameters["ParamEyeBallX"]
            eye_left_y = parameters["ParamEyeBallY"]
            eye_right_x = parameters["ParamEyeBallX"]
            eye_right_y = parameters["ParamEyeBallY"]

            print(
                f"Face=({face_angle_x:.1f}, {face_angle_y:.1f}, {face_angle_z:.1f}) "
                f"Brows=({brow_height_left:.1f}, {brow_height_right:.1f}) "
                f"EyeOpen=({eye_open_left:.1f}, {eye_open_right:.1f}) "
                f"EyePosition=({eye_left_x:.1f}, {eye_left_y:.1f}, {eye_right_x:.1f}, {eye_right_y:.1f})"
            )

            # Build and send normalized UDP message
            message = _build_udp_message(parameters)
            sock.sendto(message.tobytes(), (ADDR, PORT))

            rate.sleep()
    finally:
        sock.close()

if __name__ == "__main__":
    asyncio.run(main())
