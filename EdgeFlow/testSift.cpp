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

int main(int argc, char* argv[])
{
	string imgPath;
	double tao = 0.99;
	double thres = 0.7;
	double sigma = 1.0;
	double sigma3 = 3.0;
	int flag = 0;

	int wz = 15;
	double colSig = 15.0;
	double spaSig = 10.0;
	int iterFDoG = 2;

	//cout << argc << endl;
	for(int i=1; i<argc; i++)
	{
		if( strcmp(argv[i], "-i") ==0 )
		{
			imgPath = argv[++i];
			continue;
		}
		if( strcmp(argv[i], "-sm") ==0 )
		{
			sigma = atof(argv[++i]);
			continue;
		}
		if( strcmp(argv[i], "-sm3") ==0 )
		{
			sigma3 = atof(argv[++i]);
			continue;
		}
		if( strcmp(argv[i], "-to") ==0 )
		{
			tao = atof(argv[++i]);
			continue;
		}
		if( strcmp(argv[i], "-th") ==0 )
		{
			thres = atof(argv[++i]);
			continue;
		}
		if( strcmp(argv[i], "-f") ==0 )
		{
			flag = atoi(argv[++i]);
			continue;
		}
	}
	cout << imgPath << "   " << endl;
	Mat image;
	Mat grayImage;
	if(!imgPath.empty())
	{
		image = imread(imgPath, 1);
		grayImage = imread(imgPath, 0);
		if( image.empty() || grayImage.empty())
		{
			cout << "Read image error!" ;
			return -1;
		}
	}
	
	// BGR2Lab 2014-5-5
	Mat labImage;
	cvtColor(image, labImage, CV_BGR2Lab);
	/*imshow("labImage", labImage);
	waitKey(0);*/
	


	//bilateral filtering
	Mat filterImg;
	//color sigma对降噪起了作用
	bilateralFilter(grayImage, filterImg, wz, colSig, spaSig);
	/*imshow("Edge Image", grayImage);
	imshow("bilateralImage", filterImg);
	waitKey(0);*/
	grayImage = filterImg;

   	cout<<"rows: "<<image.rows<<"   "<<"cols: "<<image.cols<<endl;
	imatrix img;
	int index;

	// 2014-5-6 flag=1 use a,b   flag=0, use I
	img.flag = flag;
	img.init(image.rows, image.cols);

	for (int i = 0; i < image.rows; i++) {
			img.p[i] = new int[image.cols];
			for (int j = 0; j < image.cols; j++) {
				index = i*image.cols+j;
				//printf("1: %d\n", grayImage.data[index]);
				img.p[i][j] = grayImage.data[index];
				//printf("2: %d\n", img.p[i][j]);
			}
	}
	// by me 2014-5-5
	int totalIndex = image.rows*image.cols;
	if(img.flag == 1)
	{
		for (int i = 0; i < image.rows; i++) {
				img.a[i] = new int[image.cols];
				for (int j = 0; j < image.cols; j++) {
					img.L[i][j] = labImage.at<Vec3b>(i, j)[0];//   (i, j)处的L分量
					img.a[i][j] = labImage.at<Vec3b>(i, j)[1];//   (i, j)处的a分量
					img.b[i][j] = labImage.at<Vec3b>(i, j)[2];//   (i, j)处的b分量
				}
		}
		Mat a_image=grayImage.clone();
		Mat b_image=grayImage.clone();
		for (int i = 0; i < image.rows; i++) {
				for (int j = 0; j < image.cols; j++) {
					a_image.data[i*image.cols+j] = img.a[i][j];
					b_image.data[i*image.cols+j] = img.b[i][j];
				}
		}
	}
	///////////
	
	/*imshow("A image", a_image);
	imshow("B image", b_image);
	waitKey(0);*/
	/*imwrite("E:\\a.bmp", a_image);
	imwrite("E:\\b.bmp", b_image);*/
	/////////


	int image_x = img.getRow();
	int image_y = img.getCol();
	ETF e;
	e.init(image_x, image_y);
	e.set(img); // get gradients from input image
	//e.set2(img); // get gradients from gradient map
	e.Smooth(4, 2);
	
	GetFDoG(img, e, sigma, sigma3, tao);

	//by me 2014-5-6
	
	int pxVal;
	for (int i = 0; i<iterFDoG; i++)
	{
		for(int j = 0; j < img.getRow(); j++)
		{
			for(int k = 0; k < img.getCol(); k++)
			{
				pxVal = (int)img[j][k] + (int)(grayImage.data[j*img.getCol()+k]);
				if(pxVal > 255)
					pxVal = 255;
				img[j][k] = pxVal;
			}
		}
		GetFDoG(img, e, sigma, sigma3, tao);
	}
	
	GrayThresholding(img, thres); 

	Mat saveImg = grayImage;
	for (int i = 0; i < image.rows; i++) {
			for (int j = 0; j < image.cols; j++) {
				index = i*image.cols+j;
				saveImg.data[index] = img.p[i][j];
			}
	}
	/*imshow("Edge Image", saveImg);
	waitKey(0);*/
	int lastSlashPos = 0;
	string str = "\\";
	for(int i=imgPath.length()-1; i>=0; i--)
	{
		if(imgPath[i] == str[0])
		{
			lastSlashPos = i;
			break;
		}
	}
	
	string basePath = imgPath.substr(0, lastSlashPos);
	string imgName = imgPath.substr(lastSlashPos, imgPath.length()-basePath.length()-4);
	string ext = imgPath.substr(imgPath.length()-4, 4);

	string savePath = basePath+ "\\result"+imgName+"_edge"+ext;

	/*cout << "basePath: " << basePath << endl;
	cout << "ImageName: " << imgName << endl;
	cout << "ext: " << ext << endl;
	cout << "output path: " << savePath << endl;*/
	imwrite(savePath, saveImg);
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
	GaussianBlur(tmpImg1,img_G0,Size(5,5),1, 1);
	GaussianBlur(tmpImg1,img_G1,Size(5,5),7, 7);
	img_DoG = img_G0  - img_G1;
	normalize(img_DoG,img_DoG,255,0,CV_MINMAX);  

	imshow("Blur1", img_G0);
	imshow("Blur2", img_G1);
	imshow("DoG", img_DoG);
	waitKey(0);
	/*imwrite("E:\\ori.jpg", grayImage);
	imwrite("E:\\blur1.jpg", img_G0);
	imwrite("E:\\blur2.jpg", img_G1);*/
	imwrite("E:\\DoG.jpg", img_DoG);
}

	//initModule_nonfree();  
	//Ptr<FeatureDetector> detector = FeatureDetector::create("SIFT");
	//Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create("SIFT");
	//Ptr<DescriptorMatcher> descriptor_macher = DescriptorMatcher::create("BruteForce");
	//if( detector.empty() || descriptor_extractor.empty() )
	//	cout<<"Fail to create detector";
	//string imgPath1;
	//string imgPath2;

	//cout << argc << endl;
	//for(int i=1; i<argc; i++)
	//{
	//	if( strcmp(argv[i], "-i") ==0 )
	//	{
	//		imgPath1 = argv[++i];
	//		imgPath2 = argv[++i];
	//		continue;
	//	}
	//}
	//cout << imgPath1 << "   " << endl;
	//if(!imgPath1.empty() && !imgPath2.empty() )
	//{
	//	Mat image1 = imread(imgPath1, 1);
	//	Mat image2 = imread(imgPath2, 1);
	//	if( image1.empty() || image2.empty() )
	//	{
	//		cout << "Read image error!" ;
	//		return -1;
	//	}
	//	//imshow("Image1", image1);//显示的标题为Matches
	//	//imshow("Image2", image2);
	//	//waitKey(0);

	//	double t = getTickCount();
	//	vector<KeyPoint> keypoints1, keypoints2;
	//	detector->detect(image1, keypoints1);
	//	detector->detect(image2, keypoints2);
	//	cout << "图像1特征点的个数：" << keypoints1.size() << endl;
	//	cout << "图像2特征点的个数：" << keypoints2.size() << endl;

	//	Mat descriptors1, descriptors2;
	//	descriptor_extractor->compute( image1, keypoints1, descriptors1 );
	//	descriptor_extractor->compute( image2, keypoints2, descriptors2 );
	//	t = ((double)getTickCount()-t)/getTickFrequency();
	//	cout << "SIFT算法用时：" << t << "秒" << endl;

	//	cout << "图像1特征描述矩阵大小：" << descriptors1.size() << "，特征向量个数：" << descriptors1.rows << "，维数" << descriptors1.cols << endl;
	//	cout << "图像2特征描述矩阵大小：" << descriptors2.size() << "，特征向量个数：" << descriptors2.rows << "，维数" << descriptors2.cols << endl;
	//
	//	Mat img_keypoints1, img_keypoints2;
	//	drawKeypoints(image1, keypoints1, img_keypoints1, Scalar::all(-1), 0);
	//	drawKeypoints(image2, keypoints2, img_keypoints2, Scalar::all(-1), 0);	
	//	
	//	vector<DMatch> matches; 
	//	descriptor_macher->match(descriptors1, descriptors2, matches);
	//	cout << "Match个数：" << matches.size() << endl;

	//	double max_dist = 0;
	//	double min_dist = 100;
	//	for(int i=0; i<matches.size(); i++)
	//	{
	//		double dist = matches[i].distance;
	//		if(dist < min_dist) min_dist = dist;
	//		if(dist > max_dist) max_dist = dist;
	//	}
	//	cout << "最大距离：" << max_dist << endl;
	//	cout << "最小距离：" << min_dist << endl;
	//	
	//	vector<DMatch> goodMatches;
	//	for(int i=0; i<matches.size(); i++)
	//	{
	//		if(matches[i].distance < 0.5*max_dist)
	//			goodMatches.push_back(matches[i]);
	//	}
	//	cout << "goodMatch个数" << goodMatches.size() << endl;

	//	Mat img_matches;
	//	drawMatches(image1, keypoints1, image2, keypoints2, goodMatches, img_matches,
	//		Scalar::all(-1), CV_RGB(255, 0, 0), Mat(), 2);

	//	imshow("MatchSIFT", img_matches);
	//	waitKey(0);
	//}

	//cout<<"hello";
	//system("pause");
	//Mat img1 = imread();


