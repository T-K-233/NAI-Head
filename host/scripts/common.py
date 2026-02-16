import asyncio
import pyvts


async def create_vts_client() -> pyvts.vts:
    plugin_info = {
        "plugin_name": "VTubeStudio Python Interface",
        "developer": "-T.K.-",
        "authentication_token_path": "./vts_token.txt"
    }

    vts = pyvts.vts(plugin_info=plugin_info)
    await vts.connect()

    await vts.request_authenticate_token()
    success = await vts.request_authenticate()
    if not success:
        print("Failed to connect to VTubeStudio, exiting...")
        exit(1)
    print("Connected to VTubeStudio.")

    return vts


async def main():
    vts = await create_vts_client()
    await vts.close()


if __name__ == "__main__":
    asyncio.run(main())