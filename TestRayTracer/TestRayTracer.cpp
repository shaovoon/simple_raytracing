//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <iostream>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "RandomNumGen.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iomanip>
#include <chrono>
#include <string>
#include <Windows.h>
#include <ppl.h>
#include <Gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
bool SavePngFile(const wchar_t* pszFile, Gdiplus::Bitmap& bmp);

class timer
{
public:
	timer() = default;
	void start(const std::string& text_)
	{
		text = text_;
		begin = std::chrono::high_resolution_clock::now();
	}
	void stop()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		std::cout << std::setw(20) << text << " timing:" << std::setw(5) << ms << "ms" << std::endl;
	}

private:
	std::string text;
	std::chrono::high_resolution_clock::time_point begin;
};

vec3 color(const ray& r, hitable* world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}


hitable* random_scene() {
	int n = 500;
	hitable** list = new hitable * [n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = RandomNumGen::GetRand();
			vec3 center(a + 0.9 * RandomNumGen::GetRand(), 0.2, b + 0.9 * RandomNumGen::GetRand());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(RandomNumGen::GetRand() * RandomNumGen::GetRand(), RandomNumGen::GetRand() * RandomNumGen::GetRand(), RandomNumGen::GetRand() * RandomNumGen::GetRand())));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5 * (1 + RandomNumGen::GetRand()), 0.5 * (1 + RandomNumGen::GetRand()), 0.5 * (1 + RandomNumGen::GetRand())), 0.5 * RandomNumGen::GetRand()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

int main() {
	int nx = 256;
	int ny = 256;
	int ns = 500;
	//std::cout << "P3\n" << nx << " " << ny << "\n255\n";
	hitable* list[5];
	float R = cos(M_PI / 4);
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	hitable* world = new hitable_list(list, 5);
	//world = random_scene();

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float dist_to_focus = 10.0;
	float aperture = 0.1;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);

	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken = NULL;

	using namespace Gdiplus;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
	{
		Bitmap bmp(nx, ny, PixelFormat32bppARGB);
		BitmapData bitmapData;
		Rect rect(0, 0, bmp.GetWidth(), bmp.GetHeight());

		bmp.LockBits(
			&rect,
			ImageLockModeRead,
			PixelFormat32bppARGB,
			&bitmapData);

		UINT* pixelsSrc = (UINT*)bitmapData.Scan0;

		if (!pixelsSrc)
			return false;

		int stride = bitmapData.Stride >> 2;

		timer stopwatch;
		stopwatch.start("ray_tracer");
		using namespace Concurrency;
		parallel_for(0, ny, [&](int j)
			//for (int j = ny - 1; j >= 0; j--) 
			{
				for (int i = 0; i < nx; i++)
				{
					vec3 col(0, 0, 0);
					for (int s = 0; s < ns; s++) {
						float u = float(i + RandomNumGen::GetRand()) / float(nx);
						float v = float(j + RandomNumGen::GetRand()) / float(ny);
						ray r = cam.get_ray(u, v);
						col += color(r, world, 0);
					}
					col /= float(ns);
					col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
					int ir = int(255.99 * col[0]);
					int ig = int(255.99 * col[1]);
					int ib = int(255.99 * col[2]);

					int index = ((ny - 1) - j) * stride + i;
					pixelsSrc[index] = (0xff000000 | (ir << 16) | (ig << 8) | ib);

					//std::cout << ir << " " << ig << " " << ib << "\n";
				}
				//float complete = ((ny - 1) - j) / (float)ny;
				//std::cout << "Complete:" << (complete * 100.0f) << std::endl;
			});

		stopwatch.stop();

		for (int j = ny - 1; j >= 0; j--) 
		{
			for (int i = 0; i < nx; i++)
			{
				int index = ((ny - 1) - j) * stride + i;

				unsigned char ir = (pixelsSrc[index] & 0xff0000) >> 16;
				unsigned char ig = (pixelsSrc[index] & 0xff00) >> 8;
				unsigned char ib = (pixelsSrc[index] & 0xff);
				pixelsSrc[index] = (0xff000000 | (ib << 16) | (ig << 8) | ir);

				//std::cout << ir << " " << ig << " " << ib << "\n";
			}
			//float complete = ((ny - 1) - j) / (float)ny;
			//std::cout << "Complete:" << (complete * 100.0f) << std::endl;
		}

		stbi_write_png("c:\\temp\\ray_trace_stb_sample10.png", nx, ny, 4, bitmapData.Scan0, bitmapData.Stride);

		bmp.UnlockBits(&bitmapData);

		SavePngFile(L"c:\\temp\\ray_trace_sample10.png", bmp);
	}

	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	using namespace Gdiplus;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return false;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return false;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return true;  // Success
		}
	}

	free(pImageCodecInfo);
	return false;  // Failure
}

bool SavePngFile(const wchar_t* pszFile, Gdiplus::Bitmap& bmp)
{
	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	Gdiplus::Status status = bmp.Save(pszFile, &pngClsid, NULL);
	return status == Gdiplus::Ok ? true : false;
}
