import asyncio
from loop_rate_limiters import RateLimiter
from common import create_vts_client


async def main():
    vts = await create_vts_client()

    rate = RateLimiter(frequency=30.0)

    while True:
        response = await vts.request(
            vts.vts_request.BaseRequest("Live2DParameterListRequest")
        )
        parameters = {}
        for parameter in response["data"]["parameters"]:
            parameters[parameter["name"]] = parameter["value"]

        # yaw left (-45) to right (+45)
        face_angle_x = parameters["ParamAngleX"]
        # pitch down (-45) to up (+45)
        face_angle_y = parameters["ParamAngleY"]
        # tilt left (-45) to right (+45)
        face_angle_z = parameters["ParamAngleZ"]

        # full down (0) to full raise (1)
        brow_height_left = parameters["ParamBrowLY"]
        brow_height_right = parameters["ParamBrowRY"]

        # full closed (0) to full open (1)
        eye_open_left = parameters["ParamEyeLOpen"]
        eye_open_right = parameters["ParamEyeROpen"]

        # x position left (-1) to right (+1)
        eye_left_x = parameters["ParamEyeBallX"]
        # y position down (-1) to up (+1)
        eye_left_y = parameters["ParamEyeBallY"]
        # x position left (-1) to right (+1)
        eye_right_x = parameters["ParamEyeBallX"]
        # y position down (-1) to up (+1)
        eye_right_y = parameters["ParamEyeBallY"]

        print(
            f"Face=({face_angle_x:.1f}, {face_angle_y:.1f}, {face_angle_z:.1f}) "
            f"Brows=({brow_height_left:.1f}, {brow_height_right:.1f}) "
            f"EyeOpen=({eye_open_left:.1f}, {eye_open_right:.1f}) "
            f"EyePosition=({eye_left_x:.1f}, {eye_left_y:.1f}, {eye_right_x:.1f}, {eye_right_y:.1f})"
        )

        rate.sleep()


if __name__ == "__main__":
    asyncio.run(main())
