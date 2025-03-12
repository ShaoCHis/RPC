#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
using namespace std;

bool isMatch(string a,string b)
{
    istringstream is(b);
    vector<string> words;
    string temp;
    while(is>>temp)
        words.emplace_back(temp);
    unordered_map<char,string> ctow;
    unordered_map<string,char> wtoc;
    for(int i = 0;i<a.size();i++)
    {
        if(ctow.find(a[i])==ctow.end()&&wtoc.find(words[i])==wtoc.end())
        {
            ctow[a[i]] = words[i];
        }
        else
        {
            if(ctow.find(a[i])!=ctow.end()&&wtoc.find(words[i])!=wtoc.end())
            {
                if(ctow[a[i]] == words[i]&&wtoc[words[i]]==a[i])
                    continue;
                else
                    return false;
            }
        }
    }
    return true;
}