//	//cv::initModule_nonfree();
//	/*cvLoadimage("Filename",1);
//	cvshowImage("windowName",&image);*/
//	const char* pImg1 = "D:/ling31.jpg";
//	const char* pImg2 = "D:/ling32.jpg";
//
//	Mat image1 = imread(pImg1);  
//    Mat imageGray1 = imread(pImg1, 0);  
//	Mat image2 = imread(pImg2);  
//    Mat imageGray2 = imread(pImg2, 0);  
//
//
//	Mat descriptors1;  
//	Mat descriptors2;
//    vector<KeyPoint> keypoints1;  
//	vector<KeyPoint> keypoints2;  
//    // 新版本2.4.0方法  
//    
////  
//    Ptr<Feature2D> sift1 = Algorithm::create<Feature2D>("Feature2D.SIFT");  
//    sift1->set("contrastThreshold", 0.1f);//0.01f);  
//    (*sift1)(image1, noArray(), keypoints1, descriptors1);  
//    (*sift1)(image2, noArray(), keypoints2, descriptors2);  
////    // 2.3.1方法  
////  SiftFeatureDetector sift2(0.06f, 10.0);  
////  sift2.detect(imageGray, keypoints);  
////      
//    drawKeypoints(image1, keypoints1, image1, Scalar(255,0,0));  
//	drawKeypoints(image2, keypoints2, image2, Scalar(255,0,0));  
////  
//    imshow("test1", image1); 
//	imshow("test2", image2);  
//	//waitKey(0);
//	BruteForceMatcher<L2<float>>matcher;
//	vector<DMatch>matches;
//    matcher.match(descriptors1,descriptors2,matches);
//	Mat img_matches;
//    drawMatches(image1,keypoints1,image2,keypoints2,matches,img_matches);//将匹配出来的结果放入内存img_matches中
//
//    //显示匹配线段
//    imshow("sift_Matches",img_matches);//显示的标题为Matches
//
//	waitKey(0);
//	system("pause");


