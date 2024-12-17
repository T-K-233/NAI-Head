from torchvision import transforms
from torch.utils.data import Dataset, DataLoader, random_split
import torch
import torch.nn as nn
import torch.optim as optim
import pandas as pd
import torch.nn.functional as F

INPUT_SIZE = 21 * 2
NUM_CLASSES = 4

class KeypointClassifier(nn.Module):
    def __init__(self, input_size, num_classes):
        super().__init__()
        # super(KeypointClassifier, self).__init__()
        self.dropout1 = nn.Dropout(0.2)
        self.fc1 = nn.Linear(input_size, 20)
        self.dropout2 = nn.Dropout(0.4)
        self.fc2 = nn.Linear(20, 10)
        self.fc3 = nn.Linear(10, num_classes)

    def forward(self, x):
        x = self.dropout1(x)
        x = F.relu(self.fc1(x))
        x = self.dropout2(x)
        x = F.relu(self.fc2(x))
        x = F.softmax(self.fc3(x), dim=1)
        return x

def get_model():
    model = KeypointClassifier(INPUT_SIZE, NUM_CLASSES)
    model.load_state_dict(torch.load('keypoint_classifier_torch.pth', weights_only=True))
    model.eval()
    return model