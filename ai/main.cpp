#include<cstdio>
#include<cstring>
#include<iostream>
#include"definition.h"
#include"ai.h"
#include"../json/json.h"

int main(){
	std::ios::sync_with_stdio(false);
	std::string infoString;
	std::cin >> infoString;
	Json::Reader reader;
	Json::Value infoJson;
	if (!reader.parse(infoString.data(), infoJson)){
		return 1;
	}

	Info info(infoJson);
	player_ai(info);

	Json::FastWriter writer;
	string CommandListString = writer.write(info.myCommandList.asJson());
	std::cout<<CommandListString;
	fflush(stdout);
	return 0;
}