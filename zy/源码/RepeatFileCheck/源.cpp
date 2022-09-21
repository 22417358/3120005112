#include<stdio.h>
#include<string>
#include<vector>
#include<iostream>
#include<Windows.h>
#include<io.h>
#include<map>

#include"FIleMd5.h"
using namespace std;
//��ȡ��ǰ����·��
string getNowDir()
{
	char buff[5000];
//	GetCurrentDirectory(5000, buff);
	return string(buff);
}

//��ȡĿ¼�������ļ�
void getFiles(string path, vector<string>& files)
{
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ������һ���洢�ļ���Ϣ�Ľṹ��  
	struct _finddata_t fileinfo;
	string p;//�ַ��������·��
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)//�����ҳɹ��������
	{
		do
		{
			//�����Ŀ¼,����֮�����ļ����ڻ����ļ��У�  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				//�ļ���������"."&&�ļ���������".."
				//.��ʾ��ǰĿ¼
				//..��ʾ��ǰĿ¼�ĸ�Ŀ¼
				//�ж�ʱ�����߶�Ҫ���ԣ���Ȼ�����޵ݹ�������ȥ�ˣ�
				/*if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);*/
			}
			//�������,�����б�  
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		//_findclose������������
		_findclose(hFile);
	}
}

int main()
{
	map<string, int>mpt;
	vector<string>fileSrc;
	int repeatNum = 0;//�ظ��ļ�����
	getFiles(".", fileSrc);
	cout << "/***�ļ�����v1.0***/" << endl;
	cout << "��ʼ���ҵ�ǰĿ¼" << endl << endl;
	for (int i = 0; i < fileSrc.size(); i++)
	{
		string md5 = MD5_file(fileSrc[i].c_str(), 32);
		if (mpt.count(md5))
		{
			//�ظ�
			int oldi = mpt[md5] - 1;
			repeatNum++;
			cout << "�����ļ��ظ�:" << endl;
			cout << fileSrc[oldi] << endl << fileSrc[i] << endl;

			string input;
			cout << "�Ƿ���ļ�(y/n):";
			cin >> input;
			if (input == "y" || input == "Y")
			{
				char str[60000];
				strcpy(str,(string("start \"\" \"") + fileSrc[oldi] + "\"").c_str());
				system(str);
				system((string("start \"\" \"") + fileSrc[i] + "\"").c_str());
			}

			cout << "�Ƿ�ɾ���ڶ����ļ�(y/n):";
			cin >> input;
			if (input == "y" || input == "Y")
			{
				if (!remove(fileSrc[i].c_str()))cout << "ɾ���ɹ�";
				else cout << "ɾ��ʧ��";
			}

			cout << endl << endl;
		}
		else {
			mpt[md5] = i + 1;
		}
	}

	cout << "��ѯ���,";
	if (repeatNum)
	{
		cout << "����" << repeatNum << "���ظ��ļ���" << endl;
	}
	else cout << "δ�����ظ��ļ�" << endl << endl;
	system("pause");
	return 0;
}