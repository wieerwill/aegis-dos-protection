import sys
from os import path
from PyQt5.QtWidgets import QApplication, QWidget, QLabel
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QPixmap

WINDOW_TITLE = "Attack Status"
POSITION = [0, 0]
SIZE = [912, 513]

FOLDER = ""
if not path.isfile(FOLDER + "gui.py"):
    FOLDER = "subprojects/gui/"
PATH_ATTACK_FILE = "/home/guru/is_attacking/attacking" # Location of 'attacking'-file (interface)
PATH_SMILEY = FOLDER + "img/smiley.png"
PATH_SKULL = [FOLDER + "img/skull_black.png", FOLDER + "img/skull_orange.png"]

DELTA_TIME = 300
GREEN = "#7cb342"
ORANGE = "#e65100"
BLACK = "#000000"

class Window(QWidget):
    run = True # Run/stop loop
    inverted = True # Use inverted colors

    def __init__(self):
        # Initialize window
        super().__init__()
        self.setWindowTitle(WINDOW_TITLE)
        self.setGeometry(POSITION[0], POSITION[1], SIZE[0], SIZE[1])

        # Show image
        self.image = QLabel(self)
        self.loop()
        self.show()

    def loop(self):
        # Start timer
        if self.run:
            QTimer.singleShot(DELTA_TIME, self.loop)

        # Update image
        if path.isfile(PATH_ATTACK_FILE):
            self.inverted = not self.inverted
            if self.inverted:
                self.update_image(PATH_SKULL[1], BLACK)
            else:
                self.update_image(PATH_SKULL[0], ORANGE)
        else:
            self.update_image(PATH_SMILEY, GREEN)

    def update_image(self, image_path, background_color):
        self.image.setPixmap(QPixmap(image_path))
        self.setStyleSheet("background-color: " + background_color)


if __name__ == "__main__":
    # Run app until closed
    app = QApplication(sys.argv)
    window = Window()
    app.exec_()

    # Terminate
    window.run = False
    sys.exit()
    