#include <stdio.h>   
#include <stdlib.h>   
#include <conio.h>   
#include <string.h>   
#include <time.h>   
const unsigned PRO_SET_COMM_COU = 10;       //Ԥ��������     
const unsigned FILE_SYS_SIZE = 2048 * 2048; //ģ���ļ�ϵͳ��������4M   
const unsigned BITMAP_LEN = 64;             //λʾͼ�ĳ��� 64   
const unsigned BLOCK_SIZE = 512;            //�ļ���СΪ512   
const unsigned BLOCK_COUNT = 512;           //�ļ�������
const unsigned COMMAND_LEN = 200;           //��������󳤶�    
const unsigned NAME_LEN = 20;               //��ļ����ĳ���   
const unsigned PASSWORD_LEN = 20;           //�û�������󳤶�   
const unsigned LOGIN_COUNT = 3;             //�û���¼���Դ���   



const char* PRO_SET_COMM[] = { "create", "open", "read", "write", "close",

"delete", "mkdir", "cd", "dir", "logout" };

//�ļ�Ԫ�ؿɹ�������Ȩ��   
typedef enum
{
    pub,                //�κ��˿����κβ���   
    protect,            //�Ǵ����߻����Ա��ֻ���Բ쿴   
    pri                 //�Ǵ����߻����Ա�����������κβ���   
} FileAccess;

//�ļ�Ԫ������   
typedef enum
{
    file,               //�ļ�   
    dir                 //�ļ���   
} FileType;

//�û�����   
typedef enum
{
    administrator,              //�����ߣ�ӵ������Ȩ��   
    guest                //һ���û�   
} UserType;

//�ļ�״̬   
typedef enum
{
    closed,
    opened,
    reading,
    writing
} FileStatus;


//һ���ļ������ṹ   
typedef struct
{
    unsigned Index;                     //�ļ�Ԫ���������   
    char FileName[NAME_LEN];            //�ļ�Ԫ����   
    char ParentName[NAME_LEN];          //���ڵ���   
    unsigned FileBlockId;               //�ļ�Ԫ�������������   
    unsigned FileLevel;                 //�ļ�Ԫ�����ڲ�Σ��㣫�ļ�Ԫ����Ϊһ���ļ�Ԫ�ص��߼�λ��   
    unsigned effect;                    //�Ƿ���Ч��0-��Ч��1-��Ч   
} FileIndexElement;

//�ļ������ṹ��Ŀ¼����   
typedef struct
{
    FileIndexElement* FIStart;      //�ļ�ϵͳ�е��ļ�������ʼλ��   
    unsigned FILen;                 //�ļ���������󳤶�   
    unsigned FICount;               //�ļ���������       
} FileIndex;

//�ļ���Ľṹ   
typedef struct fb
{
    unsigned FileBlockId;       //�ļ�����   
    unsigned BLOCK_SIZE;        //�ļ��������   
    char* FileBlockAddr;        //�ļ����ַ   
    struct fb* next;            //��һ���ļ���ĵ�ַ   
} FileBlock;

//�ļ�ϵͳ��λʾͼ�ṹ   
typedef struct
{
    unsigned BITMAP_LEN;        //�ļ�λʾͼ����   
    char* BMStart;              //λʾͼ����ʼָ��   
} BitMap;

//�ļ�ϵͳ�ṹ   
typedef struct
{
    char* FSStart;                  //�ļ�ϵͳ����ʼ��ַ   
    unsigned SuperBlockSize;        //�ļ�ϵͳ������   
    BitMap bm;                      //�ļ�ϵͳ�е�λʾͼ   
    unsigned BLOCK_COUNT;           //�ļ�ϵͳ���ļ��������   
    FileBlock* head;                //�ļ�ϵͳ���ļ����׵�ַ   
    FileIndex FI;                   //�ļ�ϵͳ�е��ļ�����   
} SuperBlock;

typedef struct
{
    char* UserName;                 //�û�����   
    UserType ut;                    //�û�����   
} User;

