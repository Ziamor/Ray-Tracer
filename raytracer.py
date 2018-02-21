#!/usr/bin/env python

# Alex Zukowski
# 5347704

import sys
from ctypes import *

class Color(Structure):
	'''Class version of c Color struct'''
	_fields_ = [("r", c_float), ("g", c_float), ("b", c_float)]
class Scene(Structure):
	'''Class version of c Scene struct'''
	pass

# Load render library
libRender = CDLL("lib/libRender.so")

# Load library functions

# Get the color value at pixal x y used for part A
getPixalOfFunkyTexture = libRender.getPixalOfFunkyTexture
getPixalOfFunkyTexture.argtypes = [c_float, c_float, c_float, c_float]
getPixalOfFunkyTexture.restype = POINTER(Color)

# Create a new scene file
newScene = libRender.newScene
newScene.argtypes = [c_int, c_int, c_float, c_float, c_float, c_int, c_int, c_int, c_int]
newScene.restype = POINTER(Scene)

# Add a sphere to the scene
addSphere = libRender.addSphere
addSphere.argtypes = [c_int, c_int, c_int, c_int, c_float, c_float, c_float, c_float, c_float, c_float, c_float, POINTER(Scene)]

# Add a light to the scene
addLight = libRender.addLight
addLight.argtypes = [c_int, c_int, c_int, c_float, c_float, c_float, POINTER(Scene)]

# Get the color value at pixal x y from a ray cast into a scene. Used for part B and C
rayGetPixal = libRender.rayGetPixal
rayGetPixal.argtypes = [c_int, c_int, c_int, POINTER(Scene)]
rayGetPixal.restype = POINTER(Color)

# Deallocate mem for a scene
destroyScene = libRender.destroyScene
destroyScene.argtypes = [POINTER(Scene)]

# Deallocate mem for a color
destroyColor = libRender.destroyColor
destroyColor.argtypes = [POINTER(Color)]

# Get arg length and values
argc = len(sys.argv)
argv = sys.argv

# At min 4 args are required including the default first one(Script name)
if argc < 4:
	print "Invalid number of parameters"
	sys.exit()

# Represents what operation to run. 1 for funky texture of part A. 2 for silhouette of part B. 3 for raytracer of part C
operation = int(argv[1])

# Get the image size
x = int(argv[2])
y = int(argv[3])

# Get the file name
file_name = argv[4]

# The scene file to load the scene
scene_file = None

# A pointer to the scene
scene = None

# Exit if an invalid operation is selected
if not (operation == 1 or operation == 2 or operation == 3):
	print "Invalid operation selected"
	sys.exit()
# If the operation is 2 or 3, we need to create a scene from a file
if operation == 2 or operation == 3:
	# Get the file name
	scene_file = argv[5]

	# Open the scene file
	with open(scene_file, "r") as f:
		# Read the first line as it describes the scene
		line = f.next().strip().split(",")
		
		# Number of max spheres in the scene
		sphereCount = int(line[0])
		# Number of max lights in the scene
		lightCount = int(line[1])

		# Set the scenes render settings
		enableAmbient = int(line[2]);	
		enableDiffuse = int(line[3]);	
		enableSpecular = int(line[4]);	
		enableShadow = int(line[5]);	

		# Read the second line and get the ambient light information
		line = f.next().strip().split(",")
		ambR = float(line[1])	
		ambG = float(line[2])	
		ambB = float(line[3])	
		
		# Create the new scene
		scene = newScene(sphereCount, lightCount, ambR, ambG, ambB, enableAmbient, enableDiffuse, enableSpecular, enableShadow)
		# For each line add the correct object to the scene
		for line in f:
			line = line.strip()
			objData = line.split(",")
			# If first element = sphere then add a sphere
			if objData[0] == "sphere":
				addSphere(int(objData[1]), int(objData[2]), int(objData[3]), int(objData[4]), float(objData[8]), float(objData[9]), float(objData[10]), float(objData[11]), float(objData[5]), float(objData[6]), float(objData[7]), scene)
			# If first element = light then add a light
			elif objData[0] == "light":
				addLight(int(objData[1]), int(objData[2]), int(objData[3]), float(objData[4]), float(objData[5]), float(objData[6]), scene)

# Render and create the image file
with open(file_name, "wb") as f:
	print "Image size x: " +  str(x) + " y: " + str(y)
	# Write the header information
	f.write("P6" + chr(10))
	f.write(str(x) + " " + str(y) + chr(10))
	f.write(str(255) + chr(10))
	# Loop through all pixals in image
	for yi in range(0,y):
		for xi in range(0,x):
			pixal = None
			# If operation is 1 then get the pixal from the funky texture
			if operation == 1:
				pixal = getPixalOfFunkyTexture(xi,yi,x,y)
			# else if 2 get the silhouette pixal
			elif operation == 2:
				pixal = rayGetPixal(xi,yi,1,scene)
			# else if 3 get the raytracer pixal
			elif operation == 3:
				pixal = rayGetPixal(xi,yi,0,scene)
			# If for some reason pixal is null, we exit
			if pixal is None:
				print "Something went horribly wrong"
				sys.exit()
			# Write the pixal to the file
			f.write(chr(int(pixal.contents.r * 255.0)))
			f.write(chr(int(pixal.contents.g * 255.0)))
			f.write(chr(int(pixal.contents.b * 255.0)))
			# Deallocate the pixal as we are done with it
			destroyColor(pixal)

# Deallocate the scene if it was created
if scene != None:
	destroyScene(scene);
