#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "xxtea.h"
#include <iostream>

using namespace std;

#define MAX_PATH 512  //最大文件长度定义为512


/*对目录中所有文件执行print_file_info操作*/
void dirwalk(char *dir, void (*func)(char *))
{
    char name[MAX_PATH];
    struct dirent *dp;
    DIR *dfd;
    
    if((dfd = opendir(dir)) == NULL){
        fprintf(stderr, "dirwalk: can't open %s\n", dir);
        return;
    }
    
    while((dp = readdir(dfd)) != NULL){ //读目录记录项
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp -> d_name, "..") == 0){
            continue;  //跳过当前目录以及父目录
        }
        
        if(strlen(dir) + strlen(dp -> d_name) + 2 > sizeof(name)){
            fprintf(stderr, "dirwalk : name %s %s too long\n", dir, dp->d_name);
        }else{
            sprintf(name, "%s/%s", dir, dp->d_name);
            (*func)(name);
        }
    }
}

char path[50];
char key[50] ;
xxtea_long signLen = 0;

void decrypt(char *name)
{
    FILE *fp;

    
    fp = fopen(name, "rb+");
    if (fp == NULL) {
        printf("[-] can't open file:%s",name);
        return ;
    }
    
    //计算文件大小
    fseek(fp, 0L, SEEK_END);
    long len = ftell(fp);
    
    //printf("len:%ld   filename:%s\n",len,name);
    
    unsigned char * buff = new unsigned char[len];
    unsigned char * result ;
    
    fseek(fp, 0L, SEEK_SET);
    fread(buff, sizeof(unsigned char), len, fp);
    

    
    xxtea_long ret_length = 0 ;
    xxtea_long keyLen =  10;
    //解密
    result = xxtea_decrypt(buff+signLen, (xxtea_long)len-signLen, (unsigned char*)key, keyLen, &ret_length);
    
    if(0 != ret_length){
        printf("[+] file:%s --- decrypt success!\n",name);

    }else{
        printf("[-] file:%s --- decrypt failed!\n",name);
    }
    //解密结果写入文件
    fseek(fp, 0, SEEK_SET);
    fwrite(result, ret_length, 1, fp);
    fclose(fp);
}

/*打印文件信息*/
void print_file_info(char *name)
{
    struct stat stbuf;
    
    if(stat(name, &stbuf) == -1){
        fprintf(stderr, "file size: open %s failed\n", name);
        return;
    }
    
    if((stbuf.st_mode & S_IFMT) == S_IFDIR){
        dirwalk(name, print_file_info);  //如果是目录遍历下一级目录
    }else{
        //printf("%8ld    %s\n", stbuf.st_size, name);//不是目录，打印文件size及name
        decrypt(name);
    }
}


void usage()
{
    cout << "usage: ./xxtea_decrypt <key> <sign length> <path>\n";
    exit(0);
}

int main(int argc, char *argv[])
{

    if (argc < 3) {
        usage();
    }
    else{
        
        strcpy(path, argv[3]);
        strcpy(key, argv[1]);
        signLen = atoi(argv[2]);
        
        printf("[+] file path:%s\n[+] decrypt key:%s\n[+] sign length:%d\n",path,key,signLen);
        print_file_info(path);
    }
    
    return 0;
}
