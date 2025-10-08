#include "ExposureFusion.h"



void ExposureFusion::QualityMeasuresProcessing()
{
	Mat weightMap;
	Mat contrast;
	Mat saturation;
	Mat wellexposedness;
	Mat originalGray;
	Mat originalColor;
	time_t tok, tic = clock();
	for (int nfrm = 0; nfrm < nframes; nfrm++)
	{
		cout << "Quality measure processing - Frame number: " << nfrm + 1<<std::endl;
		
		QualityMeasures *qm = new QualityMeasures(inputImages[nfrm].clone(), inputGrayImages[nfrm].clone());
		
		WeightMaps.push_back(qm->getterWeightMap());
		delete(qm);
	}
	tok = clock();
	cout << endl << "processing time of QualitymeasureProcessing: " << (float)(tok - tic) / CLOCKS_PER_SEC << endl;
	cout << endl;
}

void ExposureFusion::FusionProcessing()
{
	int nframes = getnframes();
	int rows = inputImages[0].rows;
	int cols = inputImages[0].cols;
	int pyramidDepth = 4;
	
	setNormalizedWeightMaps();//获取每个输入图像的归一化权重
	
	vector<Mat> bgr;
	
	for (int i = 0; i < 3; i++)
	{
		bgr.push_back(setResultByPyramid(i));		
	}

	Mat dst;
	merge(bgr, dst);
	resultimage = dst.clone();
}

void ExposureFusion::setNormalizedWeightMaps()
{
	int nframes = getnframes();
	int rows = inputImages[0].rows;
	int cols = inputImages[0].cols;

	float sumPix = 0;
#if MODE==GRAY	
	for (int nfrm = 0; nfrm < nframes; nfrm++)
	{
		Mat NorWeightMap(inputImages[0].rows, inputImages[0].cols, CV_32FC1);
		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				sumPix = 0;
				
				for (int n = 0; n < nframes; n++)
				{
					sumPix += WeightMaps[n].at<float>(y, x);
				}
				NorWeightMap.at<float>(y, x) = WeightMaps[nfrm].at<float>(y, x) / sumPix;
			}
		}
		NorWeightMaps.push_back(NorWeightMap);
	}	
#endif
}

