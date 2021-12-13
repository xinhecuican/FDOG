// testSift.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "opencv2/core/core.hpp"//因为在属性中已经配置了opencv等目录，所以把其当成了本地目录一样
//#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <iostream>

#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"  

#include "imatrix.h"
#include "ETF.h"
#include "fdog.h"
#include "myvec.h"

using namespace cv;
using namespace std;

void dogFilter(Mat grayImage);
void operator_init(imatrix& prewitt_v, imatrix& prewitt_h, imatrix& sobel_v, imatrix& sobel_h);
void prewitt(imatrix& img, imatrix& ans);
void sobel(imatrix& img, imatrix& ans);
void canny(imatrix& img, imatrix& ans);
imatrix prewitt_v(3, 3);
imatrix prewitt_h(3, 3);
imatrix sobel_v(3, 3);
imatrix sobel_h(3, 3);
imatrix across_left(3, 3);
imatrix across_right(3, 3);

int main(int argc, char* argv[])
{
	string imgPath;
	double tao = 0.99;
	double thres = 0.7;
	double sigma = 1.0;
	double sigma3 = 3.0;

	int wz = 15;
	double colSig = 15.0;
	double spaSig = 10.0;
	int iterFDoG = 2;

	//cout << argc << endl;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			cout << argv[i + 1] << endl;
			imgPath = argv[++i];
			continue;
		}
		if (strcmp(argv[i], "-sm") == 0)
		{
			sigma = atof(argv[++i]);
			continue;
		}
		if (strcmp(argv[i], "-sm3") == 0)
		{
			sigma3 = atof(argv[++i]);
			continue;
		}
		if (strcmp(argv[i], "-to") == 0)
		{
			tao = atof(argv[++i]);
			continue;
		}
		if (strcmp(argv[i], "-th") == 0)
		{
			thres = atof(argv[++i]);
			continue;
		}
	}
	cout << imgPath << "   " << endl;
	Mat image;
	Mat grayImage;
	if (!imgPath.empty())
	{
		image = imread(imgPath, 1);
		grayImage = imread(imgPath, 0);
		if (image.empty() || grayImage.empty())
		{
			cout << "Read image error!";
			return -1;
		}
	}

	//bilateral filtering
	Mat filterImg;
	//color sigma对降噪起了作用
	bilateralFilter(grayImage, filterImg, wz, colSig, spaSig);
	/*imshow("Edge Image", grayImage);
	imshow("bilateralImage", filterImg);
	waitKey(0);*/
	grayImage = filterImg;

	cout << "rows: " << image.rows << "   " << "cols: " << image.cols << endl;
	imatrix img;
	imatrix prewitt_img(image.rows, image.cols);
	imatrix sobel_img(image.rows, image.cols);
	imatrix canny_img(image.rows, image.cols);
	int index;

	// 2014-5-6 flag=1 use a,b   flag=0, use I
	img.init(image.rows, image.cols);

	for (int i = 0; i < image.rows; i++) {
		img.p[i] = new int[image.cols];
		prewitt_img.p[i] = new int[image.cols];
		sobel_img.p[i] = new int[image.cols];
		canny_img.p[i] = new int[image.cols];
		for (int j = 0; j < image.cols; j++) {
			index = i * image.cols + j;
			//printf("1: %d\n", grayImage.data[index]);
			img.p[i][j] = grayImage.data[index];
			prewitt_img.p[i][j] = grayImage.data[index];
			sobel_img.p[i][j] = grayImage.data[index];
			canny_img.p[i][j] = grayImage.data[index];
			//printf("2: %d\n", img.p[i][j]);
		}
	}

	int image_x = img.getRow();
	int image_y = img.getCol();
	ETF e;
	e.init(image_x, image_y);
	e.set(img); // get gradients from input image
	//e.set2(img); // get gradients from gradient map
	e.Smooth(4, 2);

	GetFDoG(img, e, sigma, sigma3, tao, false);

	//by me 2014-5-6
	//Iterative FDoG
	int pxVal;
	for (int i = 0; i < iterFDoG; i++)
	{
		for (int j = 0; j < img.getRow(); j++)
		{
			for (int k = 0; k < img.getCol(); k++)
			{
				pxVal = (int)img[j][k] + (int)(grayImage.data[j * img.getCol() + k]);//加强边缘 
				if (pxVal > 255)
					pxVal = 255;
				img[j][k] = pxVal;
			}
		}
		GetFDoG(img, e, sigma, sigma3, tao, false);
	}
	GetFDoG(img, e, sigma, sigma3, tao, true);
	GrayThresholding(img, thres);

	/* 算子初始化 */
	operator_init(prewitt_v, prewitt_h, sobel_v, sobel_h);
	//prewitt
	imatrix prewitt_ans(image.rows, image.cols);
	prewitt(prewitt_img, prewitt_ans);
	//sobel
	imatrix sobel_ans(image.rows, image.cols);
	sobel(sobel_img, sobel_ans);
	//canny
	imatrix canny_ans(image.rows, image.cols);
	canny(canny_img, canny_ans);

	Mat saveImg(image.rows, image.cols, grayImage.type());
	Mat prewitt_save(image.rows, image.cols, grayImage.type());
	Mat sobel_save(image.rows, image.cols, grayImage.type());
	Mat canny_save(image.rows, image.cols, grayImage.type());
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			index = i * image.cols + j;
			saveImg.data[index] = img.p[i][j];
			prewitt_save.data[index] = prewitt_ans.p[i][j];
			sobel_save.data[index] = sobel_ans.p[i][j];
			canny_save.data[index] = canny_ans.p[i][j];
		}
	}
	/*imshow("Edge Image", saveImg);
	waitKey(0);*/
	int lastSlashPos = 0;
	string str = "\\";
	for (int i = imgPath.length() - 1; i >= 0; i--)
	{
		if (imgPath[i] == str[0])
		{
			lastSlashPos = i;
			break;
		}
	}

	string basePath = imgPath.substr(0, lastSlashPos);
	string imgName = imgPath.substr(lastSlashPos, imgPath.length() - basePath.length() - 4);
	string ext = imgPath.substr(imgPath.length() - 4, 4);

	string savePath = basePath + imgName + "_fdog_edge" + ext;
	string prewitt_path = basePath + imgName + "_prewitt_edge" + ext;
	string sobel_path = basePath + imgName + "_sobel_edge" + ext;
	string canny_path = basePath + imgName + "_canny_edge" + ext;

	/*cout << "basePath: " << basePath << endl;
	cout << "ImageName: " << imgName << endl;
	cout << "ext: " << ext << endl;
	cout << "output path: " << savePath << endl;*/
	imwrite(savePath, saveImg);
	imwrite(prewitt_path, prewitt_save);
	imwrite(sobel_path, sobel_save);
	imwrite(canny_path, canny_save);
	//system("pause");
	return 0;
}


