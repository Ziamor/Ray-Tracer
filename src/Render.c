/*
 * Alex Zukowski
 * 5347704
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Render.h"

/*
 * Get the pixal located at x,y from a gradiant image. The red value starts at 0 and progresses to 100% as x increases.
 * Green value is alwasy 25%. Blue increases as y increases.
 */
Color* getPixalOfFunkyTexture(float x, float y, float x_size, float y_size){
	float x_percent = x / x_size; // Get the current position percent for x and y
	float y_percent = y / y_size;
	Color *pixal = calloc(1, sizeof(Color)); // Allocate mem for a color
	pixal->r = x_percent; // Set the r value to the current x progress
	pixal->g = 0.25; // Set g to a fixed 25%
	pixal->b  = y_percent; // Set the b value to y progress
	return pixal;
}

/*
 * Allocatte space for a new scene and return it
 */
Scene* newScene(int sphereCount, int lightCount, float r, float g, float b, int enableAmbient, int enableDiffuse, int enableSpecular, int enableShadow){
	printf("Creating scene with ambient: %i diffuse: %i specualar: %i shadow: %i\n", enableAmbient, enableDiffuse, enableSpecular, enableShadow);
	Scene *scene = calloc(1,sizeof(Scene)); // Allocate memory for the scene
	Spheres *spheres = calloc(1,sizeof(Spheres) + sizeof(Sphere) * sphereCount); // Allocate space for the sphere container
	Lights *lights = calloc(1,sizeof(Lights) + sizeof(Light) * lightCount); // Allocate space for the light container

	// Init Spheres
	scene->spheres = spheres;
	scene->spheres->insertPoint = 0;
	scene->spheres->sphereCount = sphereCount;

	// Init Lights
	scene->lights = lights;
	scene->lights->insertPoint = 0;
	scene->lights->lightCount = lightCount;

	// Init the ambient light
	Color *ambient = calloc(1,sizeof(Color));
	scene->ambient = ambient;
	scene->ambient->r = r;
	scene->ambient->g = g;
	scene->ambient->b = b;

	// Set the render settings of the scene
	scene->enableAmbient = enableAmbient;
	scene->enableDiffuse = enableDiffuse;
	scene->enableSpecular = enableSpecular;
	scene->enableShadow = enableShadow;
	return scene;
}

/*
 * Deallocate memory of the scene
 */
void destroyScene(Scene *scene){
	int i;
	// Free each sphere from the scene as well as it's color
	for(i = 0; i < scene->spheres->sphereCount; i++){
		Sphere *sphere = scene->spheres->spheres[i];
		if(sphere == NULL)
			continue;
		if(sphere->color != NULL)
			free(sphere->color);
		free(sphere);
		scene->spheres->spheres[i] = NULL;
	}
	// Free each light from the scene as well as it's color
	for(i = 0; i < scene->lights->lightCount; i++){
		Light *light = scene->lights->lights[i];
		if(light == NULL)
			continue;
		if(light->color != NULL)
			free(light->color);
		free(light);
		scene->lights->lights[i] = NULL;
	}
	// Free scene and its containers
	free(scene->spheres);
	free(scene->lights);
	free(scene->ambient);
	free(scene);
}

/*
 * Deallocate memory of a color
 */
void destroyColor(Color *color){
	if(color != NULL)
		free(color);
}

/*
 * Add a sphere to the scene.
 */
void addSphere(int x, int y, int z, int rad, float kd, float ka, float ks, float se, float r, float g, float b, Scene *scene){
	printf("Adding sphere with X Y Z Radius: %i %i %i %i kd ka ks se: %f %f %f %f RGB: %f %f %f\n",x ,y, z, rad, kd, ka, ks, se, r, g, b);
	Sphere *sphere = calloc(1,sizeof(Sphere));

	// Set the sphere position and radius
	sphere->x = x;
	sphere->y = y;
	sphere->z = z;
	sphere->r = rad;

	// Set the spheres shader settings
	sphere->kd = kd;
	sphere->ka = ka;
	sphere->ks = ks;
	sphere->se = se;

	// Set the spheres color
	Color *color = calloc(1,sizeof(Color));
	color->r = r;
	color->g = g;
	color->b = b;
	sphere->color = color;

	// Check if we have room to insert sphere
	if(scene->spheres->insertPoint >= scene->spheres->sphereCount)
		printf("Trying to insert more spheres then scene can handle.\n");
	else{
		// Add the sphere to the sphere container at the insert point and increment the insert point for the next add
		scene->spheres->spheres[scene->spheres->insertPoint] = sphere;
		scene->spheres->insertPoint += 1;
	}
}

/*
 * Add a light to the scene
 */
