#include "DXLib/Utilities.h"
#include <cv.h>
#include <highgui.h>
#include <vector>
#include <cmath>

using namespace SubSurfaceScatter;

int main(int argc, char *argv[]) {
	std::cout << "Loading normal map '" << argv[1] << "' and writing result to '" << argv[2] << "'." << std::endl;

	cv::Mat_<cv::Vec3b> Input = cv::imread(argv[1]);

	float r, g, b, tmp, length;
	for(int row = 0; row < Input.rows; ++row) {
		for(int col = 0; col < Input.cols; ++col) {
			// load
			b = Input(row, col)[0];
			g = Input(row, col)[1];
			r = Input(row, col)[2];
			// skip empty regions
			if(r == 0.0f && g == 0.0f && b == 0.0f)
				continue;
			// unpack
			r = 2.0f * r / 255.0f - 1.0f;
			g = 2.0f * g / 255.0f - 1.0f;
			b = 2.0f * b / 255.0f - 1.0f;
			// convert & pack
			// x = x
			// y = -z
			// z = y
			tmp = g;
			r = r;
			g = -b;
			b = tmp;

			// normalize (just to be save)
			length = std::sqrtf(r * r + g * g + b * b);
			r /= length;
			g /= length;
			b /= length;
			// pack & save (as bgr)
			Input(row, col)[0] = (uchar)((0.5f * b + 0.5f) * 255.0f);
			Input(row, col)[1] = (uchar)((0.5f * g + 0.5f) * 255.0f);
			Input(row, col)[2] = (uchar)((0.5f * r + 0.5f) * 255.0f);
		}
	}

	std::vector<int> params;
	params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	params.push_back(0);
	if(!cv::imwrite(argv[2], Input, params))
		std::cerr << "Error writing to file '" << argv[2] << "'" << std::endl;

	return 0;
}