Mat ExposureFusion::setResultByPyramid(int nch)
{
	int pyramidDepth = 4;
	vector<Mat> gaussianPyramid;
	vector<vector<Mat>> gaussianWeightMapPyramid;
	vector<vector<Mat>> laplacianImagePyramid;
	vector<Mat> fusedPyramid;
	vector<vector<Mat>> fusedPyramidColor;
	int nframes = getnframes();
	Mat src;
	Mat ucharMap;
	Mat laplimg;
	Mat upGauss;
	Mat cuGauss;
	Mat pvGauss;
	Mat laplacianResult;
	Mat fuseimg;
	Mat rsLaplac;
	Mat result;
	float pix = 0;
		
#if MODE==GRAY
	vector<Mat> BGR;
	for (int nfrm = 0; nfrm < nframes; nfrm++)
	{
		split(inputImages[nfrm], BGR);
		src = BGR[nch].clone();
		laplacianImagePyramid.push_back(vector<Mat>());
		gaussianWeightMapPyramid.push_back(vector<Mat>());
		buildPyramid(src, gaussianPyramid, pyramidDepth);		
		
		ucharMap = Mat(NorWeightMaps[nfrm].size(), CV_8UC1);
		for (int y = 0; y < ucharMap.rows; y++)
		{
			for (int x = 0; x < ucharMap.cols; x++)
			{
				pix = NorWeightMaps[nfrm].at<float>(y, x) * 255;
				pix = (pix > 255) ? 255 : pix;
				pix = (pix < 0) ? 0 : pix;
				ucharMap.at<uchar>(y, x) = (uchar)pix;
			}
		}
		buildPyramid(ucharMap, gaussianWeightMapPyramid[nfrm], pyramidDepth);

		for (int i = 1; i < gaussianPyramid.size(); i++)
		{
			Mat prev = gaussianPyramid[i-1].clone();
			Mat crnt = gaussianPyramid[i].clone();
			

			pyrUp(crnt, crnt, prev.size());
			laplimg = Mat(prev.size(), CV_8SC1);

			for (int y = 0; y < prev.rows; y++)
			{
				for (int x = 0; x < prev.cols; x++)
				{
					laplimg.at<char>(y, x) = prev.at<uchar>(y, x) - crnt.at<uchar>(y, x);
				}
			}
			
			laplacianImagePyramid[nfrm].push_back(laplimg.clone());
			prev.release();
			crnt.release();
			laplimg.release();
		}

		laplacianImagePyramid[nfrm].push_back(gaussianPyramid[pyramidDepth].clone());//拉普拉斯金字塔最后一层=高斯金字塔的最后一层
		ucharMap.release();
	}
	cout << "Set laplacian image pyramid " << endl << "Set gaussian weight map pyramid" << endl;//构建指定图像通道的拉普拉斯金字塔与对应图像权重的高斯金字塔


	for(int l=0; l<pyramidDepth; l++)	
	{
		//cout << "pyramid depth: " << l << endl;
		laplacianResult = Mat(laplacianImagePyramid[0][l].size(), CV_32SC1);
		for(int y = 0; y < laplacianResult.rows; y++)
		{
			for (int x = 0; x < laplacianResult.cols; x++)
			{
				pix = 0;
				for (int nfrm = 0; nfrm < nframes; nfrm++)
				{
					pix += gaussianWeightMapPyramid[nfrm][l].at<uchar>(y, x) * laplacianImagePyramid[nfrm][l].at<char>(y, x);
				} 
				laplacianResult.at<int>(y, x) = pix/255;
			}
		}
		fusedPyramid.push_back(laplacianResult.clone());
		laplacianResult.release();
	}


	laplacianResult = Mat(laplacianImagePyramid[0][pyramidDepth].size(), CV_8UC1);
	for (int y = 0; y < laplacianResult.rows; y++)
	{
		for (int x = 0; x < laplacianResult.cols; x++)
		{
			pix = 0;
			for (int nfrm = 0; nfrm < nframes; nfrm++)
			{
				pix += gaussianWeightMapPyramid[nfrm][pyramidDepth].at<uchar>(y, x) * (laplacianImagePyramid[nfrm][pyramidDepth].at<uchar>(y, x));
			}
			
			laplacianResult.at<uchar>(y, x) = (uchar)(pix/255);	// �� uchar�̳� char�� �ϸ� �̻��ϰ� ������; int���� ���͵��� �� �����ѵ�
		}
	}
	fusedPyramid.push_back(laplacianResult.clone());
		
	cout << "Set fused pyramid" << endl;
	

	int ipix = 0;	
	Mat temp = fusedPyramid[pyramidDepth].clone();
	Mat fusedLaplacianImage = fusedPyramid[pyramidDepth-1].clone();
	int rows = fusedLaplacianImage.rows;
	int cols = fusedLaplacianImage.cols;

	Mat sumimg(Size(cols, rows), CV_8UC1);
	pyrUp(temp, temp, fusedLaplacianImage.size());
	
	for (int y = 0; y < temp.rows; y++)
	{
		for (int x = 0; x < temp.cols; x++)
		{
			ipix = temp.at<uchar>(y, x) + fusedLaplacianImage.at<int>(y, x);
			ipix = (ipix > 255) ? 255 : ipix;
			ipix = (ipix < 0) ? 0 : ipix;
			sumimg.at<uchar>(y, x) = (uchar)ipix;
		}
	}
	
	for (int i = pyramidDepth - 2; i >= 0; i--)
	{
		fusedLaplacianImage = fusedPyramid[i].clone();

		pyrUp(sumimg, sumimg, fusedLaplacianImage.size());
		for (int y = 0; y < sumimg.rows; y++)
		{
			for (int x = 0; x < sumimg.cols; x++)
			{
				ipix = fusedLaplacianImage.at<int>(y, x) + sumimg.at<uchar>(y, x);
				ipix = (ipix > 255) ? 255 : ipix;
				ipix = (ipix < 0) ? 0 : ipix;
				sumimg.at<uchar>(y,x) = (uchar)ipix;
			}
		}
	}
	
	result = sumimg.clone();
#endif

	return result;
}

void ExposureFusion::RobustNormalization()
{
	std::vector<int> Hist(256);
	float cut_percent = 0.01; // 1%

	int Size = resultimage.cols*resultimage.rows*resultimage.channels();
	unsigned char * ptr = resultimage.data;
	for (int i = 0; i < Size; i++)
	{
		Hist[*ptr]++;
		ptr++;
	}
	
	int Nthr = cut_percent * Size;
	unsigned char pin_min=0, pin_max=0;
	int sum_min=0, sum_max = 0;

	for (int i = 0; i < 255; i++)
	{
		if (pin_min == 0) {
			sum_min += Hist[i];
			if (sum_min >= Nthr)
				pin_min = i;
		}
		if (pin_max == 0) {
			sum_max += Hist[Hist.size() - i - 1];
			if (sum_max >= Nthr)
				pin_max = Hist.size() - i - 1;
		}

	}

	float dt = pin_max - pin_min;
	if (dt < 0.001)dt = 0.001;
	float k = 256.0f / dt;
	float b = 0 -(pin_min*k);

	for (int i = 0; i < Size; i++)
	{
		int pix = (*(resultimage.data + i)) * k + b;
		if (pix > 255) *(resultimage.data + i) = 255;
		else if (pix < 0) *(resultimage.data + i) = 0;
		else *(resultimage.data + i) = pix;

	}
}