/*
 * Alex Zukowski
 * 5347704
 */

typedef struct {
	float r,g,b;
} Color;

typedef struct {
	int x,y,z,r;
	float kd,ka,ks,se;
	Color *color;
} Sphere;

typedef struct {
	int x,y,z;
	Color *color;
} Light;

typedef struct {
	int sphereCount;
	int insertPoint;
	Sphere *spheres[];
} Spheres;

typedef struct {
	int lightCount;
	int insertPoint;
	Light *lights[];
} Lights;

typedef struct {
	Color *ambient;
	Spheres *spheres;
	Lights *lights;
	int enableAmbient;
	int enableDiffuse;
	int enableSpecular;
	int enableShadow;
} Scene;

Color* getPixalOfFunkyTexture(float x, float y, float x_size, float y_size);

Scene* newScene(int sphereCount, int lightCount, float r, float g, float b, int eAmbient, int eDiffuse, int eSpecular, int eShadow);
void destroyScene(Scene *scene);

void destroyColor(Color *color);

void addSphere(int x, int y, int z, int rad, float kd, float ka, float ks, float se, float r, float g, float b, Scene *scene);
void addLight(int x, int y, int z, float r, float g, float b, Scene *scene);

static void multiplyColor(Color *colorOne, Color *colorTwo, Color *out);
static void addColor(Color *colorOne, Color *colorTwo, Color *out);

Color* rayGetPixal(int x, int y, int silhoutette, Scene *scene);
static void setShader(float x, float y, float z, float dx, float dy, float dz, float t, float kd, float ka, float ks, float se, Color *color, Sphere *sphere, Scene *scene);
static float discriminant(float x, float y, float z, float cx, float cy, float cz, float cr, float dx, float dy, float dz);
static float distance(float x, float y, float z, float cx, float cy, float cz, float cr, float dx, float dy, float dz);
static int checkIfIntersection(int x, int y, int z, int dx, int dy, int dz, Sphere *sphereCheck, Scene *scene);
static float clampf(float a);
static Color* clamp(Color *a);
