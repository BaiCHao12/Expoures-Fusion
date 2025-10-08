#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#define UTC_TIMESTAP std::chrono::system_clock::now().time_since_epoch().count()

template< typename... Args >
std::string string_format(const char* format, Args... args)
{
    size_t length = std::snprintf(nullptr, 0, format, args...);
    if (length <= 0)
    {
        return "";
    }

    char* buf = new char[length + 1];
    std::snprintf(buf, length + 1, format, args...);

    std::string str(buf);
    delete[] buf;
    return std::move(str);
}


#define KVLOG(fmt, ...) \
printf((std::string("[%lld] ")+std::string(fmt)+std::string("\n")).c_str(),UTC_TIMESTAP,##__VA_ARGS__);



//读取指定路径下的指定后缀的文件\文件夹
//path 文件路径
//files 存储文件名字的线性表
//format 指定文件的后缀
bool GetFolds(std::string path, std::vector<std::string> &vfolds);
bool GetFiles(std::string path, std::vector<std::string> &files, std::string format = "");

#ifdef OPENCV_ALL_HPP
void ShowIMG( cv::Mat IMG, std::string WName="Disp", int Delay_ms=0, bool Destry=false)
{
	if (!IMG.data)return;
	cv::Mat dis= IMG.clone();
	while (dis.rows > 1080 || dis.cols > 1920)
	{
		cv::resize(dis, dis, cv::Size(), 0.5, 0.5);
	}
	cv::imshow(WName, dis);
	if(Delay_ms>=0)
		cv::waitKey(Delay_ms);
	if (Destry)
		cv::destroyWindow(WName);
}
#endif // OPENCV_ALL_HPP