//�ļ�ϵͳ�е�Ԫ�ؽṹ�������ļ����ļ���   
typedef struct fse
{
    struct fse* parent;                 //ָ���Լ��ĸ��׽ڵ�   
    unsigned FileLevel;                 //�ļ�Ԫ�����ڲ�Σ��㣫�ļ�Ԫ����Ϊһ���ļ�Ԫ�ص��߼�λ��   
    char FileName[NAME_LEN];            //�ļ�Ԫ����   
    unsigned FileBlockId;               //�ļ�Ԫ�������������   
    unsigned FileElemLen;               //�ļ�Ԫ�صĳ���   
    FileType Type;                      //�ļ�Ԫ������   
    FileAccess Access;                  //�ļ�Ԫ�ؿɹ�������Ȩ��   
    User Creator;                       //�ļ�������   
    char CreateTime[18];                //����ʱ�䣬���ڸ�ʽ��MM/DD/YY HH:MI:SS  


    char LastModTime[18];               //���һ���޸�ʱ��   
    char* FileData;                     //һ���ļ������ݿ�ʼ��ַ���ļ���ʱ��ֵΪNULL   
    FileStatus fileStu;                 //�����һ���ļ���ʾ�ļ���ǰ��״̬   
} FSElement;

//ϵͳ��ǰ״̬   
typedef struct
{
    User CurrentUser;               //��ǰ�û�   
    unsigned FileLevel;             //�û������ļ�ϵͳ��   
    FSElement* CurrParent;          //��ǰ��ĸ��ڵ�   
    char* CurrentPath;              //��ǰ·��   
} CurrentStatus;

SuperBlock FS;      //һ��ȫ���ļ�ϵͳ�ı���   
CurrentStatus CS;   //��ǰϵͳ״̬   
FSElement* base;    //�ļ�Ԫ�صĸ�   

/////////////////////////////////////////////////////////////////////  

bool InitFileSys();   //Ѱ�ҵ�һ���հ׵��ļ���ID 
unsigned FindBlankFileBlockId()
{
    unsigned char c;
    for (unsigned i = 0; i < FS.bm.BITMAP_LEN / 8; i++)
    {
        c = FS.bm.BMStart[i] | 0x7F;
        if (c == 0x7F)
        {
            return i * 8;       //һ���ֽ���ߵ�һλΪ0����ʾ������δʹ��   
        }

        c = FS.bm.BMStart[i] | 0xBF;
        if (c == 0xBF)
        {
            return i * 8 + 1;
        }

        c = FS.bm.BMStart[i] | 0xDF;
        if (c == 0xDF)
        {
            return i * 8 + 2;
        }
        c = FS.bm.BMStart[i] | 0xEF;
        if (c == 0xEF)
        {
            return i * 8 + 3;
        }
        c = FS.bm.BMStart[i] | 0xF7;
        if (c == 0xF7)
        {
            return i * 8 + 4;
        }
        c = FS.bm.BMStart[i] | 0xFB;
        if (c == 0xFB)
        {
            return i * 8 + 5;
        }
        c = FS.bm.BMStart[i] | 0xFD;
        if (c == 0xFD)
        {
            return i * 8 + 6;
        }
        c = FS.bm.BMStart[i] | 0xFE;
        if (c == 0xFE)
        {
            return i * 8 + 7;
        }
    }
    return BLOCK_COUNT + 1;
}


char* FindBlankFileBlock(unsigned fileblockid)   //Ѱ�ҵ�һ���ļ����ַ  
{
    FileBlock* fblock = FS.head;
    while (fblock->next != NULL)
    {
        if (fblock->FileBlockId == fileblockid)
        {
            return fblock->FileBlockAddr;
        }
        else
        {
            fblock = fblock->next;
        }
    }
    return NULL;
}


void GetCurrentTime(char* currtime)   //�õ���ǰʱ����ַ���  
{
    char dbuffer[9];
    char tbuffer[9];
    _strdate(dbuffer);
    _strtime(tbuffer);
    strcpy(currtime, dbuffer);
    strcat(currtime, " ");
    strcat(currtime, tbuffer);
}


