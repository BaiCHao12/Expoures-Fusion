#include"Common.hpp"

#include<sstream>
#include<io.h>
#include<fstream>
#include<vector>
//读取指定路径下的指定后缀的文件
//path 文件路径
//files 存储文件名字的线性表
//format 指定文件的后缀

bool GetFolds(std::string path, std::vector<std::string> &vfolds)
{
	long long hFile = 0;     // 文件句柄
	struct _finddata_t fileInfo;  //存储文件信息的结构体
	std::string filename;
	std::string p = path;
	p.append("\\*");
	//查找指定的文件的名称 成功返回句柄编号，失败返回-1
	if ((hFile = _findfirst(p.c_str(), &fileInfo)) != -1)
	{
		do
		{
			//判断是否为文件
			if ((fileInfo.attrib & _A_SUBDIR))
			{
				//迭代查找文件
				if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0)
				{
					vfolds.push_back(filename.assign(fileInfo.name));
				}
			}
		} while (_findnext(hFile, &fileInfo) == 0);//循环查找

		//关闭查找
		_findclose(hFile);
		return true;
	}
	return false;
}
bool GetFiles(std::string path, std::vector<std::string> &files, std::string format)
{
	long long hFile = 0;     // 文件句柄
	struct _finddata_t fileInfo;  //存储文件信息的结构体
	std::string filename;
	std::string p = path;

	if (format == "")
	{
		p.append("\\*");
	}
	else
	{
		p.append("\\*" + format);
	}
	//查找指定的文件的名称 成功返回句柄编号，失败返回-1
	if ((hFile = _findfirst(p.c_str(), &fileInfo)) != -1)
	{
		do
		{
			//判断是否为文件，如果是文件进行迭代查找
			if ((fileInfo.attrib & _A_SUBDIR))
			{
			}
			else
			{
				files.push_back(filename.assign(fileInfo.name));
			}
		} while (_findnext(hFile, &fileInfo) == 0);//循环查找

		//关闭查找
		_findclose(hFile);
		return true;
	}
	return false;
}