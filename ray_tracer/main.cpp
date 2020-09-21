#define SDL_MAIN_HANDLED

#include <math.h>
#include <vector>
#include <SDL.h>

struct Vec3
{
	double x, y, z;
	Vec3(double x, double y, double z) :
		x(x), y(y), z(z) {}

	Vec3 operator+(const Vec3& v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	Vec3 operator-(const Vec3& v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3 operator*(double mul) const
	{
		return Vec3(x * mul, y * mul, z * mul);
	}

	Vec3 operator/(double div) const
	{
		return Vec3(x / div, y / div, z / div);
	}

	bool operator==(const Vec3& v1)
	{
		if (v1.x == x && v1.y == y && v1.z == z)
			return true;

		return false;
	}

	Vec3 Normalize() const
	{
		double magnitude = sqrt(x * x + y * y + z * z);
		return Vec3(x / magnitude, y / magnitude, z / magnitude);
	}
};

inline double dot(const Vec3& vec1, const Vec3& vec2)
{
	return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
}

struct Ray
{
	Vec3 origin, destination;
	Ray(const Vec3& org, const Vec3& dest) :
		origin(org), destination(dest) {}
};

struct Sphere
{
	Vec3 center;
	double radius;
	Vec3 clr;

	Sphere(const Vec3& ctr, double rad, Vec3& clr) :
		center(ctr), radius(rad), clr(clr) {}

	Vec3 GetNormal(const Vec3& pi) const
	{
		return (pi - center) / radius;
	}

	bool Intersects(const Ray& ray, double& t) const
	{
		const Vec3 origin = ray.origin;
		const Vec3 dest = ray.destination;
		const Vec3 orgCtr = origin - this->center;
		const double b = 2 * dot(orgCtr, dest);
		const double center = dot(orgCtr, orgCtr) - radius * radius;
		double discrim = b * b - 4 * center;

		if (discrim < 1e-4)
			return false;

		discrim = sqrt(discrim);
		const double t0 = -b - discrim;
		const double t1 = -b + discrim;

		t = (t0 < t1) ? t0 : t1;

		return true;
	}
};

void PixelShader(Vec3& clr)
{
	clr.x = (clr.x > 255) ? 255 : (clr.x < 0) ? 0 : clr.x;
	clr.y = (clr.y > 255) ? 255 : (clr.y < 0) ? 0 : clr.y;
	clr.z = (clr.z > 255) ? 255 : (clr.z < 0) ? 0 : clr.z;
}

int main()
{
	const int W = 250;
	const int H = 250;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* win = 
		SDL_CreateWindow(
		"ray tracer", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		W, H, SDL_WINDOW_SHOWN);

	SDL_Renderer* renderer = 
		SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	//SDL_RenderSetLogicalSize(renderer, W*2, H*2);

	Vec3 white = { 255, 255, 255 };
	Vec3 black = { 0, 0, 0 };
	Vec3 red = { 255, 0, 110 };
	Vec3 green = { 110, 255,0 };
	Vec3 blue = { 0, 110, 255 };

	Sphere sphere(Vec3(W * 0.5, H * 0.5, 0), 15, red);
	Sphere sphere2(Vec3(W * 0.25, H * 0.25, 0), 30, green);
	Sphere sphere3(Vec3(W * 0.75, H * 0.75, 0), 45, blue);
	
	Sphere light(Vec3(W * 0.5, H * 0.5, -20), 40, white);
	Sphere lightBulb(Vec3(W * 0.5, H * 0.5, 60), 40, white);

	std::vector<Sphere> objList = { lightBulb, sphere, sphere2, sphere3 };

	double t, t2;
	Vec3 pixclr = black;
	double lightIntensity = 0.5;
	double theta = 0.0;

	bool isRunning = true;
	while (isRunning)
	{
		SDL_Event event;
		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT)
			isRunning = false;

		for (int y = 0; y < H; ++y)
			for (int x = 0; x < W; ++x)
			{
				pixclr = black;
				Vec3 pixclrBounce = black;

				const Ray ray(Vec3(x, y, 0), Vec3(0, 0, 1));
				for (auto& sphere : objList)
				{
					if (sphere.Intersects(ray, t))
					{
						const Vec3 pointOfIntersection = ray.origin + ray.destination * t;
						const Vec3 len = light.center - pointOfIntersection;
						const Vec3 normal = sphere.GetNormal(pointOfIntersection);
						const double dt = dot(len.Normalize(), normal.Normalize());

						pixclr = (sphere.clr + light.clr * dt) * lightIntensity;

						//bounce
						for (auto& sphereB : objList)
						{
							const Ray ray2(pointOfIntersection, Vec3(0, 0, 1));
							if (sphereB.Intersects(ray2, t2))
							{
								if (sphereB.clr == white)
									continue;

								const Vec3 pointOfIntersection2 = ray2.origin + ray2.destination * t2;
								const Vec3 len2 = light.center - pointOfIntersection2;
								const Vec3 normal2 = sphereB.GetNormal(pointOfIntersection2);
								const double dt2 = dot(len2.Normalize(), normal2.Normalize());

								pixclrBounce = (sphereB.clr * dt2) * lightIntensity;

								pixclr = pixclr + pixclrBounce;
							}
						}

						PixelShader(pixclr);
					}
				}

				SDL_SetRenderDrawColor(renderer, (int)pixclr.x, (int)pixclr.y, (int)pixclr.z, 255);
				SDL_RenderDrawPoint(renderer, x, y);
			}

		objList[0].center.x = W/2 + (W/4) * sin(theta);
		objList[0].center.y = H/2 + (H/4) * cos(theta/2);

		light.center.x = W / 2 + (W / 4) * sin(theta);
		light.center.y = H / 2 + (H / 4) * cos(theta / 2);

		//light.clr.x *= sin(theta);
		//PixelShader(light.clr);

		SDL_RenderPresent(renderer);

		theta += 0.1;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
};