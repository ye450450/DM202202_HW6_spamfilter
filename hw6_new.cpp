#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

using namespace std;
#define HAM_TRAIN_SIZE 100
#define SPAM_TRAIN_SIZE 100
#define HAM_TEST_SIZE 20
#define SPAM_TEST_SIZE 20

//string delimiter를 구분자로 해서 구분해 vector<string>values에 넣어주는 함수이다.
void parse(string str, vector<string>& values, string& delimiter){
    int pos =0;
    string token;

    while((pos= str.find(delimiter))!=string::npos){
        token = str.substr(0,pos);
        values.push_back(token);
        str.erase(0,pos+delimiter.length());
    }
    values.push_back(str);
}

//char를 구분자로 해서 vector로 반환해주는 함수이다.
vector<string> split(string text, char delimiter)
{
    {
        vector<string> str_vect;
        stringstream input(text);
        string temp;

        while (getline(input, temp, delimiter))
        {
            str_vect.push_back(temp);
        }

        return str_vect; //delimiter로 나눈 string을 저장하는 vector를 반환해준다.
    }
}

//input_file에 있는 단어와 단어가 들어가 있는 메일 수를 map type에 저장해주는 함수이다.
void make_wordnum(string input_file, map<string, int>& count){
    string punctuation="+\'=\\`~!@#$%^&*()-<>?/.,:;-_{}[]|\n\"";//지어줘야하는 문자들이다.
    vector<string> mail_content; // 메일내용만을 담고 있는 vector이다.
    vector<string> mail_prev; //," 로 나눈 문자이다.
    vector<string> just_word; //" "를 단위로 나누어진 단어를 임시로 저장한다. 가공되기 전 단어를 받는다.
    set<string> split_all_word; //필요없는 문자들을 제거하고 unique한 단어들을 저장한다.
    set<string> split_word; //문장단위로 있는 단어를 저장한다.
    string wholeline;//모든 문서를 한번에 받는 string이다.

    ifstream file(input_file);

    stringstream ss;
    ss << file.rdbuf();
    wholeline = ss.str();//파일의 전체내용을 string형식으로 저장해준다.
    string delimiter(",\"");

    parse(wholeline,mail_prev,delimiter); // ,"를 기준으로 구분해준다.
    
    mail_prev.erase(mail_prev.begin()); //처음에 숫자와 메일유형이 오기 때문에 이를 지워준다.

    for(string value : mail_prev){
        value=value.substr(0,value.rfind("\"")); //뒤에서부터 "를 찾아 그 뒤에 있는 메일번호와 메일유형은 필요하지 않기 때문에 지워준다.
        mail_content.push_back(value); //이제 메일내용만을 담고 있기 때문에 이를 mail_content에 담아준다.
        if(value.find("\n")!=string::npos)
            value=regex_replace(value,regex("\n")," "); //개행문자는 구분을 못해주어서 개행문자를 " "(공백)으로 바꾸어주었다.
        just_word = split(value, ' '); // ' '(공백)을 기준으로 가공하기 전의 단어들로 저장해준다.
         for(string w : just_word){
            for (char c: punctuation) {
                w.erase(remove(w.begin(), w.end(), c), w.end()); //필요없는 문자를 제거해준다.
            }
            if(w!="")
            split_all_word.insert(w);
        } // 가공하기 전의 단어들을 가져와 필요없는 문자들을 제거하고 set에 넣어주어 중복없는 단어들로 정리한다.
    }

    for(string line : mail_content){
        if(line.find("\n")!=string::npos)
            line=regex_replace(line,regex("\n")," "); //개행문자는 구분을 못해주어서 개행문자를 " "(공백)으로 바꾸어주었다.
        just_word = split(line, ' ');
        for(string w : just_word){
            for (char c: punctuation) {
                    w.erase(remove(w.begin(), w.end(), c), w.end()); //필요없는 문자를 제거해준다.
            }
            if(w!=""){
                split_word.insert(w);
            }
        } //메일내용에서 필요없는 문자들을 제거하고 중복없는 단어로 정리해 set에 넣어준다. (split_word는 하나의 메일에서 나온 단어들이다.)
        for(string word:split_word){
           for(string word_all:split_all_word){
                if(word==word_all)
                count[word_all]= count[word_all]+1;// 메일에 총 메일에서 나온 단어가 존재하면 개수를 하나씩 올려 map 변수에 저장해준다.
           }
        }
        split_word.clear();//메일마다 달라지기 때문에 비워주어야한다.
    }//하나의 메일 단위로 단어를 받아와 총 메일의 그 단어가 몇 번 있는지를 저장해준다.
}

