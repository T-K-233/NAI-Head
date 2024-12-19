import os


class VTubeStudioBridgeCfg:
    api_name = "VTubeStudioPublicAPI"
    api_version = "1.0"
    
    plugin_name = "VTubeStudioBridge"
    plugin_developer = "-T.K.-"
    
    # store token in user home directory
    token_path = os.path.join(os.path.expanduser("~"), ".vts_token")
    
    url = "ws://localhost:8001"
