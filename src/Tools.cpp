#include "Tools.h"
#include "Tools.h"
#include <cmath>
#include "Tools.h"
#include <ctime>


std::string gengerString(int n){
	srand((unsigned)time(NULL));                        
   //产生随机化种子
	printf("生成%d个字符的字符串\n",n);                                      
	std::string str = "";
	for(int i=1;i <= n;i++){
		int flag;                        
		flag = rand()%2;
        if(i % 10 == 0){
            str += "\n";
            continue;
        }                     
      //随机使flag为1或0，为1就是大写，为0就是小写 
		if(flag == 1)                        
      //如果flag=1 
			str += rand()%('Z'-'A'+1)+'A';       
         //追加大写字母的ascii码 
		else 
			str += rand()%('z'-'a'+1)+'a';       
         //如果flag=0，追加为小写字母的ascii码 
		
	}
	return str;
} 

bool genTestsFile(std::string res_dir, const int test_str_len){
   FILE *fd_des = fopen(res_dir.data(), "wb");
   if (fd_des == NULL){
      std::cout <<("[ERROR]测试文件创建失败！");
      return -1;
   }
   std::string test_str = gengerString(test_str_len);

   fwrite(test_str.data(), 1, test_str_len, fd_des);
   fclose(fd_des);
   return 1;
}


myPath::myPath(){
    path.clear();
    from_root=true;
}
myPath::myPath(const myPath &full_path){
    this->path = full_path.path;
    this->from_root = full_path.from_root;
}
myPath::myPath(const std::string& raw_path){

    std::string cur_str = raw_path;
    // 去除一开始的斜杠
    if (cur_str[0] == '/'){
        cur_str = cur_str.substr(1, cur_str.length()); //跳过正斜
        //path.push_back("/");
        from_root = true;
    }
    else{
        from_root = false;
    }
    int index = cur_str.find("/");
    while(index != std::string::npos){
        //
        std::string cur_piece = cur_str.substr(0, index);
        if(cur_piece.length() == 0){
            break;
        }
        path.push_back(cur_piece);
        cur_str = cur_str.substr(index + 1, cur_str.length());
        index = cur_str.find("/");
    }

    if(cur_str.length() > 0){
        path.push_back(cur_str);
    }

} //explicit关键字避免隐式类型转换

std::string myPath::toString() const {
    std::string full_path = "";
    if(path.size() > 0){
        full_path += path[0];
    }

    for(int i = 1 ; i < path.size(); i ++ ){
        full_path += "/" + path[i];
    }

    return full_path;
}
const std::string myPath::getInodeName() const{
    if(path.size() < 1)
        return "/";
    else
        return path[path.size() - 1]; 
}

int myPath::getLevel() const{
    return path.size() - 1;
}
std::string myPath::my_pop(){
    std::string cur = "/";
    if(path.size() > 0){
        cur = path[getLevel()];
        path.pop_back();
    }
    return cur;               
}