void addLight(int x, int y, int z, float r, float g, float b, Scene *scene){
	printf("Adding light with X Y Z: %i %i %i\n", x, y, z);
	Light *light = calloc(1,sizeof(Light));

	// Set the light position
	light->x = x;
	light->y = y;
	light->z = z;

	// Set the light color
	Color *color = calloc(1,sizeof(Color));
	color->r = r;
	color->g = g;
	color->b = b;
	light->color = color;

	// Check if we have room to insert light
	if(scene->lights->insertPoint >= scene->lights->lightCount)
		printf("Trying to insert more lights then scene can handle.\n");
	else{
		// Add the light to the light container at the insert point and increment the insert point for the next add
		scene->lights->lights[scene->lights->insertPoint] = light;
		scene->lights->insertPoint += 1;
	}
}

/*
 * Takes two colors and multiplies them and then sets the value to a third color to be returned
 */
static void multiplyColor(Color *cOne, Color *cTwo, Color *out){
	out->r = cOne->r * cTwo->r;
	out->g = cOne->g * cTwo->g;
	out->b = cOne->b * cTwo->b;
}

/*
 * Takes two colors and adds them and then sets the value to a third color to be returned
 */
static void addColor(Color *cOne, Color *cTwo, Color *out){
	out->r = cOne->r + cTwo->r;
	out->g = cOne->g + cTwo->g;
	out->b = cOne->b + cTwo->b;
}

/*
 * Gets the color value of a ray casted at a position x y
 */
Color* rayGetPixal(int x, int y, int silhouette, Scene *scene){
	Color *pixal = calloc(1, sizeof(Color));
	int i;
	// Loop through all spheres and cast a ray at it
	for(i = 0; i < scene->spheres->insertPoint; i++){
		// Get the next sphere
		Sphere *sphere = scene->spheres->spheres[i];
		// If the sphere is NULL skip it
		if(sphere == NULL)
			continue;

		// Set the values up to calcualte intersection
		int z = 0;
		int cx = sphere->x;
		int cy = sphere->y;
		int cz = sphere->z;
		int cr = sphere->r;
		int dx = 0;
		int dy = 0;
		int dz = 1;

		// Get the discriminant value for the sphere at point x y
		float discriminantValue = discriminant(x,y,z,cx,cy,cz,cr,dx,dy,dz);
		// If the value is greater or equal to 0 then that means there is an intersection
		if(discriminantValue >= 0.0){
			// if silhouette is enable all we care is that we intersected somthing so set the pixal to white and return it
			if(silhouette){
				pixal->r = 1;
				pixal->g = 1;
				pixal->b = 1;
				return pixal;
			}
			// get the distance of the intersection
			float t = distance(x,y,z,cx,cy,cz,cr,dx,dy,dz);

			// Copy the spheres r g b values to the pixal
			pixal->r = sphere->color->r;
			pixal->g = sphere->color->g;
			pixal->b = sphere->color->b;
			// If ambient is enabled multiply the spheres color by the ambient light
			if(scene->enableAmbient)
				multiplyColor(sphere->color, scene->ambient, pixal);
			// Set diffuse/specular/shadow
			setShader(x,y,z,dx,dy,dz,t,sphere->kd,sphere->ka,sphere->ks,sphere->se, pixal,sphere,scene);

			return clamp(pixal);
		}

		// Ignore this, nothing to see here...
		/*else if(scene->enableShadow){
			int j;
			for(j = 0; j < scene->lights->insertPoint; j++){
				Light *light = scene->lights->lights[j];
				if(checkIfIntersection(x,y,z,light->x,light->y,light->z,sphere,scene)){
					pixal->r = 0.75 * 0.5;
					pixal->g = 0.75 * 0.5;
					pixal->b = 0.75 * 0.5;
					return clamp(pixal);
				}
			}
		}*/
	}

	// If we are here that means no intersection was found so just render the background

	// If silhouette is enabled set the background color to black
	if(silhouette){
		pixal->r = 0;
		pixal->g = 0;
		pixal->b = 0;
		return pixal;
	}

	// Set the background color
	pixal->r = 0.75;
	pixal->g = 0.75;
	pixal->b = 0.75;

	return clamp(pixal);
}

/*
 * Set diffuse/specular/shadows if they are enabled
 */
