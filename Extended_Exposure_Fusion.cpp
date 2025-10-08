#include "ExposureFusion.h"
#include "Common.hpp"
using namespace std;
using namespace cv;

int main(void)
{
	std::string DataSetPath = "C:\\Users\\mk12878\\Desktop\\00000";
	std::vector<std::string> vDataSetFolds;
	if (GetFolds(DataSetPath, vDataSetFolds))
	{
		for (int i = 0; i < vDataSetFolds.size(); i++)
		{
			std::vector<std::string> vImageFile;
			if (GetFiles(DataSetPath+"\\"+vDataSetFolds[i], vImageFile))
			{
				for (int n = 0; n < vImageFile.size(); n++) {
					vImageFile[n] = DataSetPath + "\\" + vDataSetFolds[i] + "\\" + vImageFile[n];
					//if(vImageFile[n].find("E1000")!= vImageFile[n].npos)
					//{
					//	std::string commond=string_format("copy %s %s", vImageFile[n].c_str(), (DataSetPath + "\\").c_str());
					//	system("echo off");
					//	system(commond.c_str());
					//}

					//if (vImageFile[n].find("E200.") == vImageFile[n].npos  &&
					//	vImageFile[n].find("E500.") == vImageFile[n].npos &&
					//	vImageFile[n].find("E1000.") == vImageFile[n].npos &&
					//	vImageFile[n].find("E1500.") == vImageFile[n].npos)
					//{
					//	vImageFile.erase(vImageFile.begin() + n);
					//	n--;
					//}

				}
				
				KVLOG("EF Process %s", (DataSetPath + "\\" + vDataSetFolds[i]).c_str())
				ExposureFusion EF(vImageFile);
				time_t tok, tic = clock();
				EF.QualityMeasuresProcessing();
				KVLOG("EF>Finish to QualityMeasuresProcessing")
				EF.FusionProcessing();
				KVLOG("EF>Finish to FusionProcessing")
				EF.RobustNormalization();
				tok = clock();
				KVLOG("EF>Total processing time :%.2f ms", (float)(tok - tic) / CLOCKS_PER_SEC)
				EF.SaveImageBMP(DataSetPath+"\\"+ vDataSetFolds[i]+"_EF_normal.jpg");
				KVLOG("EF>Save EF Resault %s \n\n", (DataSetPath + "\\" + vDataSetFolds[i] + "_EF.jpg").c_str())
				system("cls");
			}
		}
	}
	return 0;
}