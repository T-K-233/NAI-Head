import time

from VTubeStudioBridge import VTubeStudioAPI, InputParameter

client = VTubeStudioAPI()

client.authenticate()



client.set_parameters(
    {
        InputParameter.EyeOpenLeft: 0.1,
        InputParameter.MouthOpen: 1,
    }
)


client.close()

print("Done")