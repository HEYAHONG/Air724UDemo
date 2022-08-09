#include "fstream"
#include "dirent.h"
#include "unistd.h"
#include "sys/stat.h"
#include  <string>
#include  <vector>
#include "stdlib.h"
#include "stdint.h"

std::vector<std::string> filelist;
void listdir(std::string dirname,std::string root)
{
#ifdef WIN32
#endif // WIN32
    DIR *dir=NULL;
    if(dirname.empty())
    {
        dir=opendir(root.c_str());
    }
    else
    {
        dir=opendir((root+dirname).c_str());
    }
    if(dir!=NULL)
    {
        struct dirent * dirnode=NULL;
        while((dirnode=readdir(dir)))
        {
            std::string name=dirname+std::string(dirnode->d_name);
            struct stat state= {0};
            if(0!=stat((root+name).c_str(),&state))
            {
                continue;
            }
            if(state.st_mode & S_IFREG)
            {
                if(name.empty())
                {
                    continue;
                }
                filelist.push_back(name);
                printf("file: %s\n",name.c_str());
            }
            if(state.st_mode & S_IFDIR)
            {
                if(std::string(dirnode->d_name)=="..")
                {
                    //不向上遍历
                    continue;
                }
                if(std::string(dirnode->d_name)==".")
                {
                    //不遍历本目录
                    continue;
                }
                listdir(name+"/",root);
            }
        }
        closedir(dir);
    }

}

#include "json/value.h"
#include "json/writer.h"
std::string fsaddr;
std::string fssize;
void fsgen(std::string filename,std::string fileroot)
{
    Json::Value root;
    std::string json;
    {
        Json::Value flist;
        for(std::string name:filelist)
        {
            Json::Value file;
            file["file"]=name;
            file["local_file"]=(fileroot+name);
            flist.append(file);
        }
        Json::FastWriter writer;
        json=writer.write(flist);
        for(auto it=json.begin();it!=json.end();it++)
        {
            if((*it)=='\n' || (*it)=='\r')
            {
                json.erase(it);
                break;
            }
        }
    }


    {
        char buff[4096+json.length()]= {0};
        sprintf(buff,"{\"type\":\"FBD2\",\"offset\":\"%s\",\"size\":\"%s\",\"erase_block\":\"0x8000\",\"logic_block\":\"0x200\",\"partiton\":[{\"plain_file\":%s,\"lzma_block_size\":\"0x8000\",\"offset\":\"0\",\"count\":\"0\"}]}",fsaddr.c_str(),fssize.c_str(),json.c_str());
        {
            std::fstream file;
            file.open(filename.c_str(),std::ios_base::out);
            if(file.is_open())
            {
                file.write(buff,strlen(buff));
                file.close();
            }
        }
    }
}

int main(int argc,char *argv[])
{
    /*
    argv[1]:root path
    argv[2]:sffs.json path
    argv[3]:sffs addr
    argv[4]:sffs size
    */
    if(argc<5)
    {
        return -1;
    }

    fsaddr=argv[3];
    fssize=argv[4];

    setbuf(stdout,NULL);

    {
        printf("root:%s\nsffs.json:%s\n",argv[1],argv[2]);
    }

    listdir("",std::string(argv[1])+"/");


    fsgen(argv[2],std::string(argv[1])+"/");

    return 0;


}