static void setShader(float x, float y, float z, float dx, float dy, float dz, float t, float kd, float ka, float ks, float se, Color *color, Sphere *sphere, Scene *scene){
	float cx = sphere->x;
	float cy = sphere->y;
	float cz = sphere->z;
	float cr = sphere->r;

	// Get the intersection point
	x = x + t * dx;
	y = y + t * dy;
	z = z + t * dz;

	// Get unit bector
	float nx = (x - cx)/cr;
	float ny = (y - cy)/cr;
	float nz = (z - cz)/cr;

	Color *diffuse = calloc(1,sizeof(Color));
	Color *specular = calloc(1,sizeof(Color));
	int i;
	// Loop through all lights in the scene
	for(i = 0; i < scene->lights->insertPoint; i++){
		Light *light = scene->lights->lights[i];
		if(light == NULL)
			continue;
		// Get the vector of the light
		float lx = light->x - x;
		float ly = light->y - y;
		float lz = light->z - z;

		// If shadows are enabled and there is an intersection with a sphere before the light is hit we skip any more rendering so that only ambient is visable
		if(scene->enableShadow && checkIfIntersection(x,y,z,light->x,light->y,light->z,sphere,scene))
			continue;

		// Get the vector length
		float len = sqrt(pow(x,2) + pow(y,2) + pow(z,2));

		// Get light unit vector
		lx = lx / len;
		ly = ly / len;
		lz = lz / len;

		// Calculate the diffuse multiplier
		float fx = nx * lx;
		float fy = ny * ly;
		float fz = nz * lz;

		float f = fx + fy + fz;

		// Make sure the multiplier is > 0
		if(f < 0)
			f = 0;

		// Get the RGB values of the spheres material
		float red = sphere->color->r;
		float green = sphere->color->g;
		float blue = sphere->color->b;

		// Get the diffuse color
		diffuse->r = (red * kd * f) + (red * ka);
		diffuse->g = (green * kd * f) + (green * ka);
		diffuse->b = (blue * kd * f) + (blue * ka);

		// Calculate reflection vector
		float factor = dx * nx + dy * ny + dz + nz;
		float refx = dx + 2 * factor * nx;
		float refy = dy + 2 * factor * ny;
		float refz = dz + 2 * factor * nz;

		// Get the spec multiplier
		float spec = pow(refx * lx + refy * ly + refz * lz, se);
		// Get the specualr color
		specular->r = (red * ks * spec) + (red * ks);
		specular->g = (green * ks * spec) + (green * ks);
		specular->b = (blue * ks * spec) + (blue * ks);

		// Add diffuse and specualr to the color if enabled
		if(scene->enableDiffuse)
			addColor(diffuse,color,color);
		if(scene->enableSpecular)
			addColor(specular,color,color);
	}
	// Free diffuse and specualr from memory
	free(diffuse);
	free(specular);
}

/*
 * Calcualtes the discriminate value
 */
static float discriminant(float x, float y, float z, float cx, float cy, float cz, float cr, float dx, float dy, float dz){
	int a = dx * dx + dy * dy + dz * dz;
	int b = 2 * dx * (x - cx) + 2 * dy * (y - cy) + 2 * dz * (z - cz);
	int c = cx * cx + cy * cy + cz * cz + x * x + y * y + z * z + -2 * (cx * x + cy * y + cz * z) - cr * cr;

	float discriminant = (float) pow(b,2) - 4.0 * (float) a * (float) c;

	return discriminant;
}

/*
 * Calculates the distance of intersection
 */
static float distance(float x, float y, float z, float cx, float cy, float cz, float cr, float dx, float dy, float dz){
	int a = dx * dx + dy * dy + dz * dz;
	int b = 2 * dx * (x - cx) + 2 * dy * (y - cy) + 2 * dz * (z - cz);
	int c = cx * cx + cy * cy + cz * cz + x * x + y * y + z * z + -2 * (cx * x + cy * y + cz * z) - cr * cr;

	float discriminant = (float) pow(b,2) - 4.0 * (float) a * (float) c;

	float t = (-b-sqrt(discriminant))/(2*a);

	return t;
}

/*
 * Checks if a point of a sphere intersects with another sphere
 */
static int checkIfIntersection(int x, int y, int z, int x1, int y1, int z1, Sphere *sphereCheck, Scene *scene){

	int i;
	for(i = 0; i < scene->spheres->insertPoint; i++){
		Sphere *sphere = scene->spheres->spheres[i];
		if(sphere == sphereCheck)
			continue;
		float dx = x1 - x;
		float dy = y1 - y;
		float dz = dz - z;
		float discriminantValue = discriminant(x,y,z,sphere->x,sphere->y,sphere->z,sphere->r,dx,dy,dz);
		if(discriminantValue >= 0.0)
			return 1;
	}
	return 0;
}

/*
 * Clamps the upper bound to 1 and lower bound to 0
 */
float clampf(float a){
	if(a > 1)
		return 1;
	if(a < 0)
		return 0;
	return a;
}
/*
 * Clamps the colors RGB values
 */
Color* clamp(Color *a){
	a->r = clampf(a->r);
	a->g = clampf(a->g);
	a->b = clampf(a->b);
	return a;
}
