#include<stdio.h>
#include<string>
#include<vector>
#include<iostream>
#include<Windows.h>
#include<io.h>
#include<map>

#include"FIleMd5.h"
using namespace std;
//获取当前程序路径
string getNowDir()
{
	char buff[5000];
//	GetCurrentDirectory(5000, buff);
	return string(buff);
}

//获取目录下所有文件
void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息，声明一个存储文件信息的结构体  
	struct _finddata_t fileinfo;
	string p;//字符串，存放路径
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)//若查找成功，则进入
	{
		do
		{
			//如果是目录,迭代之（即文件夹内还有文件夹）  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				//文件名不等于"."&&文件名不等于".."
				//.表示当前目录
				//..表示当前目录的父目录
				//判断时，两者都要忽略，不然就无限递归跳不出去了！
				/*if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);*/
			}
			//如果不是,加入列表  
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		//_findclose函数结束查找
		_findclose(hFile);
	}
}

int main()
{
	map<string, int>mpt;
	vector<string>fileSrc;
	int repeatNum = 0;//重复文件数量
	getFiles(".", fileSrc);
	cout << "/***文件查重v1.0***/" << endl;
	cout << "开始查找当前目录" << endl << endl;
	for (int i = 0; i < fileSrc.size(); i++)
	{
		string md5 = MD5_file(fileSrc[i].c_str(), 32);
		if (mpt.count(md5))
		{
			//重复
			int oldi = mpt[md5] - 1;
			repeatNum++;
			cout << "发现文件重复:" << endl;
			cout << fileSrc[oldi] << endl << fileSrc[i] << endl;

			string input;
			cout << "是否打开文件(y/n):";
			cin >> input;
			if (input == "y" || input == "Y")
			{
				char str[60000];
				strcpy(str,(string("start \"\" \"") + fileSrc[oldi] + "\"").c_str());
				system(str);
				system((string("start \"\" \"") + fileSrc[i] + "\"").c_str());
			}

			cout << "是否删除第二个文件(y/n):";
			cin >> input;
			if (input == "y" || input == "Y")
			{
				if (!remove(fileSrc[i].c_str()))cout << "删除成功";
				else cout << "删除失败";
			}

			cout << endl << endl;
		}
		else {
			mpt[md5] = i + 1;
		}
	}

	cout << "查询完成,";
	if (repeatNum)
	{
		cout << "发现" << repeatNum << "组重复文件。" << endl;
	}
	else cout << "未发现重复文件" << endl << endl;
	system("pause");
	return 0;
}