void dogFilter(Mat grayImage)
{
	// DoG
	Mat img_G0;
	Mat img_G1;
	Mat img_DoG;
	Mat tmpImg1;
	tmpImg1 = grayImage;
	GaussianBlur(tmpImg1, img_G0, Size(5, 5), 1, 1);
	GaussianBlur(tmpImg1, img_G1, Size(5, 5), 7, 7);
	img_DoG = img_G0 - img_G1;
	normalize(img_DoG, img_DoG, 255, 0, CV_MINMAX);

	imshow("Blur1", img_G0);
	imshow("Blur2", img_G1);
	imshow("DoG", img_DoG);
	waitKey(0);
	/*imwrite("E:\\ori.jpg", grayImage);
	imwrite("E:\\blur1.jpg", img_G0);
	imwrite("E:\\blur2.jpg", img_G1);*/
	imwrite("E:\\DoG.jpg", img_DoG);
}

void operator_init(imatrix& prewitt_v, imatrix& prewitt_h, imatrix& sobel_v, imatrix& sobel_h)
{
	int a[3][3] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			prewitt_v.p[i][k] = a[i][k];
		}
	}
	int b[3][3] = { 1, 1, 1, 0, 0, 0, -1, -1, -1 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			prewitt_h.p[i][k] = b[i][k];
		}
	}
	int c[3][3] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			sobel_v.p[i][k] = c[i][k];
		}
	}
	int d[3][3] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			sobel_h.p[i][k] = d[i][k];
		}
	}
	int e[3][3] = { 1, 1, 0, 1, 0, -1, 0, -1, -1 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			across_left[i][k] = e[i][k];
		}
	}
	int f[3][3] = { 0, 1, 1, -1, 0, 1, -1, -1, 0 };
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			across_right[i][k] = f[i][k];
		}
	}
}

