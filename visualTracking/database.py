"""
Elliot Greenlee
Date Started: 2018-05-16
University of Tennessee, Knoxville: WIND
"""

# Imports


# Data Classes
class BoundingBox:
    def __init__(self, upper_left_x, upper_left_y, height, width):
        self.upper_left_x = 0
        self.upper_left_y = 0
        self.height = 0
        self.width = 0

        self.center_x = 0
        self.center_y = 0
        self.lower_right_x = 0
        self.lower_right_y = 0

        self.update(upper_left_x, upper_left_y, height, width)

    def update(self, upper_left_x, upper_left_y, height, width):
        self.upper_left_x = upper_left_x
        self.upper_left_y = upper_left_y
        self.height = height
        self.width = width

        self.center_x = int(upper_left_x + width/2.0)
        self.center_y = int(upper_left_y + height/2.0)
        self.lower_right_x = int(upper_left_x + width)
        self.lower_right_y = int(upper_left_y + height)


class Target:
    def __init__(self, ID, subject, confidence, upper_left_x, upper_left_y, height, width):
        self.ID = ID
        self.subject = subject
        self.confidence = confidence
        self.bounding_box = BoundingBox(upper_left_x, upper_left_y, height, width)
        self.source_probability = 0
        self.tracking = False

    def update_bounding_box(self, upper_left_x, upper_left_y, height, width):
        self.bounding_box.update(upper_left_x, upper_left_y, height, width)

# Control
running = True

# Camera Values
input_frame = None
capturing = True
camera_height = 0
camera_width = 0

# Nuclear Detector Values
source_angle = 0

# Object Detector Values
net = None
meta = None

# Object Tracker Values


# Fusion Values


# Overall Tracking Values
targets = []

# Visualizer Values
output_frame = None


def main():
    return

if __name__ == "__main__":
    main()
