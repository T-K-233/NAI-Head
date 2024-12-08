import time
import math
import json
import uuid

import numpy as np
from websockets.sync.client import connect

from .cfg import VTubeStudioBridgeCfg
from .fields import MessageType, InputParameter



class VTubeStudioAPI:
    """
    The API Driver for interacting with VTubeStudio.
    """
    def __init__(self, cfg: VTubeStudioBridgeCfg = VTubeStudioBridgeCfg()):
        """
        Initialize the API Driver.

        Args:
            url (str, optional): The URL of the VTubeStudio WebSocket server. Default value from VTubeStudio is "ws://10.0.0.2:8001".
        """
        self.cfg = cfg
        self.websocket = connect(self.cfg.url)
    
    def close(self):
        """
        Close the WebSocket connection.
        """
        self.websocket.close()
    
    def __enter__(self):
        """
        Return the object itself when used in a with statement.
        """
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """
        Close the WebSocket connection when the object is deleted.
        """
        self.close()
    
    def build_message(self, message_type: MessageType, data: dict = None) -> dict:
        message = {
            "apiName": VTubeStudioBridgeCfg.api_name,
            "apiVersion": VTubeStudioBridgeCfg.api_version,
            "requestID": str(uuid.uuid4()),
            "messageType": message_type,
            "data": data
        }
        
        return message
        

    def send(self, message: dict, timeout: float = 0.1) -> dict:
        """
        Send a message to the VTubeStudio WebSocket server and receive a response.
        
        Args:
            message (dict): The message to send to the VTubeStudio WebSocket server.
            timeout (float, optional): The timeout for the response. Default value is 0.1 seconds.
        """
        self.websocket.send(json.dumps(message))
        response = json.loads(self.websocket.recv(timeout=timeout))
        return response
    
    def set_parameters(self, parameters: dict[str, float], timeout: float = 0.1):
        """
        Send a message to the VTubeStudio WebSocket server to set a parameter value.
        
        Args:
            parameters (dict[str, float]): The parameters to set.
            timeout (float, optional): The timeout for the response. Default value is 0.1 seconds.
        """
        
        message = self.build_message(MessageType.InjectParameterDataRequest, {
            "faceFound": False,
            "mode": "set",
            "parameterValues": [
                {
                    "id": parameter,
                    "value": value
                }
                for parameter, value in parameters.items()
            ]
        })
        self.send(message)

    def read_token(self) -> str | None:
        """
        Read the authentication token from the token file.
        """
        try:
            with open(self.cfg.token_path, "r") as file:
                return file.read()
        except FileNotFoundError:
            return None
    
    def write_token(self, token: str):
        """
        Write the authentication token to the token file.
        """
        with open(self.cfg.token_path, "w") as file:
            file.write(token)

    def request_token(self) -> str:
        """
        Request an authentication token from the VTubeStudio WebSocket server.
        """
        token_request = self.build_message(MessageType.AuthenticationTokenRequest, {
            "pluginName": VTubeStudioBridgeCfg.plugin_name,
            "pluginDeveloper": VTubeStudioBridgeCfg.plugin_developer,
        })
        
        timeout = 10
        
        print(f"Requesting token... Please authorize in VTubeStudio, the request expires in {timeout} seconds.")
        response = self.send(token_request, timeout=timeout)
        
        if not response:
            raise Exception("Failed to request token from VTubeStudio.")
        if not response.get("data") or not response["data"].get("authenticationToken"):
            raise Exception("Received invalid response from VTubeStudio.")
        
        token = response["data"]["authenticationToken"]
        
        # store token
        self.write_token(token)
        
        return token

    def authenticate(self) -> bool:
        token = self.read_token()
        
        if not token:
            token = self.request_token()
        
        auth_request = self.build_message(
            MessageType.AuthenticationRequest,
            {
                "pluginName": VTubeStudioBridgeCfg.plugin_name,
                "pluginDeveloper": VTubeStudioBridgeCfg.plugin_developer,
                "authenticationToken": token,
            }
        )
        response = self.send(auth_request)
        
        if not response["data"]["authenticated"]:
            reason = response["data"]["reason"]
            print(f"Failed to authenticate with VTubeStudio with reason: {reason}")
            return False
        
        print("Authenticated with VTubeStudio.")
        return True