void mask_image(imatrix& img, imatrix& mask, imatrix& ans)
{
	int ir = img.getRow();
	int ic = img.getCol();
	int mr = mask.getRow() / 2;
	int mc = mask.getCol() / 2;
	for (int i = 1; i < ir - 1; i++)
	{
		for (int k = 1; k < ic - 1; k++)
		{
			int sum = 0;
			for (int m = -mr; m <= mr; m++)
			{
				for (int n = -mc; n <= mc; n++)
				{
					sum += img.p[i+m][k+n] * mask.p[m + mr][n + mc];
				}
			}
			ans.p[i][k] = sum;
		}
	}
	for (int i = 1; i <= ir - 2; i++) {
		ans.p[i][0] = ans.p[i][1];
		ans.p[i][ic - 1] = ans.p[i][ic - 2];
	}

	for (int j = 1; j <= ic - 2; j++) {
		ans.p[0][j] = ans.p[1][j];
		ans.p[ir - 1][j] = ans.p[ir - 2][j];
	}

	ans.p[0][0] = (ans.p[0][1] + ans.p[1][0]) / 2;
	ans.p[0][ic - 1] = (ans.p[0][ic - 2] + ans.p[1][ic - 1]) / 2;
	ans.p[ir - 1][0] = (ans.p[ir - 1][1] + ans.p[ir - 2][0]) / 2;
	ans.p[ir - 1][ic - 1] = (ans.p[ir - 1][ic - 2] + ans.p[ir - 2][ic - 1]) / 2;
}

void prewitt(imatrix& img, imatrix& ans)
{
	int ir = img.getRow();
	int ic = img.getCol();
	imatrix v_ans(ir, ic);
	imatrix h_ans(ir, ic);
	imatrix leftup_ans(ir, ic);
	imatrix rightup_ans(ir, ic);
	mask_image(img, prewitt_h, h_ans);
	mask_image(img, prewitt_v, v_ans);
	mask_image(img, across_left, leftup_ans);
	mask_image(img, across_right, rightup_ans);
	for (int i = 0; i < ir; i++)
	{
		for (int k = 0; k < ic; k++)
		{
			if (h_ans.p[i][k] > 48 || v_ans.p[i][k] > 48 || leftup_ans[i][k] > 48 || rightup_ans[i][k] > 48)
			{
				ans[i][k] = 0;
			}
			else
			{
				ans[i][k] = 255;
			}
		}
	}
}
void sobel(imatrix& img, imatrix& ans)
{
	int ir = img.getRow();
	int ic = img.getCol();
	imatrix v_ans(ir, ic);
	imatrix h_ans(ir, ic);
	imatrix leftup_ans(ir, ic);
	imatrix rightup_ans(ir, ic);
	mask_image(img, sobel_h, h_ans);
	mask_image(img, sobel_v, v_ans);
	mask_image(img, across_left, leftup_ans);
	mask_image(img, across_right, rightup_ans);
	for (int i = 0; i < ir; i++)
	{
		for (int k = 0; k < ic; k++)
		{
			if (h_ans.p[i][k] > 48 || v_ans.p[i][k] > 48 || leftup_ans[i][k] > 48 || rightup_ans[i][k] > 48)
			{
				ans[i][k] = 0;
			}
			else
			{
				ans[i][k] = 255;
			}
		}
	}
}

