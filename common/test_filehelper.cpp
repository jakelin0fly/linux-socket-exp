#include "helper.h"
#include <vector>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
	// Helper::Serialize("data.txt", 3);
	// Helper::Serialize("data.txt", 56);
	// Helper::Serialize("data.txt", 8);

	// std::vector<int> v;
	// Helper::Deserialize("data.txt", v);
	// for (int i = 0; i < v.size(); ++i)
	// {
	// 	cout << v[i] << endl;
	// }

	std::vector<ServerStruct> v;
	Helper::GetServersFromFile("config.txt", v);
	for (int i = 0; i < v.size(); ++i)
	{
		cout<<v[i].str_ip<<":"<<v[i].port<<endl;
	}

	return 0;
}