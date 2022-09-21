#include <stdio.h>   
#include <stdlib.h>   
#include <conio.h>   
#include <string.h>   
#include <time.h>   
const unsigned PRO_SET_COMM_COU = 10;       //预设命令数     
const unsigned FILE_SYS_SIZE = 2048 * 2048; //模拟文件系统的容量设4M   
const unsigned BITMAP_LEN = 64;             //位示图的长度 64   
const unsigned BLOCK_SIZE = 512;            //文件大小为512   
const unsigned BLOCK_COUNT = 512;           //文件块数量
const unsigned COMMAND_LEN = 200;           //命令行最大长度    
const unsigned NAME_LEN = 20;               //最长文件名的长度   
const unsigned PASSWORD_LEN = 20;           //用户密码最大长度   
const unsigned LOGIN_COUNT = 3;             //用户登录尝试次数   



const char* PRO_SET_COMM[] = { "create", "open", "read", "write", "close",

"delete", "mkdir", "cd", "dir", "logout" };

//文件元素可供操作性权限   
typedef enum
{
    pub,                //任何人可做任何操作   
    protect,            //非创建者或管理员，只可以察看   
    pri                 //非创建者或管理员，不可以做任何操作   
} FileAccess;

//文件元素类型   
typedef enum
{
    file,               //文件   
    dir                 //文件夹   
} FileType;

//用户类型   
typedef enum
{
    administrator,              //管理者，拥用所有权限   
    guest                //一般用户   
} UserType;

//文件状态   
typedef enum
{
    closed,
    opened,
    reading,
    writing
} FileStatus;


//一个文件索引结构   
typedef struct
{
    unsigned Index;                     //文件元素索引编号   
    char FileName[NAME_LEN];            //文件元素名   
    char ParentName[NAME_LEN];          //父节点名   
    unsigned FileBlockId;               //文件元素所在物理块编号   
    unsigned FileLevel;                 //文件元素所在层次，层＋文件元素名为一个文件元素的逻辑位置   
    unsigned effect;                    //是否有效，0-无效，1-有效   
} FileIndexElement;

//文件索引结构或目录表项   
typedef struct
{
    FileIndexElement* FIStart;      //文件系统中的文件索引起始位置   
    unsigned FILen;                 //文件索引的最大长度   
    unsigned FICount;               //文件索引数量       
} FileIndex;

//文件块的结构   
typedef struct fb
{
    unsigned FileBlockId;       //文件块编号   
    unsigned BLOCK_SIZE;        //文件块的容量   
    char* FileBlockAddr;        //文件块地址   
    struct fb* next;            //下一个文件块的地址   
} FileBlock;

//文件系统的位示图结构   
typedef struct
{
    unsigned BITMAP_LEN;        //文件位示图长度   
    char* BMStart;              //位示图的起始指针   
} BitMap;

//文件系统结构   
typedef struct
{
    char* FSStart;                  //文件系统的起始地址   
    unsigned SuperBlockSize;        //文件系统的容量   
    BitMap bm;                      //文件系统中的位示图   
    unsigned BLOCK_COUNT;           //文件系统中文件块的数量   
    FileBlock* head;                //文件系统中文件块首地址   
    FileIndex FI;                   //文件系统中的文件索引   
} SuperBlock;

typedef struct
{
    char* UserName;                 //用户名称   
    UserType ut;                    //用户类型   
} User;

//文件系统中的元素结构，包括文件和文件夹   
typedef struct fse
{
    struct fse* parent;                 //指向自己的父亲节点   
    unsigned FileLevel;                 //文件元素所在层次，层＋文件元素名为一个文件元素的逻辑位置   
    char FileName[NAME_LEN];            //文件元素名   
    unsigned FileBlockId;               //文件元素所在物理块编号   
    unsigned FileElemLen;               //文件元素的长度   
    FileType Type;                      //文件元素类型   
    FileAccess Access;                  //文件元素可供操作的权限   
    User Creator;                       //文件创建者   
    char CreateTime[18];                //创建时间，日期格式：MM/DD/YY HH:MI:SS  


    char LastModTime[18];               //最后一次修改时间   
    char* FileData;                     //一个文件的数据开始地址，文件夹时该值为NULL   
    FileStatus fileStu;                 //如果是一个文件表示文件当前的状态   
} FSElement;

