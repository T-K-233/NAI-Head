import time
import asyncio
import struct

import numpy as np
import serial
from cc.xboxcontroller import XboxController

from common import create_vts_client



async def main():
    vts = await create_vts_client()

    while True:
        response = await vts.request(
            vts.vts_request.requestTrackingParameterList()
        )
        parameters = {}
        for parameter in response["data"]["defaultParameters"]:
            parameters[parameter["name"]] = parameter["value"]

        face_angle_x = parameters["FaceAngleX"]
        face_angle_y = parameters["FaceAngleY"]
        face_angle_z = parameters["FaceAngleZ"]
        brows = parameters["Brows"]
        eye_open_left = parameters["EyeOpenLeft"]
        eye_open_right = parameters["EyeOpenRight"]
        eye_left_x = parameters["EyeLeftX"]
        eye_left_y = parameters["EyeLeftY"]
        eye_right_x = parameters["EyeRightX"]
        eye_right_y = parameters["EyeRightY"]

        print(f"Face angle: {face_angle_x:.2f}, {face_angle_y:.2f}, {face_angle_z:.2f}")
        print(f"Brows: {brows:.2f}")
        print(f"Eye open: {eye_open_left:.2f}, {eye_open_right:.2f}")
        print(f"Eye position: {eye_left_x:.2f}, {eye_left_y:.2f}, {eye_right_x:.2f}, {eye_right_y:.2f}")
        print("--------------------------------")
        time.sleep(0.1)



ser = serial.Serial("COM5", 115200)
stick = XboxController(0, deadzone=0, dampen=1e-2)

state = {
    "EyeOpenLeft": 1.,
    "EyeOpenRight": 1.,
    "EyeLeftX": 0.,
    "EyeLeftY": 0.,
    "EyeRightX": 0.,
    "EyeRightY": 0.,
}

trigger_controlled = True
synced_mode = True

state_0_filtered = 0
state_1_filtered = 0
state_2_filtered = 0
state_3_filtered = 0
state_4_filtered = 0
state_5_filtered = 0

while True:
    stick.update()
    
    state["EyeLeftX"] = stick.get_left_x()
    state["EyeLeftY"] = stick.get_left_y()
    
    state["EyeRightX"] = stick.get_right_x()
    state["EyeRightY"] = stick.get_right_y()

    if trigger_controlled:
        state["EyeOpenLeft"] = 1 - stick.get_left_trigger()
        state["EyeOpenRight"] = 1 - stick.get_right_trigger()
    else:
        if stick.get_x_button():
            state["EyeOpenLeft"] = 0.
            state["EyeOpenRight"] = 0.
        else:
            state["EyeOpenLeft"] = 1.
            state["EyeOpenRight"] = 1.
    
    if stick.get_a_button():
        trigger_controlled = True
    if stick.get_b_button():
        trigger_controlled = False
    
    if stick.get_left_bumper():
        synced_mode = True
    if stick.get_right_bumper():
        synced_mode = False

    if synced_mode:
        state["EyeRightX"] = state["EyeLeftX"]
        state["EyeRightY"] = state["EyeLeftY"]
        state["EyeOpenRight"] = state["EyeOpenLeft"]
    
    filter_alpha = 0.8
    state_0_filtered = filter_alpha * state["EyeOpenLeft"] + (1 - filter_alpha) * state_0_filtered
    state_1_filtered = filter_alpha * state["EyeOpenRight"] + (1 - filter_alpha) * state_1_filtered
    state_2_filtered = filter_alpha * state["EyeLeftX"] + (1 - filter_alpha) * state_2_filtered
    state_3_filtered = filter_alpha * state["EyeLeftY"] + (1 - filter_alpha) * state_3_filtered
    state_4_filtered = filter_alpha * state["EyeRightX"] + (1 - filter_alpha) * state_4_filtered
    state_5_filtered = filter_alpha * state["EyeRightY"] + (1 - filter_alpha) * state_5_filtered

    ser_buffer = struct.pack("<ffffff", state_0_filtered, state_1_filtered, state_2_filtered, state_3_filtered, state_4_filtered, state_5_filtered)
    ser.write(ser_buffer)
    ser.flush()

    payload = {
        "apiName": "VTubeStudioPublicAPI",
        "apiVersion": "1.0",
        "requestID": "SomeID",
        "messageType": "InjectParameterDataRequest",
        "data": {
            "faceFound": False,
            "mode": "set",
            "parameterValues": [
                {
                    "id": InputParameters.EyeLeftX,
                    "value": state["EyeLeftX"]
                },
                {
                    "id": InputParameters.EyeLeftY,
                    "value": state["EyeLeftY"]
                },
                {
                    "id": InputParameters.EyeRightX,
                    "value": state["EyeRightX"]
                },
                {
                    "id": InputParameters.EyeRightY,
                    "value": state["EyeRightY"]
                },
                {
                    "id": InputParameters.EyeOpenLeft,
                    "value": state["EyeOpenLeft"]
                },
                {
                    "id": InputParameters.EyeOpenRight,
                    "value": state["EyeOpenRight"]
                }
            ]
        }
    }
    client.send(payload)


    # buf = ser.read_all()
    # if buf:
    #     print(buf)
    


    time.sleep(.1)


client.websocket.close()


if __name__ == "__main__":
    asyncio.run(main())
