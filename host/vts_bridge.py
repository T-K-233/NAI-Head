import time
import math
import json

import numpy as np
from websockets.sync.client import connect

PLUGIN_NAME = "Sample Plugin"
PLUGIN_DEVELOPER = "-T.K.-"

message_template = {
        "apiName": "VTubeStudioPublicAPI",
        "apiVersion": "1.0",
        # "requestID": "",
        "messageType": "",
        "data": {
            "pluginName": PLUGIN_NAME,
            "pluginDeveloper": PLUGIN_DEVELOPER
        }
    }

class InputParameters:
    """
    see https://github.com/DenchiSoft/VTubeStudio/wiki/VTS-Model-Settings#supported-input-parameters-face-tracking-etc
    """
    FacePositionX   = "FacePositionX"   # horizontal position of face
    FacePositionY   = "FacePositionY"   # vertical position of face
    FacePositionZ   = "FacePositionZ"   # distance from camera
    FaceAngleX      = "FaceAngleX"      # face right/left rotation
    FaceAngleY      = "FaceAngleY"      # face up/down rotation
    FaceAngleZ      = "FaceAngleZ"      # face lean rotation
    MouthSmile      = "MouthSmile"      # how much you're smiling
    MouthOpen       = "MouthOpen"       # how open your mouth is
    Brows           = "Brows"           # up/down for both brows combined
    MousePositionX  = "MousePositionX"  # x-pos. of mouse or finger within set range
    MousePositionY  = "MousePositionY"  # y-pos. of mouse or finger within set range
    TongueOut       = "TongueOut"       # stick out your tongue
    EyeOpenLeft     = "EyeOpenLeft"     # how open your left eye is
    EyeOpenRight    = "EyeOpenRight"    # how open your right eye is
    EyeLeftX        = "EyeLeftX"        # eye-tracking
    EyeLeftY        = "EyeLeftY"        # eye-tracking
    EyeRightX       = "EyeRightX"       # eye-tracking
    EyeRightY       = "EyeRightY"       # eye-tracking
    CheekPuff       = "CheekPuff"       # detects when you puff out your cheeks
    BrowLeftY       = "BrowLeftY"       # up/down for left brow
    BrowRightY      = "BrowRightY"      # up/down for right brow
    VoiceFrequency  = "VoiceFrequency"  # depends on detected phonemes
    VoiceVolume     = "VoiceVolume"     # how loud microphone volume is
    VoiceVolumePlusMouthOpen = "VoiceVolumePlusMouthOpen" # MouthOpen + VoiceVolume
    VoiceFrequencyPlusSmile = "VoiceFrequencyPlusSmile" # MouthSmile + VoiceFrequency
    VoiceA          = "VoiceA"          # detected phoneme: A
    VoiceE          = "VoiceE"          # detected phoneme: E
    VoiceI          = "VoiceI"          # detected phoneme: I
    VoiceO          = "VoiceO"          # detected phoneme: O
    VoiceU          = "VoiceU"          # detected phoneme: U
    VoiceSilence    = "VoiceSilence"    # detected phoneme: Silence
    MouthX          = "MouthX"          # Mouth X position (shift mouth left/right)
    FaceAngry       = "FaceAngry"       # detects angry face (EXPERIMENTAL, not recommended)
    # hand-tracking parameters: https://github.com/DenchiSoft/VTubeStudio/wiki/Hand-Tracking

class VTubeStudioAPI:
    def __init__(self, url="ws://10.0.0.2:8001"):
        self.websocket = connect(url)

    def send(self, message: dict):
        self.websocket.send(json.dumps(message))
        response = json.loads(self.websocket.recv(timeout=0.1))
        return response

    def requestToken(self):
        token_request = message_template.copy()
        token_request["messageType"] = "AuthenticationTokenRequest"
        response = self.send(token_request)
        token = response["data"]["authenticationToken"]
        with open("token.txt", "w") as file:
            file.write(token)
        return token

    def authenticate(self):
        # detect if token exists
        try:
            with open("token.txt", "r") as file:
                token = file.read()
        except FileNotFoundError:
            token = None
        
        if not token:
            self.requestToken()
        
        auth_request = message_template.copy()
        auth_request["messageType"] = "AuthenticationRequest"
        auth_request["data"]["authenticationToken"] = token
        
        response = self.send(auth_request)
        print(response)