void canny(imatrix& img, imatrix& ans)
{
	int ir = img.getRow();
	int ic = img.getCol();
	imatrix v_ans(ir, ic);
	imatrix h_ans(ir, ic);
	mask_image(img, sobel_h, h_ans);
	mask_image(img, sobel_v, v_ans);
	imatrix g(ir, ic, imatrix::FLOAT);
	imatrix g2(ir, ic, imatrix::FLOAT);
	for (int i = 0; i < ir; i++)
	{
		for (int k = 0; k < ic; k++)
		{
			g.f[i][k] = sqrt(v_ans.p[i][k] * v_ans.p[i][k] + h_ans.p[i][k] * h_ans.p[i][k]);
		}
	}

	//非极大值抑制
	float max_g = 0;
	for (int i = 1; i < ir - 1; i++)
	{
		for (int k = 1; k < ic - 1; k++)
		{
			float p1 = 0;
			float p2 = 0;
			if (h_ans.p[i][k] == 0)
			{
				p1 = g.f[i - 1][k];
				p2 = g.f[i + 1][k];
			}
			else
			{
				float tan_theta = v_ans.p[i][k] / h_ans.p[i][k];
				float tan_theta2 = tan_theta != 0 ? 1 / tan_theta : 0;

				if (tan_theta >= 0 && tan_theta <= 1)
				{
					p1 = (1 - tan_theta2) * g.f[i - 1][k] + tan_theta2 * g.f[i - 1][k + 1];
					p2 = (1 - tan_theta2) * g.f[i + 1][k] + tan_theta2 * g.f[i + 1][k - 1];
					
				}
				else if (tan_theta > 1)
				{
					p1 = (1 - tan_theta) * g.f[i][k + 1] + tan_theta * g.f[i - 1][k + 1];
					p2 = (1 - tan_theta) * g.f[i][k - 1] + tan_theta * g.f[i + 1][k - 1];
				}
				else if (tan_theta<0 && tan_theta > -1)
				{
					p1 = (1 + tan_theta2) * g.f[i - 1][k] - tan_theta2 * g.f[i - 1][k - 1];
					p2 = (1 + tan_theta2) * g.f[i + 1][k] - tan_theta2 * g.f[i + 1][k + 1];
					
				}
				else
				{
					p1 = (1 + tan_theta) * g.f[i][k + 1] - tan_theta * g.f[i + 1][k + 1];
					p2 = (1 + tan_theta) * g.f[i - 1][k] - tan_theta * g.f[i - 1][k - 1];
				}
			}

			if (g.f[i][k] < p1 || g.f[i][k] < p2)
			{
				g2.f[i][k] = 0;
			}
			else
			{
				g2.f[i][k] = g.f[i][k];
				if (g.f[i][k] > max_g)
				{
					max_g = g.f[i][k];
				}
			}
		}
	}

	//双阈值检测
	float tl = 0.1f * max_g;
	float th = 0.12f * max_g;
	for (int i = 0; i < ir ; i++)
	{
		for (int k = 0; k < ic ; k++)
		{
			if (g2.f[i][k] < tl)
			{
				g2.f[i][k] = 0;
			}
			else if (g2.f[i][k] < th)
			{
				g2.f[i][k] = 2;
			}
			else
			{
				g2.f[i][k] = 8;
			}
		}
	}

	//抑制孤立点
	for (int i = 1; i < ir - 1; i++)
	{
		for (int k = 1; k < ic - 1; k++)
		{
			if (g2.f[i][k] > 1 && g2.f[i][k] < 4)
			{
				bool success = false;
				for (int m = -1; m <= 1; m++)
				{
					for (int n = -1; n <= 1; n++)
					{
						if (g2.f[i + m][k + n] > 4)
						{
							success = true;
							goto OUT;
						}
					}
				}
OUT:;
				if (!success)
				{
					g2.f[i][k] = 0;
				}
			}
		}
	}

	for (int i = 0; i < ir; i++)
	{
		for (int k = 0; k < ic; k++)
		{
			if (g2.f[i][k] > 1)
			{
				ans.p[i][k] = 0;
			}
			else
			{
				ans.p[i][k] = 255;
			}
		}
	}
}