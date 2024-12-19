import socket
import cv2
import mediapipe as mp
from keypoint_classifier import *
import copy
import itertools
import numpy as np

ADDR = "172.28.0.64"
PORT = 7000

N_STATES = 12

states = np.zeros(N_STATES, dtype=np.float32)

states[7] = 1.0
states[8] = 1.0
states[9] = 0.0

# gesture
states[10] = 0.0

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", PORT))



mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

def calc_landmark_list(image, landmarks):
    image_width, image_height = image.shape[1], image.shape[0]

    landmark_point = []

    # Keypoint
    for _, landmark in enumerate(landmarks.landmark):
        landmark_x = min(int(landmark.x * image_width), image_width - 1)
        landmark_y = min(int(landmark.y * image_height), image_height - 1)
        # landmark_z = landmark.z

        landmark_point.append([landmark_x, landmark_y])

    return landmark_point

def pre_process_landmark(landmark_list):
    temp_landmark_list = copy.deepcopy(landmark_list)

    # Convert to relative coordinates
    base_x, base_y = 0, 0
    for index, landmark_point in enumerate(temp_landmark_list):
        if index == 0:
            base_x, base_y = landmark_point[0], landmark_point[1]

        temp_landmark_list[index][0] = temp_landmark_list[index][0] - base_x
        temp_landmark_list[index][1] = temp_landmark_list[index][1] - base_y

    # Convert to a one-dimensional list
    temp_landmark_list = list(
        itertools.chain.from_iterable(temp_landmark_list))

    # Normalization
    max_value = max(list(map(abs, temp_landmark_list)))

    def normalize_(n):
        return n / max_value

    temp_landmark_list = list(map(normalize_, temp_landmark_list))

    return temp_landmark_list

gesture_model = get_model()

cap = cv2.VideoCapture(0)

with mp_hands.Hands(
    model_complexity=0,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5) as hands:

    print("Camera started")
    while cap.isOpened():
        # print("Reading frame")
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            exit()
        # If loading a video, use 'break' instead of 'continue'.
            

        # To improve performance, optionally mark the image as not writeable to
        # pass by reference.
        image.flags.writeable = False
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image)
        # Draw the hand annotations on the image.
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        label_ind = 0
        if results.multi_hand_landmarks:
            landmark_list = calc_landmark_list(image, results.multi_hand_landmarks[0])
            # print(results.multi_hand_landmarks)
            pre_processed_landmark_list = pre_process_landmark(
                            landmark_list)
            features = torch.tensor(pre_processed_landmark_list).unsqueeze(0)
            outputs = gesture_model(features)
            label_ind = torch.argmax(outputs).item() + 1
            for hand_landmarks in results.multi_hand_landmarks:
                mp_drawing.draw_landmarks(
                image,
                hand_landmarks,
                mp_hands.HAND_CONNECTIONS,
                mp_drawing_styles.get_default_hand_landmarks_style(),
                mp_drawing_styles.get_default_hand_connections_style())

        print(label_ind)

        states[10] = label_ind
        sock.sendto(states.tobytes(), (ADDR, PORT))
        
        # Flip the image horizontally for a selfie-view display.
        cv2.imshow("MediaPipe Hands", cv2.flip(image, 1))
        if cv2.waitKey(5) & 0xFF == 27:
            break
    cap.release()