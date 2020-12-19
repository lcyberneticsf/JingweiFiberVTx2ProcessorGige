//#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include "detect_recog.h"
//#include <opencv2\contrib\contrib.hpp>  
//#include <opencv2\core\core.hpp>  
//#include <opencv2\highgui\highgui.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
//#include <io.h>  
//#include <direct.h> 
#include <sys/types.h>
//#include <conio.h>
using namespace std;
using namespace cv;
int m_nFileFind = 0;


void all_from_json(const std::string& json, std::vector<BaseDetectResult>& results)
{
	//FILE* fp = fopen("f:/json.txt", "w");
	//fwrite(json.c_str(), sizeof(char),strlen(json.c_str()),fp);
	results.clear();
	Json::Value root;
	Json::Reader reader;
	reader.parse(json, root);
	int size = root.size();
	BaseDetectResult single_result;
	std::string strType = root["dataset_type"].asString();
	Json::Value json_value;
	json_value = root["dataset_type"];
	single_result.release();
	//single_result.from_json(root["regions"][0]["polygon"]);
	if (root["regions"].isArray())
	{
		int nArraySize = root["regions"].size();
		for (int i = 0; i < root["regions"].size(); i++)
		{
			nArraySize = root["regions"][i]["polygon"].size();
			//strType = root["regions"][i]["polygon"]["outer"]["points"][0]["x"].asString();
			single_result.from_json(root["regions"][i]["polygon"]);
			results.push_back(single_result);
			single_result.release();
		}
	}
	root.clear();
}



void release_json(std::vector<BaseDetectResult>& results)
{
	int nsize = results.size();
	for (int i = 0; i < results.size(); i++)
	{
		nsize = results[i].defects.size();
		for (int j = 0; j < results[i].defects.size(); j++)
		{
			nsize = results[i].defects[j].contours.size();
			results[i].defects[j].contours.clear();
		}
		results[i].defects.clear();
	}
	results.clear();	
}


void get_json_information(const std::string json_str, Aqlabel m_label, std::string label_path, int num)
{
	Json::Value root;
	Json::Reader reader;
	reader.parse(json_str, root);
	aq::aidi::Label label;
	m_label.dataset_type = root["dataset_type"].asString();
	//int type = root["dataset_type"].asInt();;
	m_label.width = root["img_size"]["width"].asFloat();
	m_label.height = root["img_size"]["height"].asFloat();
	m_label.score = root["score"].asFloat();
	label.set_dataset_type(static_cast<aq::aidi::Label_DataSetType>(2));//static_cast 强制类型转换
	label.mutable_img_size()->set_width(m_label.width);
	label.mutable_img_size()->set_height(m_label.height);
	label.set_score(m_label.score);

	Json::Value regions = root["regions"];
	int nsize = regions.size();
	m_label.m_region.resize(nsize);
	for (int i = 0; i < regions.size(); i++)
	{
		aq::aidi::Region* region = label.add_regions();

		string name = regions[i]["name"].asString();
		float score_r = regions[i]["score"].asFloat();
		region->set_name(name);
		region->set_score(score_r);
		aq::aidi::Polygon* poly = region->mutable_polygon();
		aq::aidi::Ring* outer = poly->mutable_outer();
		Json::Value outers = regions[i]["polygon"]["outer"]["points"];
		for (int j = 0; j < outers.size(); j++)
		{
			aq::aidi::Point2f* pt = outer->add_points();
			float x = outers[j]["x"].asFloat();
			float y = outers[j]["y"].asFloat();
			std::string strTypeX = outers[j]["x"].asString();
			std::string strTypeY = outers[j]["y"].asString();
			cv::Point point(x, y);
			pt->set_x(x);
			pt->set_y(y);
			m_label.m_region[i].regions.push_back(point);
		}
		m_label.m_region[i].name.push_back(name);
		m_label.m_region[i].score_r.push_back(score_r);
	}
	if (num != -1)
	{
		/*aq::aidi::Label label1;
		ifstream infile("G:\\经纬检测\\检测\\FastDetection_0\\label\\1.aqlabel", ios::binary);
		label1.ParseFromIstream(&infile);
		std::string m_show1 = label1.DebugString();
		std::string m_show = label.DebugString();*/
		string  savepath = label_path + std::to_string(num) + ".aqlabel";
		SaveLabel(label, savepath);
	}

}

bool SaveLabel(const aq::aidi::Label& label, string save_path)
{

	cout << save_path << endl;

	ofstream outfile(save_path, ofstream::binary);//ios::binary
	if (!outfile.is_open()) {
		return false;
	}
	size_t byte_len = label.ByteSizeLong();
	char* data = new char[byte_len];

	label.SerializeToArray(data, byte_len);
	outfile.write(data, byte_len);
	outfile.close();
	free(data);
	return true;
}