void AddFileIndex(unsigned fileblockid, unsigned filelevel, char* filename, char* parentname)   //�����ļ�����  
{
    FS.FI.FIStart[FS.FI.FICount].FileBlockId = fileblockid;
    FS.FI.FIStart[FS.FI.FICount].FileLevel = filelevel;
    strcpy(FS.FI.FIStart[FS.FI.FICount].FileName, filename);
    if (parentname == NULL)
    {
        memset(FS.FI.FIStart[FS.FI.FICount].ParentName, '\0', NAME_LEN);
    }
    else
    {
        strcpy(FS.FI.FIStart[FS.FI.FICount].ParentName, parentname);
    }
    FS.FI.FIStart[FS.FI.FICount].Index = FS.FI.FICount;
    FS.FI.FIStart[FS.FI.FICount].effect = 1;
    FS.FI.FICount++;
}


void UpdateBitMap(unsigned fileblockid)    //�Ƹ�����λʾͼ��λ��
{

    int dirInBitmap = ((int)(fileblockid / 8));
    int dirInChar = fileblockid % 8;

    char* c = &(FS.bm.BMStart[dirInBitmap]);
    char xor;
    switch (dirInChar)
    {
    case 0:
        xor = 0x80;
        break;
    case 1:
        xor = 0x40;
        break;
    case 2:
        xor = 0x20;
        break;
    case 3:
        xor = 0x10;
        break;
    case 4:
        xor = 0x08;
        break;
    case 5:
        xor = 0x04;
        break;
    case 6:
        xor = 0x02;
        break;
    case 7:
        xor = 0x01;
        break;
    }
    *c = *c^xor;
}


FSElement* CreateFileElement(FileAccess acc, char* filename, FileType type, char* filecontent, FSElement* parent)   //����һ���ļ�Ԫ��  
{
    //���ҵ�һ���հ��ļ���ID   
    unsigned blankFileBlockId = FindBlankFileBlockId();
    if (blankFileBlockId >= BLOCK_COUNT)
    {
        printf("δ�ҵ�һ���ļ����id\n");
        return NULL;
    }

    //���ҵ�һ���հ׿�ĵ�ַ   
    char* blank = FindBlankFileBlock(blankFileBlockId);
    if (blank == NULL)
    {
        printf("δ�ҵ�һ���ļ���ĵ�ַ\n");
        return NULL;
    }

    FSElement* fs = (FSElement*)blank;

    fs->Access = acc;
    fs->Creator = CS.CurrentUser;
    GetCurrentTime(fs->CreateTime);
    fs->FileBlockId = blankFileBlockId;
    fs->FileLevel = CS.FileLevel;
    strcpy(fs->FileName, filename);
    strcpy(fs->LastModTime, fs->CreateTime);
    fs->Type = type;
    fs->parent = parent;

    if (type == dir)
    {
        fs->FileElemLen = sizeof(FSElement);
        fs->FileData = NULL;
    }
    else
    {
        fs->FileElemLen = (unsigned)strlen(filename);
        fs->fileStu = closed;
        fs->FileData = (char*)fs + sizeof(FSElement);
        if (filecontent == NULL)
        {
        }
        else
        {
            strcpy(fs->FileData, filecontent);
        }
    }

    //��������     
    if (parent == NULL)
    {
        AddFileIndex(blankFileBlockId, CS.FileLevel, filename, NULL);
    }
    else
    {
        AddFileIndex(blankFileBlockId, CS.FileLevel, filename, parent->FileName);
    }

    //����BITMAP   
    UpdateBitMap(blankFileBlockId);
    return fs;
}



FileBlock* CreateFileBlockList(char* datahead, unsigned blockcap, unsigned len) //�����ļ�������  


{
    if (datahead == NULL || len == 0)
    {
        return NULL;
    }
    FileBlock* head;
    FileBlock* pnew;
    FileBlock* pold;

    head = pold = pnew = (FileBlock*)malloc(sizeof(FileBlock));
    for (unsigned i = 0; i < len; i++)
    {
        pold->FileBlockId = i;
        pold->BLOCK_SIZE = BLOCK_SIZE;
        pold->FileBlockAddr = datahead + i * blockcap;
        memset(pold->FileBlockAddr, '\0', blockcap);
        if (i != len - 1)
        {
            pnew = (FileBlock*)malloc(sizeof(FileBlock));
        }
        else
        {
            pnew = NULL;
        }
        pold->next = pnew;
        pold = pnew;

    }
    return head;
}

