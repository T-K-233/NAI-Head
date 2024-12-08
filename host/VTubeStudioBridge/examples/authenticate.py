import time

from VTubeStudioBridge import VTubeStudioAPI

client = VTubeStudioAPI()

client.authenticate()

client.close()

print("Done")
