#include "filehelper.h"
#include <vector>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
	FileHelper::Serialize("data.txt", 3);
	FileHelper::Serialize("data.txt", 56);
	FileHelper::Serialize("data.txt", 8);

	std::vector<int> v;
	FileHelper::Deserialize("data.txt", v);
	for (int i = 0; i < v.size(); ++i)
	{
		cout << v[i] << endl;
	}

	return 0;
}