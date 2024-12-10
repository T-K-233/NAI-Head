


class MessageType:
    AuthenticationRequest = "AuthenticationRequest"
    AuthenticationTokenRequest = "AuthenticationTokenRequest"
    InjectParameterDataRequest = "InjectParameterDataRequest"


class InputParameter:
    """
    A list of supported input parameters for VTubeStudio.
    
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