//test를 해주는 함수로 확률을 계산해서 spam 메일인지 아닌지를 판단해주는 함수이다.
int test(string input_file, map<string, int> count1, map<string, int > count2){
    string punctuation="+\'=\\`~!@#$%^&*()-<>?/.,:;-_{}[]|\n\"";//지어줘야하는 문자들이다.
    vector<string> mail_prev; //," 로 나눈 문자이다.
    vector<string> just_word; //" "를 단위로 나누어진 단어를 임시로 저장한다. 가공되기 전 단어를 받는다.
    set<string> split_word; //문장단위로 있는 단어를 저장한다.
    string wholeline;//모든 문서를 한번에 받는 string이다.
    
    double threshold =0.6; //허용해주는 범위를 저장하는 변수이다.
    int num= 1; //번호를 표현하기 위해 사용한다.
    int count_ham=0; //non-spam메일로 판단한 개수를 세기위해 선언한 변수이다.
    int count_spam= 0; //spam 메일로 판단한 개수를 세기위해 선언한 변수이다.
    bool spam_check=false;//유형이 spam인지 확인한다.
    bool ham_check=false; //유형이 ham인지 확인한다. 
    string type;//어느 유형인지를 저장하는 변수이다.

    ifstream file(input_file);

    stringstream ss;
    ss << file.rdbuf();
    wholeline = ss.str();
    string delimiter(",\"");

    parse(wholeline,mail_prev,delimiter); // ,"를 기준으로 구분해준다.
    
    if(mail_prev[0].find("spam")!=string::npos)
        spam_check= true;
    else
        ham_check= true;
    mail_prev.erase(mail_prev.begin()); //처음에 숫자와 메일유형이 오기 때문에 이를 지워준다.

    for(string value : mail_prev){
        value=value.substr(0,value.rfind("\"")); //뒤에서부터 "를 찾아 메일번호와 메일유형을 필요하지 않기 때문에 지워준다.
        if(value.find("\n")!=string::npos)
            value=regex_replace(value,regex("\n")," "); //개행문자는 구분을 못해주어서 개행문자를 " "(공백)으로 바꾸어주었다.
        just_word = split(value, ' ');
         for(string w : just_word){
            for (char c: punctuation) {
                w.erase(remove(w.begin(), w.end(), c), w.end()); //필요없는 문자를 제거해준다.
            }
            if(w!="")
            split_word.insert(w);
        } //필요없는 문자들을 제거하고 중복없는 단어로 정리해 set에 넣어준다.
        double p =1.0; //spam메일이 단어를 가질 확률을 저장하는 부분
        double q =1.0; //ham메일이 단어를 가질 확률을 저장하는 부분
        double r =0.0; //총 확률을 계산하는 부분이다.
        for(string word : split_word){
            if((count1[word]!=0) && (count2[word]!=0)){
                p *=((double) count2[word]/ SPAM_TRAIN_SIZE);
                q *=((double) count1[word]/ HAM_TRAIN_SIZE);
            }
        }
        r= p / (p+q);
        if(ham_check) type= "ham";
        else type= "spam";
        cout <<type << "\tnumber :"<<num  << "\tp : "<< p <<"\tq: "<< q <<"\tthreshold: "<<threshold << "\tr: "<< r <<endl;
        if (threshold > r) {
            cout << "Ham!!" << endl;
            count_ham++;
        }
        else {
            cout << "Spam!!" << endl;
            count_spam++;
        }
        split_word.clear();//비워준다.
        num++;
    }
    cout<<"judgment:\t"<<"ham : "<<count_ham<< "\tspam: "<<count_spam<<endl;
    if(type == "ham")
        return count_ham;
    else
        return count_spam;
}

//정확도를 계산해주는 함수이다.
double cal_accuracy(int num_ham, int num_spam){
    return ((double)(num_ham+num_spam))/(HAM_TEST_SIZE+SPAM_TEST_SIZE)*100;
}

int main(int argc, char* argv[]){
    map<string, int> word_ham; //ham에 있는 단어와 횟수를 저장해준다.
    map<string, int> word_spam; //spam에 있는 단어와 횟수를 저장한다.
    
    string ham_file = ".\\csv\\train\\dataset_ham_train100.csv";
    string spam_file = ".\\csv\\train\\dataset_spam_train100.csv";
    string test_ham_file = ".\\csv\\test\\dataset_ham_test20.csv";
    string test_spam_file = ".\\csv\\test\\dataset_spam_test20.csv";
    
    make_wordnum(ham_file,word_ham); //non-spam mail의 단어와 나온 개수 저장
    make_wordnum(spam_file, word_spam); //spam mail의 단어와 나온 개수 저장

    int ham = test(test_ham_file, word_ham, word_spam);
    cout<<"*************************************"<<endl;
    int spam = test(test_spam_file, word_ham, word_spam);
    double accuracy = cal_accuracy(ham, spam);
    cout<<"accurancy : "<<accuracy<<"%"<<endl;
}