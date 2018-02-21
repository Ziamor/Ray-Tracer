To use the raytracer run raytracer.py with these arguments:
Render part A: python raytracerÃ.py 1 x y image_name
Render part B: python raytracer.py 2 x y image_name scene_file
Render part C: python raytracer.py 3 x y image_name scene_file

where:
x -> image width
y -> image height
image_name -> name of the file to output
scene_file -> name of the scene file to render

Format of the scene file:
First line: [# of spheres],[# of lights],[enable ambient], [enable diffuse], [enable specular], [enable shadow]
Second line(value for ambient light from 0.0 to 1.0): ambient,[R],[G],[B]
Any line from here on follows this format:
light,[X],[Y],[Z],[R],[G],[B]
sphere,[X],[Y],[Z],[Radius],[R],[G],[B],[diffuse coefficient],[ambient coefficient],[specular reflection],[specular exponent]