bool InitFileSys()   //��ʼ��ģ���ļ�ϵͳ  
{

    if ((FS.FSStart = (char*)malloc(FILE_SYS_SIZE)) == NULL)
    {
        return false;
    }

    FS.SuperBlockSize = FILE_SYS_SIZE;
    FS.bm.BITMAP_LEN = BITMAP_LEN;
    FS.bm.BMStart = FS.FSStart;

    //����λʾͼΪδʹ��   
    memset(FS.bm.BMStart, '\0', FS.bm.BITMAP_LEN);

    //��ʼ���ļ�ϵͳ����   
    FS.FI.FIStart = (FileIndexElement*)(FS.FSStart + BITMAP_LEN);
    //��Ϊ��ģ��ϵͳ���ݶ�һ���ļ����ļ������ռ��һ���ļ��飬һ���ļ���ֻ��һ���ļ�Ԫ��   
    FS.FI.FILen = sizeof(FileIndexElement) * BLOCK_COUNT + sizeof(unsigned) *

        2;
    FS.FI.FICount = 0;
    memset(FS.FI.FIStart, '\0', FS.FI.FILen);

    //��ʼ���ļ���   
    FS.BLOCK_COUNT = BLOCK_COUNT;
    FS.head = CreateFileBlockList((FS.FSStart + FILE_SYS_SIZE - BLOCK_SIZE *

        BLOCK_COUNT),
        BLOCK_SIZE, FS.BLOCK_COUNT);//����ĺ�BLOCK_SIZE * BLOCK_COUNT����Ԫ�����洢����   

    if (FS.head == NULL)
    {
        return false;
    }


    //��ʼ��ϵͳ��ǰ״̬   
    CS.CurrentUser.UserName = (char*)calloc(10, sizeof(char));
    strcpy(CS.CurrentUser.UserName, "man");
    CS.CurrentUser.ut = administrator;
    CS.CurrParent = NULL;
    CS.FileLevel = 0;
    CS.CurrentPath = (char*)calloc(1000, sizeof(char));

    //����һ����Ŀ¼   
    base = CreateFileElement(pub, "root", dir, NULL, NULL);
    if (base == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool Login()   //ϵͳ��¼ģ��  
{
    char username[10];
    char password[PASSWORD_LEN];
    int c;

    for (c = 0; c < LOGIN_COUNT; c++)
    {
        int i = 0;
        memset(password, '\0', PASSWORD_LEN);
        memset(username, '\0', 10);
        printf("��¼��\n�������û�����");
        gets(username);

        printf("����:");
        while (i < PASSWORD_LEN && (password[i++] = getch()) != 0x0d) {};
        password[strlen(password) - 1] = '\0';

        if ((strcmp(username, "hzw1") == 0 && strcmp(password, "123") == 0) ||
            (strcmp(username, "hzw2") == 0 && strcmp(password, "123") == 0) ||
            (strcmp(username, "administrator") == 0 && strcmp(password, "123")

                == 0))
        {
            if (strcmp(username, "administrator") == 0)      //  һ������Ա   
            {
                strcpy(CS.CurrentUser.UserName, username);
                CS.CurrentUser.ut = administrator;
            }
            else
            {
                strcpy(CS.CurrentUser.UserName, username);
                CS.CurrentUser.ut = guest;
            }
            CS.FileLevel++;
            CS.CurrParent = base;
            strcpy(CS.CurrentPath, "/");
            printf("\n\t\t��ӭʹ�ö��û��༶Ŀ¼�ļ�ϵͳ\n");
            printf("\n");
            printf("\n%s��ӭ��½\n", username);
            printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
            break;
        }
        else
        {
            printf("\n�û���������������������롣\n");
        }
    }

    if (c >= LOGIN_COUNT)        //�Ƿ��û�   
    {
        printf("\n�Բ��������Ǹ�ϵͳ�û�����������˳�ϵͳ��\n");
        return false;
    }
    else
    {
        return true;
    }
}

void Create(char* filename)   //����һ���ļ�  
{
    if (strcmp(filename, "") == 0)
    {
        printf("�Բ����ļ�������Ϊ�ա�\n");
    }
    else
    {
        CreateFileElement(protect, filename, file, NULL, CS.CurrParent);
        printf("�ļ������ɹ�\n");
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Dir(char* path)   //�г���ǰĿ¼���ļ����ļ��� 
{
    char display[1000];
    memset(display, '\0', 1000);

    //������ʾ����   
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

            [i].effect == 1)
        {
            strcat(display, FS.FI.FIStart[i].FileName);
            strcat(display, "\t\t");

        }
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Mkdir(char* filename)   //����һ���ļ�
{
    if (strcmp(filename, "") == 0)
    {
        printf("�Բ����ļ���������Ϊ�ա�\n");
    }
    else
    {
        CreateFileElement(protect, filename, dir, NULL, CS.CurrParent);
        printf("����Ŀ¼�ɹ�\n");
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Cd(char* path)   //����һ���ļ���  
{
    int splitDisplayCou = 0;    //�ָ�����ֵĴ���   
    if (strcmp(path, "..") == 0)  //������һ��Ŀ¼,����Ŀ¼   
    {
        if (CS.FileLevel > 0)
        {
            CS.FileLevel--;
            CS.CurrParent = CS.CurrParent->parent;
            for (unsigned i = strlen(CS.CurrentPath) - 1; i > 0; i--)
            {
                if (CS.CurrentPath[i] == '/')
                {
                    splitDisplayCou++;
                    if (splitDisplayCou == 2)   //�ѹ��˵����һ��Ŀ¼��   
                    {
                        break;
                    }
                }
            }
            char temppath[1000];
            strcpy(temppath, CS.CurrentPath);
            memset(CS.CurrentPath, '\0', 1000);
            strncpy(CS.CurrentPath, temppath, i + 1);
        }
        else
        {

        }
    }
    else
    {
        char display[100] = "";
        for (unsigned i = 0; i < FS.FI.FICount; i++)
        {
            if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) ==

                0 &&
                FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

                [i].effect == 1 &&
                strcmp(FS.FI.FIStart[i].FileName, path) == 0)
            {
                strcpy(display, "�ļ����ڡ�\n");
                CS.CurrParent = (FSElement*)FindBlankFileBlock(FS.FI.FIStart

                    [i].FileBlockId);
                CS.FileLevel++;
                strcat(CS.CurrentPath, path);
                strcat(CS.CurrentPath, "/");
                break;
            }
        }
        if (strcmp(display, "") == 0)   //�ļ��в����ڣ�ʲô������   
        {
            printf("��ǰĿ¼��û����Ҫ������ļ��С�\n");
        }
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Delete(char* path)   //ɾ����ǰĿ¼���ļ�  
{
    char display[100] = "";
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

            [i].effect == 1 &&
            strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FS.FI.FIStart[i].effect = 0;    //ɾ�����   
            strcpy(display, "�ļ���ɾ����\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        strcpy(display, "��ǰĿ¼��û����Ҫɾ�����ļ���\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Open(char* path)   //�򿪵�ǰĿ¼���ļ�  
{
    char display[100];
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 && FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart[i].effect == 1 && strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart[i].FileBlockId);
            fselem->fileStu = opened;
            strcpy(display, "�ļ��Ѵ���ϡ�\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        printf("��ǰĿ¼��û����Ҫ�򿪵��ļ���\n");

    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Read(char* path)   //��ȡ��ǰĿ¼�ļ� 
{
    char display[BLOCK_SIZE];
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

            [i].effect == 1 &&
            strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart

                [i].FileBlockId);
            if (fselem->fileStu == closed)
            {
                strcpy(display, "�ļ���δ�򿪣����ȴ��ļ���\n");
            }
            else
            {
                if (fselem->FileData == NULL || strcmp(fselem->FileData, "") ==

                    0)
                {
                    strcpy(display, "�ļ������ݡ�\n");
                }
                else
                {
                    strcpy(display, fselem->FileData);
                }
                fselem->fileStu = reading;
            }

            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        strcpy(display, "��ǰĿ¼��û����Ҫ�򿪵��ļ���\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);


}

void Write(char* path)   //д��ǰĿ¼�ļ�  
{
    char display[BLOCK_SIZE];
    memset(display, '\0', BLOCK_SIZE);

    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

            [i].effect == 1 &&
            strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart

                [i].FileBlockId);
            if (fselem->fileStu == closed)
            {
                strcpy(display, "�ļ���δ�򿪣����ȴ��ļ���\n");
            }
            else
            {
                printf("\nע�⣺�ļ���󲻿��Գ��� %d �ֽڣ�����CTRL+D�����༭��\n", BLOCK_SIZE - sizeof(FSElement));
                char c;
                int i = 0;
                while ((c = getchar()) != 0x04)
                {
                    display[i++] = c;
                }
                getchar();  //����س�   
                display[i] = '\0';
                strcpy(fselem->FileData, display);
                unsigned len = strlen(display) < BLOCK_SIZE - sizeof(FSElement) ?

                    strlen(display) : BLOCK_SIZE - sizeof(FSElement);
                strncpy(fselem->FileData, display, len);
                fselem->fileStu = writing;
                strcpy(display, "�ļ�д��ɹ���\n");
            }

            break;
        }
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);



}

void Close(char* path)   //�رյ�ǰĿ¼���ļ�
{
    char display[100];
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart[i].effect == 1 && strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart[i].FileBlockId);
            fselem->fileStu = opened;
            strcpy(display, "�ļ��ѹرա�\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        strcpy(display, "��ǰĿ¼��û����Ҫ�رյ��ļ���\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}


void FindCommKey(char* command, char* key, char* path)   //����ֽ� 
{
    for (unsigned i = 0; i < strlen(command); i++)
    {
        if (command[i] == ' ')
        {
            i++;
            if (i < strlen(command))
            {
                strcpy(path, command + i);
            }
            break;
        }
        if (i <= 9)
        {
            key[i] = command[i];
        }
        else
        {
            for (unsigned j = i; j < strlen(command); j++)
            {
                if (command[j] != ' ')
                {
                    strcpy(path, command + j);
                    break;
                }
            }
            break;
        }
    }
    strlwr(key);
    strlwr(path);
}

void Shell()   //�������ģ��  
{
    char command[COMMAND_LEN];
    gets(command);
    char key[10];
    char path[COMMAND_LEN - 10];
    while (true)
    {
        memset(key, '\0', 10);
        memset(path, '\0', COMMAND_LEN - 10);
        FindCommKey(command, key, path);
        unsigned i;
        for (i = 0; i < PRO_SET_COMM_COU; i++)
        {
            if (strcmp(key, PRO_SET_COMM[i]) == 0)
            {
                break;
            }
        }

        switch (i)
        {
        case 0:
            Create(path);
            break;
        case 1:
            Open(path);
            break;
        case 2:
            Read(path);
            break;
        case 3:
            Write(path);
            break;
        case 4:
            Close(path);
            break;
        case 5:
            Delete(path);
            break;
        case 6:
            Mkdir(path);
            break;
        case 7:
            Cd(path);
            break;
        case 8:
            Dir(path);
            break;
        case 9:
            //Logout   
            break;
        default:
            printf("\n�������\n");
            printf("%sC:%s", CS.CurrentUser.UserName,

                CS.CurrentPath);
        }
        if (i == 9)
        {
            break;
        }
        gets(command);
    }
}

void ClearFileSys()   //���������ڴ�ռ�
{
    free(CS.CurrentUser.UserName);
    free(CS.CurrentPath);

    free(FS.FSStart);
}

void main()   //������
{
    //��ʼ��ϵͳ   
    if (InitFileSys())
    {
        while (Login())
        {
            Shell();   //��������
            strcpy(CS.CurrentUser.UserName, "man");
            CS.CurrentUser.ut = administrator;
            CS.FileLevel = 0;
            CS.CurrParent = NULL;
            strcpy(CS.CurrentPath, "/");
            printf("�����˳�ϵͳ��\n");
        }
    }
    else
    {
        printf("ϵͳ��ʼ��ʧ�ܣ���������˳���\n");
    }

    //����ϵͳ�ڴ�   
    ClearFileSys();
    getch();
}