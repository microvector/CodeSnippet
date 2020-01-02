#include <iostream>
#include <string.h>
#include <fstream>

#define HOSTNAME_ADDR "/etc/hostname"

using namespace std;

const string readFile(const string srcFile)
{
    cout << "srcFile   :" << srcFile << endl;
    ifstream inputStream(srcFile.c_str());
    if (!inputStream.is_open())
    {
        cout << "open src File  Error opening file" << endl;
        return " error ";
    }
    // get length of file
    inputStream.seekg(0, std::ios::end);
    // pointer point to EOF
    int length = inputStream.tellg();
    inputStream.seekg(0, std::ios::beg);
    char *content = new char[length];
    inputStream.read(content, length);
    // convert char to string ；operate string is convenient
    string contentStr = content;
    // close ifstream
    inputStream.close();
    return contentStr;
}

int main()
{
    const string con = readFile(HOSTNAME_ADDR);
    // Attention : the file`s length include '\n'
    cout << "size of flie = " << con.length() << endl;
    // We can see that there are two rows of output 
    cout << "file`s content =:" << con << "----------" << endl;
    return 0;
}