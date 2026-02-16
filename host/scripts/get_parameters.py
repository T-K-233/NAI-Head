import asyncio

from loop_rate_limiters import RateLimiter

from common import create_vts_client


async def main():
    vts = await create_vts_client()

    rate = RateLimiter(frequency=30.0)

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
        
        rate.sleep()


if __name__ == "__main__":
    asyncio.run(main())