//系统当前状态   
typedef struct
{
    User CurrentUser;               //当前用户   
    unsigned FileLevel;             //用户所在文件系统层   
    FSElement* CurrParent;          //当前层的父节点   
    char* CurrentPath;              //当前路径   
} CurrentStatus;

SuperBlock FS;      //一个全局文件系统的变量   
CurrentStatus CS;   //当前系统状态   
FSElement* base;    //文件元素的根   

/////////////////////////////////////////////////////////////////////  

bool InitFileSys();   //寻找第一个空白的文件块ID 
unsigned FindBlankFileBlockId()
{
    unsigned char c;
    for (unsigned i = 0; i < FS.bm.BITMAP_LEN / 8; i++)
    {
        c = FS.bm.BMStart[i] | 0x7F;
        if (c == 0x7F)
        {
            return i * 8;       //一个字节左边第一位为0，表示该区域未使用   
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


char* FindBlankFileBlock(unsigned fileblockid)   //寻找第一个文件块地址  
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


void GetCurrentTime(char* currtime)   //得到当前时间的字符串  
{
    char dbuffer[9];
    char tbuffer[9];
    _strdate(dbuffer);
    _strtime(tbuffer);
    strcpy(currtime, dbuffer);
    strcat(currtime, " ");
    strcat(currtime, tbuffer);
}


void AddFileIndex(unsigned fileblockid, unsigned filelevel, char* filename, char* parentname)   //更新文件索引  
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


void UpdateBitMap(unsigned fileblockid)    //计复所在位示图的位置
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


FSElement* CreateFileElement(FileAccess acc, char* filename, FileType type, char* filecontent, FSElement* parent)   //创建一个文件元素  
{
    //查找第一个空白文件块ID   
    unsigned blankFileBlockId = FindBlankFileBlockId();
    if (blankFileBlockId >= BLOCK_COUNT)
    {
        printf("未找到一个文件块的id\n");
        return NULL;
    }

    //查找第一个空白块的地址   
    char* blank = FindBlankFileBlock(blankFileBlockId);
    if (blank == NULL)
    {
        printf("未找到一个文件块的地址\n");
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

    //更新索引     
    if (parent == NULL)
    {
        AddFileIndex(blankFileBlockId, CS.FileLevel, filename, NULL);
    }
    else
    {
        AddFileIndex(blankFileBlockId, CS.FileLevel, filename, parent->FileName);
    }

    //更新BITMAP   
    UpdateBitMap(blankFileBlockId);
    return fs;
}



FileBlock* CreateFileBlockList(char* datahead, unsigned blockcap, unsigned len) //创建文件块链表  


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

bool InitFileSys()   //初始化模拟文件系统  
{

    if ((FS.FSStart = (char*)malloc(FILE_SYS_SIZE)) == NULL)
    {
        return false;
    }

    FS.SuperBlockSize = FILE_SYS_SIZE;
    FS.bm.BITMAP_LEN = BITMAP_LEN;
    FS.bm.BMStart = FS.FSStart;

    //设置位示图为未使用   
    memset(FS.bm.BMStart, '\0', FS.bm.BITMAP_LEN);

    //初始化文件系统索引   
    FS.FI.FIStart = (FileIndexElement*)(FS.FSStart + BITMAP_LEN);
    //因为是模拟系统，暂定一个文件或文件夹最多占用一个文件块，一个文件块只放一个文件元素   
    FS.FI.FILen = sizeof(FileIndexElement) * BLOCK_COUNT + sizeof(unsigned) *

        2;
    FS.FI.FICount = 0;
    memset(FS.FI.FIStart, '\0', FS.FI.FILen);

    //初始化文件块   
    FS.BLOCK_COUNT = BLOCK_COUNT;
    FS.head = CreateFileBlockList((FS.FSStart + FILE_SYS_SIZE - BLOCK_SIZE *

        BLOCK_COUNT),
        BLOCK_SIZE, FS.BLOCK_COUNT);//区域的后BLOCK_SIZE * BLOCK_COUNT个单元用来存储数据   

    if (FS.head == NULL)
    {
        return false;
    }


    //初始化系统当前状态   
    CS.CurrentUser.UserName = (char*)calloc(10, sizeof(char));
    strcpy(CS.CurrentUser.UserName, "man");
    CS.CurrentUser.ut = administrator;
    CS.CurrParent = NULL;
    CS.FileLevel = 0;
    CS.CurrentPath = (char*)calloc(1000, sizeof(char));

    //创建一个根目录   
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

bool Login()   //系统登录模块  
{
    char username[10];
    char password[PASSWORD_LEN];
    int c;

    for (c = 0; c < LOGIN_COUNT; c++)
    {
        int i = 0;
        memset(password, '\0', PASSWORD_LEN);
        memset(username, '\0', 10);
        printf("登录：\n请输入用户名：");
        gets(username);

        printf("密码:");
        while (i < PASSWORD_LEN && (password[i++] = getch()) != 0x0d) {};
        password[strlen(password) - 1] = '\0';

        if ((strcmp(username, "hzw1") == 0 && strcmp(password, "123") == 0) ||
            (strcmp(username, "hzw2") == 0 && strcmp(password, "123") == 0) ||
            (strcmp(username, "administrator") == 0 && strcmp(password, "123")

                == 0))
        {
            if (strcmp(username, "administrator") == 0)      //  一个管理员   
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
            printf("\n\t\t欢迎使用多用户多级目录文件系统\n");
            printf("\n");
            printf("\n%s欢迎登陆\n", username);
            printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
            break;
        }
        else
        {
            printf("\n用户名或密码错误，请重新输入。\n");
        }
    }

    if (c >= LOGIN_COUNT)        //非法用户   
    {
        printf("\n对不起，您不是该系统用户，按任意键退出系统。\n");
        return false;
    }
    else
    {
        return true;
    }
}

void Create(char* filename)   //创建一个文件  
{
    if (strcmp(filename, "") == 0)
    {
        printf("对不起，文件名不能为空。\n");
    }
    else
    {
        CreateFileElement(protect, filename, file, NULL, CS.CurrParent);
        printf("文件创建成功\n");
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Dir(char* path)   //列出当前目录的文件和文件夹 
{
    char display[1000];
    memset(display, '\0', 1000);

    //查找显示内容   
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

void Mkdir(char* filename)   //创建一个文件
{
    if (strcmp(filename, "") == 0)
    {
        printf("对不起，文件夹名不能为空。\n");
    }
    else
    {
        CreateFileElement(protect, filename, dir, NULL, CS.CurrParent);
        printf("创建目录成功\n");
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Cd(char* path)   //进入一个文件夹  
{
    int splitDisplayCou = 0;    //分割符出现的次数   
    if (strcmp(path, "..") == 0)  //返回上一级目录,即父目录   
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
                    if (splitDisplayCou == 2)   //已过滤掉最后一个目录名   
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
                strcpy(display, "文件存在。\n");
                CS.CurrParent = (FSElement*)FindBlankFileBlock(FS.FI.FIStart

                    [i].FileBlockId);
                CS.FileLevel++;
                strcat(CS.CurrentPath, path);
                strcat(CS.CurrentPath, "/");
                break;
            }
        }
        if (strcmp(display, "") == 0)   //文件夹不存在，什么都不做   
        {
            printf("当前目录下没有您要进入的文件夹。\n");
        }
    }
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Delete(char* path)   //删除当前目录的文件  
{
    char display[100] = "";
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart

            [i].effect == 1 &&
            strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FS.FI.FIStart[i].effect = 0;    //删除标记   
            strcpy(display, "文件已删除。\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        strcpy(display, "当前目录下没有您要删除的文件。\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Open(char* path)   //打开当前目录的文件  
{
    char display[100];
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 && FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart[i].effect == 1 && strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart[i].FileBlockId);
            fselem->fileStu = opened;
            strcpy(display, "文件已打开完毕。\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        printf("当前目录下没有您要打开的文件。\n");

    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}

void Read(char* path)   //读取当前目录文件 
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
                strcpy(display, "文件尚未打开，请先打开文件。\n");
            }
            else
            {
                if (fselem->FileData == NULL || strcmp(fselem->FileData, "") ==

                    0)
                {
                    strcpy(display, "文件无内容。\n");
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
        strcpy(display, "当前目录下没有您要打开的文件。\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);


}

void Write(char* path)   //写当前目录文件  
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
                strcpy(display, "文件尚未打开，请先打开文件。\n");
            }
            else
            {
                printf("\n注意：文件最大不可以超过 %d 字节！！按CTRL+D结束编辑。\n", BLOCK_SIZE - sizeof(FSElement));
                char c;
                int i = 0;
                while ((c = getchar()) != 0x04)
                {
                    display[i++] = c;
                }
                getchar();  //处理回车   
                display[i] = '\0';
                strcpy(fselem->FileData, display);
                unsigned len = strlen(display) < BLOCK_SIZE - sizeof(FSElement) ?

                    strlen(display) : BLOCK_SIZE - sizeof(FSElement);
                strncpy(fselem->FileData, display, len);
                fselem->fileStu = writing;
                strcpy(display, "文件写入成功。\n");
            }

            break;
        }
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);



}

void Close(char* path)   //关闭当前目录的文件
{
    char display[100];
    for (unsigned i = 0; i < FS.FI.FICount; i++)
    {
        if (strcmp(FS.FI.FIStart[i].ParentName, CS.CurrParent->FileName) == 0 &&


            FS.FI.FIStart[i].FileLevel == CS.FileLevel && FS.FI.FIStart[i].effect == 1 && strcmp(FS.FI.FIStart[i].FileName, path) == 0)
        {
            FSElement* fselem = (FSElement*)FindBlankFileBlock(FS.FI.FIStart[i].FileBlockId);
            fselem->fileStu = opened;
            strcpy(display, "文件已关闭。\n");
            break;
        }
    }

    if (strcmp(display, "") == 0)
    {
        strcpy(display, "当前目录下没有您要关闭的文件。\n");
    }
    printf("%s\n", display);
    printf("%sC:%s", CS.CurrentUser.UserName, CS.CurrentPath);
}


void FindCommKey(char* command, char* key, char* path)   //命令分解 
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

void Shell()   //命令解释模块  
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
            printf("\n命令错误。\n");
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

void ClearFileSys()   //清理分配的内存空间
{
    free(CS.CurrentUser.UserName);
    free(CS.CurrentPath);

    free(FS.FSStart);
}

void main()   //主函数
{
    //初始化系统   
    if (InitFileSys())
    {
        while (Login())
        {
            Shell();   //解释命令
            strcpy(CS.CurrentUser.UserName, "man");
            CS.CurrentUser.ut = administrator;
            CS.FileLevel = 0;
            CS.CurrParent = NULL;
            strcpy(CS.CurrentPath, "/");
            printf("您已退出系统。\n");
        }
    }
    else
    {
        printf("系统初始化失败，按任意键退出。\n");
    }

    //清理系统内存   
    ClearFileSys();
    getch();
}