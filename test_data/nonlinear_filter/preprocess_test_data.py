#-*- coding=utf-8 -*-
import cv2
import time
import numpy as np

filename = "test1.png"
img_origin = cv2.imread(filename)
size = (640, 480)
img_origin = cv2.resize(img_origin, size, interpolation=cv2.INTER_AREA)
##save rgb 3 channel image data to txt
img_origin.tofile("rgbx3x480x640.txt")
gray_origin = cv2.cvtColor(img_origin, cv2.COLOR_RGB2GRAY)
gray_origin.tofile("rgbx1x480x640.txt")
size = (640, 483)
gray_origin = cv2.resize(gray_origin, size, interpolation=cv2.INTER_AREA)
gray_origin.tofile("rgbx1x483x640.txt")

